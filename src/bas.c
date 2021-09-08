#include <kernel.h>
#include <bluetooth/services/bas.h>

static void refresh_battery(struct k_timer *timer)
{
	uint8_t level = bt_bas_get_battery_level();
	if (!--level) {
		level = 100;
	}
	bt_bas_set_battery_level(level);
}

K_TIMER_DEFINE(battery_sim_timer, refresh_battery, NULL);

void ble_ws_bas_init(void)
{
	bt_bas_set_battery_level(100);
	k_timer_start(&battery_sim_timer, K_SECONDS(60), K_SECONDS(60));
}
