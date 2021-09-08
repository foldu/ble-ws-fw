#pragma once

#include <bluetooth/mesh.h>

int ble_ws_mesh_init(void);

void ble_ws_vnd_read_sensor(struct bt_mesh_model *model,
			    struct bt_mesh_msg_ctx *ctx,
			    struct net_buf_simple *payload_buf);
