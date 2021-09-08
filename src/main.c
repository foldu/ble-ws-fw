#include <zephyr.h>
#include <stddef.h>
#include <sys/printk.h>

#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <logging/log.h>
//#include <sys/util.h>
//#include <device.h>
//#include <devicetree.h>
//#include <drivers/sensor.h>
#include <bluetooth/bluetooth.h>
//#include <bluetooth/hci.h>
//#include <bluetooth/uuid.h>
//#include <bluetooth/mesh.h>
#include "sensor.h"
#include "mesh.h"

LOG_MODULE_REGISTER(ble_ws);

void main(void)
{
	const struct device *dev = DEVICE_DT_GET_ANY(bosch_bme280);

	if (!dev) {
		LOG_ERR("No BME280 device found");
		return;
	}

	if (!device_is_ready(dev)) {
		LOG_ERR("Device %s is not ready", dev->name);
		return;
	}

	ble_ws_sensor_init(dev);

	if (bt_enable(NULL)) {
		LOG_ERR("Could not enable bluetooth");
		return;
	}

	if (ble_ws_mesh_init()) {
		LOG_ERR("Could not init mesh");
		return;
	}

	struct ble_ws_sensor_values values = { 0 };
	while (1) {
		k_sleep(K_SECONDS(60));
		if (ble_ws_sensor_read(&values)) {
			LOG_ERR("Failed reading sensors");
		} else {
			LOG_INF("%d %d %d", values.temperature, values.humidity,
				values.pressure);
		}
	}
}
