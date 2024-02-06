/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sensor_monitoring/app_leds.h>
#include <dk_buttons_and_leds.h>

static int leds_state;
static uint8_t led_id_arr[APP_LEDS_MAX];

void app_led_turn_on_all(void)
{
	dk_set_leds(DK_ALL_LEDS_MSK);
	leds_state = DK_ALL_LEDS_MSK;
}

void app_led_turn_off_all(void)
{
	dk_set_leds(DK_NO_LEDS_MSK);
	leds_state = DK_NO_LEDS_MSK;
}

void app_led_turn_on(enum leds_id_t id)
{
	if (id < LED_ID_LAST) {
		dk_set_led_on(id);
		WRITE_BIT(leds_state, id, true);
	}
}

void app_led_turn_off(enum leds_id_t id)
{
	if (id < LED_ID_LAST) {
		dk_set_led_off(id);
		WRITE_BIT(leds_state, id, false);
	}
}

bool app_led_is_on(enum leds_id_t id)
{
	if (id < LED_ID_LAST) {
		return (leds_state & BIT(id));
	}
	return false;
}

uint8_t *app_led_id_array_get()
{
	return led_id_arr;
}

int app_led_init()
{
	for (size_t i = 0; i < APP_LEDS_MAX; i++) {
		led_id_arr[i] = i;
	}
	return dk_leds_init();
}
