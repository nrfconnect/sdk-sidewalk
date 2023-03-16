/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sm_buttons.h>
#include <buttons.h>
#include <assert.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_buttons, CONFIG_SIDEWALK_LOG_LEVEL);

#define BTN_CHECK_AND_SET_MASK(_id) do {					\
		if (!atomic_test_bit(&button_press_mask, button_id_arr[_id])) {	\
			atomic_set_bit(&button_press_mask, button_id_arr[_id]);	\
			notify_event = true;					\
		}								\
} while (0);

static uint8_t button_id_arr[DEMO_BUTTONS_MAX];
static atomic_t button_press_mask = ATOMIC_INIT(0);
static uint32_t button_notify_mask;
static uint32_t button_press_time_in_sec_id_arr[DEMO_BUTTONS_MAX];
static atomic_t button_event_pending_processing = ATOMIC_INIT(false);

static void btn_event_handler(uint32_t event)
{
	if ((sm_app_state_get() != DEMO_APP_STATE_NOTIFY_SENSOR_DATA) &&
	    (event != BTN_EVENT_RESET_KEY)) {
		return;
	}

	bool notify_event = false;

	switch ((enum btn_event)event) {
	case BTN_EVENT_RESET_KEY:
		sm_main_task_msg_q_write(EVENT_FACTORY_RESET);
		break;
	case BTN_EVENT_KEY_0:
		BTN_CHECK_AND_SET_MASK(0);
		break;
	case BTN_EVENT_KEY_1:
		BTN_CHECK_AND_SET_MASK(1);
		break;
	case BTN_EVENT_KEY_2:
		BTN_CHECK_AND_SET_MASK(2);
		break;
	case BTN_EVENT_KEY_3:
		BTN_CHECK_AND_SET_MASK(3);
		break;
	default:
		break;
	}

	if (notify_event &&
	    atomic_cas(&button_event_pending_processing, false, true)) {
		sm_main_task_msg_q_write(EVENT_BUTTON_PRESS);
	}
}

void sm_buttons_action_response_process(struct sid_parse_state *state)
{
	assert(state);
	uint8_t temp_button_id_arr[DEMO_BUTTONS_MAX] = { 0 };
	struct sid_demo_action_resp action_resp;

	memset(&action_resp, 0, sizeof(action_resp));

	action_resp.button_action_resp.button_id_arr = temp_button_id_arr;
	sid_demo_app_action_resp_deserialize(state, &action_resp);
	if (state->ret_code != SID_ERROR_NONE) {
		LOG_ERR("de-serialize action resp failed %d", state->ret_code);
	} else if (action_resp.resp_type == SID_DEMO_ACTION_TYPE_BUTTON) {
		if (action_resp.button_action_resp.num_buttons == 0xFF) {
			atomic_clear(&button_press_mask);
			button_notify_mask = 0;
			for (size_t i = 0; i < DEMO_BUTTONS_MAX; i++) {
				button_press_time_in_sec_id_arr[i] = 0;
			}
		} else if (action_resp.button_action_resp.num_buttons <= DEMO_BUTTONS_MAX) {
			for (size_t i = 0; i < action_resp.button_action_resp.num_buttons; i++) {
				atomic_clear_bit(&button_press_mask, action_resp.button_action_resp.button_id_arr[i]);
				WRITE_BIT(button_notify_mask, action_resp.button_action_resp.button_id_arr[i], false);
				button_press_time_in_sec_id_arr[i] = 0;
			}
		} else {
			LOG_ERR("Invalid number of button Max allowed %d received %d",
				DEMO_BUTTONS_MAX, action_resp.button_action_resp.num_buttons);
		}
	} else {
		LOG_ERR("Invalid response received %d", action_resp.resp_type);
	}
}

void sm_buttons_pendig_flag_clear()
{
	atomic_clear(&button_event_pending_processing);
}

void sm_buttons_notify_mask_bit_set(uint8_t btn_id)
{
	if (btn_id < DEMO_BUTTONS_MAX) {
		WRITE_BIT(button_notify_mask, button_id_arr[btn_id], true);
	}
}

void sm_buttons_notify_mask_bit_clear(uint8_t btn_id)
{
	if (btn_id < DEMO_BUTTONS_MAX) {
		WRITE_BIT(button_notify_mask, button_id_arr[btn_id], false);
	}
}

bool sm_buttons_notify_mask_bit_is_set(uint8_t btn_id)
{
	if (btn_id < DEMO_BUTTONS_MAX) {
		return (button_notify_mask & BIT(button_id_arr[btn_id]));
	}
	return false;
}

uint32_t sm_buttons_notify_mask_get()
{
	return button_notify_mask;
}

void sm_buttons_press_mask_bit_clear(uint8_t btn_id)
{
	if (btn_id < DEMO_BUTTONS_MAX) {
		atomic_clear_bit(&button_press_mask, button_id_arr[btn_id]);
	}
}

bool sm_buttons_press_mask_bit_is_set(uint8_t btn_id)
{
	if (btn_id < DEMO_BUTTONS_MAX) {
		return atomic_test_bit(&button_press_mask, button_id_arr[btn_id]);
	}
	return false;
}

uint8_t *sm_buttons_id_array_get()
{
	return button_id_arr;
}

void sm_buttons_press_time_set(uint8_t btn_id, sid_time_t sec)
{
	if (btn_id < DEMO_BUTTONS_MAX) {
		button_press_time_in_sec_id_arr[btn_id] = sec;
	}
}

sid_time_t sm_buttons_press_time_get(uint8_t btn_id)
{
	if (btn_id < DEMO_BUTTONS_MAX) {
		return button_press_time_in_sec_id_arr[btn_id];
	}
	return 0;
}

int sm_buttons_init()
{
	button_set_action_long_press(DK_BTN1, btn_event_handler, BTN_EVENT_RESET_KEY);
	button_set_action_short_press(DK_BTN1, btn_event_handler, BTN_EVENT_KEY_0);
	button_set_action(DK_BTN2, btn_event_handler, BTN_EVENT_KEY_1);
	button_set_action(DK_BTN3, btn_event_handler, BTN_EVENT_KEY_2);
	button_set_action(DK_BTN4, btn_event_handler, BTN_EVENT_KEY_3);

	for (size_t i = 0; i < DEMO_BUTTONS_MAX; i++) {
		button_id_arr[i] = i;
	}

	return buttons_init();
}
