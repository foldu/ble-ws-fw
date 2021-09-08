#pragma once

#include <stdint.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>

struct ble_ws_sensor_values {
	int16_t temperature;
	uint16_t humidity;
	uint32_t pressure;
} __attribute__((__packed__));

void ble_ws_sensor_init(const struct device *dev);

int ble_ws_sensor_read(struct ble_ws_sensor_values *values);
