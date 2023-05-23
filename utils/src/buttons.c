/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "buttons.h"
#include "buttons_internal.h"
#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>
#include <errno.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(button, HAL_BUTTONS_LOG_LEVEL);

#define BUTTON_NR_ON_DK(_nr) (_nr + 1)

#define BUTTONS_NUMBER (4)

typedef struct {
	btn_handler_t handler;
	uint32_t param;
} button_act_t;

static button_act_t buttons[BUTTONS_NUMBER][BUTTON_ACTION_LAST];
static int64_t buttons_press_time[BUTTONS_NUMBER];

static int action_set(uint8_t button_nr, button_action_t act_nr, btn_handler_t handler,
		      uint32_t param)
{
	if (button_nr >= BUTTONS_NUMBER || act_nr >= BUTTON_ACTION_LAST) {
		return -ENOENT;
	}

	buttons[button_nr][act_nr].handler = handler;
	buttons[button_nr][act_nr].param = param;

	return 0;
}

static void action_run(uint8_t button_nr, button_action_t act_nr)
{
	LOG_INF("button pressed %d %s", BUTTON_NR_ON_DK(button_nr),
		BUTTON_ACTION_LONG_PRESS == act_nr ? "long" : "short");

	button_act_t action = buttons[button_nr][act_nr];

	if (action.handler) {
		action.handler(action.param);
	}
}

static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	uint8_t button_nr;

	switch (has_changed) {
	case DK_BTN1_MSK:
		button_nr = DK_BTN1;
		break;
	case DK_BTN2_MSK:
		button_nr = DK_BTN2;
		break;
	case DK_BTN3_MSK:
		button_nr = DK_BTN3;
		break;
	case DK_BTN4_MSK:
		button_nr = DK_BTN4;
		break;
	default:
		return;
	}

	if (button_state & has_changed) {
		buttons_press_time[button_nr] = k_uptime_get();
	} else {
		if (k_uptime_delta(&buttons_press_time[button_nr]) >
		    (BUTTONS_LONG_PRESS_TIMEOUT_SEC * MSEC_PER_SEC)) {
			action_run(button_nr, BUTTON_ACTION_LONG_PRESS);
		} else {
			action_run(button_nr, BUTTON_ACTION_SHORT_PRESS);
		}
	}
}

int button_set_action_long_press(uint8_t button_nr, btn_handler_t handler, uint32_t param)
{
	return action_set(button_nr, BUTTON_ACTION_LONG_PRESS, handler, param);
}

int button_set_action_short_press(uint8_t button_nr, btn_handler_t handler, uint32_t param)
{
	return action_set(button_nr, BUTTON_ACTION_SHORT_PRESS, handler, param);
}

int button_set_action(uint8_t button_nr, btn_handler_t handler, uint32_t param)
{
	int err = button_set_action_short_press(button_nr, handler, param);

	if (!err) {
		err = button_set_action_long_press(button_nr, handler, param);
	}

	return err;
}

int buttons_init(void)
{
	return dk_buttons_init(button_handler);
}

int buttons_deinit(void)
{
	memset(buttons, 0, sizeof(buttons));
	return dk_buttons_init(NULL);
}

void button_pressed(uint8_t button_nr, button_action_t action)
{
	action_run(button_nr, action);
}
