/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr.h>
#include <dk_buttons_and_leds.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(sid_template, CONFIG_SIDEWALK_LOG_LEVEL);

#define SID_LED_INDICATE_CONNECTED  (DK_LED1)
#define SID_LED_INDICATE_ERROR      (DK_LED2)

#define IS_RESET_BTN_PRESSED(_btn)      (_btn & DK_BTN1_MSK)
#define IS_CONN_REQ_BTN_PRESSED(_btn)   (_btn & DK_BTN2_MSK)
#define IS_SEND_MSG_BTN_PRESSED(_btn)   (_btn & DK_BTN3_MSK)
#define IS_SET_BAT_LV_BTN_PRESSED(_btn) (_btn & DK_BTN4_MSK)

static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	uint32_t button = button_state & has_changed;

	if (IS_RESET_BTN_PRESSED(button)) {
		
	}

	if (IS_CONN_REQ_BTN_PRESSED(button)) {
		
	}

	if (IS_SEND_MSG_BTN_PRESSED(button)) {
		
	}

	if (IS_SET_BAT_LV_BTN_PRESSED(button)) {
		
	}
}

static int board_init(void)
{
	int err = dk_buttons_init(button_handler);

	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
		return err;
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return err;
	}
	return 0;
}

void main(void)
{
	LOG_INF("Sidewalk example started!");

	if (0 != board_init()) {
		return;
	}

	for (;;) {
		k_sleep(K_MSEC(500));
	}
}
