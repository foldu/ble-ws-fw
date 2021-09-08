#include <bluetooth/uuid.h>
#include <drivers/hwinfo.h>
#include <bluetooth/bluetooth.h>
#include "mesh.h"
#include <logging/log.h>
#include "sensor.h"
#include "attention.h"
#include <assert.h>

LOG_MODULE_DECLARE(ble_ws);

#define OP_VENDOR_READ_SENSOR (BT_MESH_MODEL_OP_3(0x00, BT_COMP_ID_LF))
#define OP_VENDOR_SENSORDATA (BT_MESH_MODEL_OP_3(0x01, BT_COMP_ID_LF))
#define MOD_LF (0x0000)

struct addr_u16 {
	// 0x + addr + \0
	char ___addr[2 + 4 + 1];
};

static struct addr_u16 addr_u16_format(uint16_t addr)
{
	// FIXME: zephyr probably has some macro for this but I can't find it
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	addr = (addr << 8) | ((addr >> 8) & 0xff);
#endif
	struct addr_u16 ret;
	char *s = (char *)&ret;
	s[0] = '0';
	s[1] = 'x';
	const size_t nbytes =
		bin2hex((char *)&addr, sizeof(uint16_t), s + 2, 4 + 1);
	assert(nbytes);
	// FIXME: clang is being stupid
	(void)nbytes;

	return ret;
}

#define ADDR_U16_FORMAT(ident, addr)                                           \
	struct addr_u16 ident = addr_u16_format(addr)

static void attention_on(struct bt_mesh_model *mod)
{
	(void)mod;
	ble_ws_attention_on();
}

static void attention_off(struct bt_mesh_model *mod)
{
	(void)mod;
	ble_ws_attention_off();
}

static const struct bt_mesh_health_srv_cb health_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 1);

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_cb,
};

// supported bt mesh models
static struct bt_mesh_model root_models[] = {
	BT_MESH_MODEL_CFG_SRV,
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
};

static const struct bt_mesh_model_op vnd_ops[] = {
	{ .opcode = OP_VENDOR_READ_SENSOR,
	  .min_len = 0,
	  .func = ble_ws_vnd_read_sensor },
	BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model vnd_models[] = {
	BT_MESH_MODEL_VND(BT_COMP_ID_LF, MOD_LF, vnd_ops, NULL, NULL),
};

// extra supported models
static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(0, root_models, vnd_models),
};

static const struct bt_mesh_comp comp = {
	.cid = 0,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

static void prov_complete(uint16_t idx, uint16_t addr)
{
	(void)idx;
	ADDR_U16_FORMAT(s, addr);
	LOG_INF("Provisioned with addr %s", (char *)&s);
}

static void prov_reset()
{
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

static uint8_t dev_uuid[16];

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.output_size = 4,
	// out of bounds provisioner key on device
	.output_actions = BT_MESH_PROV_OOB_ON_DEV,
	.complete = prov_complete,
	.reset = prov_reset,
};

void ble_ws_vnd_read_sensor(struct bt_mesh_model *model,
			    struct bt_mesh_msg_ctx *ctx,
			    struct net_buf_simple *payload_buf)
{
	(void)payload_buf;
	(void)model;

	LOG_INF("Got sensor read request");

	struct ble_ws_sensor_values values = { 0 };
	if (ble_ws_sensor_read(&values)) {
		LOG_ERR("Reading sensor values failed");
		return;
	}

	BT_MESH_MODEL_BUF_DEFINE(rbuf, OP_VENDOR_SENSORDATA, 2 + 2 + 4);

	bt_mesh_model_msg_init(&rbuf, OP_VENDOR_SENSORDATA);

	net_buf_simple_add_le16(&rbuf, values.temperature);
	net_buf_simple_add_le16(&rbuf, values.humidity);
	net_buf_simple_add_le32(&rbuf, values.pressure);

	ADDR_U16_FORMAT(s, ctx->addr);
	LOG_INF("Sending response to %s", (char *)&s);

	bt_mesh_model_send(&vnd_models[0], ctx, &rbuf, NULL, NULL);
}

int ble_ws_mesh_init(void)
{
	int ret = hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));
	if (ret == -ENOTSUP) {
		LOG_ERR("hwinfo device id not supported on this device");
		return ret;
	} else if (ret < 0) {
		LOG_ERR("hwinfo device id driver error");
		return ret;
	}

	if ((ret = ble_ws_attention_init())) {
		LOG_ERR("Failed to init attention led");
		return ret;
	}

	ret = bt_mesh_init(&prov, &comp);
	if (ret) {
		LOG_ERR("bt_mesh_init failed %d", ret);
		return ret;
	}

	prov_reset();

	return ret;
}
