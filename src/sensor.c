#include "sensor.h"
#include <assert.h>
#include <string.h>
#include <sys/mutex.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(ble_ws);

K_MUTEX_DEFINE(sensor_mutex);

K_TIMER_DEFINE(sensor_read_timer, NULL, NULL);

static bool initial_read = true;

static struct ble_ws_sensor_values cached_values;

static const struct device *device;

void ble_ws_sensor_init(const struct device *dev)
{
	device = dev;
}

int ble_ws_sensor_read(struct ble_ws_sensor_values *values)
{
	assert(device);
	k_mutex_lock(&sensor_mutex, K_FOREVER);

	int ret = 0;
	if (initial_read || k_timer_status_get(&sensor_read_timer) > 0) {
		LOG_INF("Sensor read triggered");
		initial_read = false;

		ret = sensor_sample_fetch(device);
		if (ret) {
			goto err;
		}

		struct sensor_value buf;
		sensor_channel_get(device, SENSOR_CHAN_AMBIENT_TEMP, &buf);
		cached_values.temperature = (int16_t)buf.val1 * 100;
		cached_values.temperature += buf.val2 / 10000;

		sensor_channel_get(device, SENSOR_CHAN_HUMIDITY, &buf);
		cached_values.humidity = (uint16_t)buf.val1 * 100;
		cached_values.humidity += (uint16_t)(buf.val2 / 10000);

		sensor_channel_get(device, SENSOR_CHAN_PRESS, &buf);
		// FIXME: wrong
		cached_values.pressure = ((uint32_t)buf.val1) * 1000000;
		cached_values.pressure += (uint32_t)buf.val2;

		k_timer_start(&sensor_read_timer, K_SECONDS(30), K_MSEC(0));
	}
	memcpy(values, &cached_values, sizeof(struct ble_ws_sensor_values));

err:
	k_mutex_unlock(&sensor_mutex);
	return ret;
}
