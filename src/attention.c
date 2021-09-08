#include <assert.h>
#include <devicetree.h>
#include <device.h>
#include <drivers/gpio.h>

#define ATTENTION_NODE DT_ALIAS(led0)

static const struct device *led;

int ble_ws_attention_init(void)
{
	led = device_get_binding(DT_GPIO_LABEL(ATTENTION_NODE, gpios));
	if (!led) {
		return 1;
	}

	return gpio_pin_configure(led, DT_GPIO_PIN(ATTENTION_NODE, gpios),
				  GPIO_OUTPUT_ACTIVE |
					  DT_GPIO_FLAGS(ATTENTION_NODE, gpios));
}

void ble_ws_attention_on(void)
{
	assert(led);
	gpio_pin_set(led, DT_GPIO_PIN(DT_ALIAS(attention_led), gpios), false);
}

void ble_ws_attention_off(void)
{
	assert(led);
	gpio_pin_set(led, DT_GPIO_PIN(DT_ALIAS(attention_led), gpios), true);
}
