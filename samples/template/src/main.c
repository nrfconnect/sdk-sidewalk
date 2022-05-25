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

#define SIDEWALK_THREAD_STACK_SIZE      (512)
#define SIDEWALK_THREAD_PRIORITY        (K_LOWEST_APPLICATION_THREAD_PRIO)
#define SIDEWALK_MSGQ_SIZE              (4)

enum event_type {
	EVENT_TYPE_SIDEWALK,
	EVENT_TYPE_SEND_HELLO,
	EVENT_TYPE_SET_BATTERY_LEVEL,
	EVENT_TYPE_FACTORY_RESET,
	EVENT_TYPE_CONNECTION_REQUEST,
};

static void sidewalk_poll(void);

K_MSGQ_DEFINE(sid_msgq, sizeof(enum event_type), SIDEWALK_MSGQ_SIZE, 4);
K_THREAD_DEFINE(sidewalk_thread, SIDEWALK_THREAD_STACK_SIZE,
		sidewalk_poll, NULL, NULL, NULL,
		SIDEWALK_THREAD_PRIORITY, 0, 0);

void message_q_write(enum event_type event)
{
	while (0 != k_msgq_put(&sid_msgq, &event, K_NO_WAIT)) {
		k_msgq_purge(&sid_msgq);
	}
}

static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	uint32_t button = button_state & has_changed;

	if (IS_RESET_BTN_PRESSED(button)) {
		message_q_write(EVENT_TYPE_FACTORY_RESET);
	}

	if (IS_CONN_REQ_BTN_PRESSED(button)) {
		message_q_write(EVENT_TYPE_CONNECTION_REQUEST);
	}

	if (IS_SEND_MSG_BTN_PRESSED(button)) {
		message_q_write(EVENT_TYPE_SEND_HELLO);
	}

	if (IS_SET_BAT_LV_BTN_PRESSED(button)) {
		message_q_write(EVENT_TYPE_SET_BATTERY_LEVEL);
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

static void sidewalk_poll(void)
{
	LOG_INF("Starting sidewalk thread ...");

	while (1) {
		enum event_type event;

		if (!k_msgq_get(&sid_msgq, &event, K_FOREVER)) {
			switch (event) {
			case EVENT_TYPE_SIDEWALK:

				break;
			case EVENT_TYPE_SEND_HELLO:

				break;
			case EVENT_TYPE_SET_BATTERY_LEVEL:

				break;
			case EVENT_TYPE_FACTORY_RESET:

				break;
			case EVENT_TYPE_CONNECTION_REQUEST:

				break;
			default: break;
			}
		}
	}
}

void main(void)
{
	LOG_INF("Sidewalk example started!");

	if (0 != board_init()) {
		return;
	}

	while (1) {
		k_sleep(K_FOREVER);
	}
}
