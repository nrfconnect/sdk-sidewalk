/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sm_notifications.h>
#include <sm_leds.h>
#include <sm_buttons.h>
#include <sm_timers.h>
#include <sid_pal_temperature_ifc.h>
#include <assert.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#if defined(CONFIG_BOARD_THINGY53_NRF5340_CPUAPP)
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>

static const struct device *bme688_sensor_dev = DEVICE_DT_GET_ONE(bosch_bme680);
#endif

LOG_MODULE_REGISTER(sm_notify, CONFIG_SIDEWALK_LOG_LEVEL);

#define SENSOR_NOTIFY_PAYLOAD_SIZE_MAX 32
#define CAPABILITY_NOTIFY_PAYLOAD_SIZE_MAX 32

void sm_notify_capability(app_context_t *app_context)
{
	assert(app_context);

	struct sid_parse_state state = {};
	enum sid_link_type link_type = sm_active_link_type_get(app_context);
	struct sid_demo_capability_discovery cap = {
		.num_buttons = DEMO_BUTTONS_MAX,
		.button_id_arr = sm_buttons_id_array_get(),
		.num_leds = sm_leds_id_array_size_get(),
		.led_id_arr = sm_leds_id_array_get(),
		.temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS,
	};

	uint8_t temp_buffer[CAPABILITY_NOTIFY_PAYLOAD_SIZE_MAX] = { 0 };

	cap.link_type = link_type;

	sid_parse_state_init(&state, temp_buffer, sizeof(temp_buffer));
	sid_demo_app_capability_discovery_notification_serialize(&state, &cap);
	if (state.ret_code != SID_ERROR_NONE) {
		LOG_ERR("Capability discovery serialize failed -%d", state.ret_code);
		return;
	}

	struct sid_demo_msg_desc msg_desc = {
		.status_hdr_ind = false,
		.opc = SID_DEMO_MSG_TYPE_NOTIFY,
		.cmd_class = SID_DEMO_APP_CLASS,
		.cmd_id = SID_DEMO_APP_CLASS_CMD_CAP_DISCOVERY_ID,
	};

	struct sid_demo_msg demo_msg = { .payload = temp_buffer, .payload_size = state.offset };

	sid_parse_state_init(&state, app_context->buffer, PAYLOAD_MAX_SIZE);
	sid_demo_app_msg_serialize(&state, &msg_desc, &demo_msg);
	if (state.ret_code != SID_ERROR_NONE) {
		LOG_ERR("Demo msg capability discovery serialize failed -%d", state.ret_code);
		return;
	}

	struct sid_msg msg = {
		.data = app_context->buffer,
		.size = state.offset,
	};

	struct sid_msg_desc desc = {
		.link_type = link_type,
		.type = SID_MSG_TYPE_NOTIFY,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

	LOG_INF("Sending demo app message notify capability");
	sm_send_msg(app_context, &desc, &msg);
}

void sm_notify_sensor_data(app_context_t *app_context, bool button_pressed)
{
	assert(app_context);

	struct sid_parse_state state;
	enum sid_link_type link_type = SID_LINK_TYPE_1;
	struct sid_demo_action_notification action_notify = { 0 };
	struct sid_timespec curr_time = { 0 };
	uint8_t temp_buffer[SENSOR_NOTIFY_PAYLOAD_SIZE_MAX] = { 0 };

	memset(&state, 0, sizeof(state));

	action_notify.button_action_notify.action_resp = (button_pressed) ?
								 SID_DEMO_ACTION_BUTTON_PRESSED :
								 SID_DEMO_ACTION_BUTTON_NOT_PRESSED;
	sid_get_time(app_context->sidewalk_handle, SID_GET_GPS_TIME, &curr_time);

	if (!button_pressed) {
		action_notify.temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS;
#if defined(CONFIG_BOARD_THINGY53_NRF5340_CPUAPP)
		if (device_is_ready(bme688_sensor_dev)) {
			struct sensor_value temp;
			sensor_sample_fetch(bme688_sensor_dev);

			sensor_channel_get(bme688_sensor_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
			action_notify.temperature = temp.val1;
		}
#else
		action_notify.temperature = sid_pal_temperature_get();
#endif
	} else {
		action_notify.temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_NOT_SUPPORTED;
		uint8_t temp_button_arr[DEMO_BUTTONS_MAX] = { 0 };
		uint8_t num_buttons_pressed = 0;
		for (size_t i = 0; i < DEMO_BUTTONS_MAX; i++) {
			if (sm_buttons_press_mask_bit_is_set(i) &&
			    !sm_buttons_notify_mask_bit_is_set(i)) {
				uint8_t *btn_id_array = sm_buttons_id_array_get();
				temp_button_arr[num_buttons_pressed] = btn_id_array[i];
				num_buttons_pressed += 1;
				sm_buttons_notify_mask_bit_set(i);
				sm_buttons_press_time_set(i, curr_time.tv_sec);
				sm_btn_press_timer_set_and_run(
					K_SECONDS(CONFIG_SM_TIMER_BUTTON_PRESS_CHECK_PERIOD_SECS),
					false);
			}
		}
		action_notify.button_action_notify.button_id_arr = temp_button_arr;
		action_notify.button_action_notify.num_buttons = num_buttons_pressed;
	}

	action_notify.gps_time_in_seconds = curr_time.tv_sec;

	link_type = sm_active_link_type_get(app_context);
	action_notify.link_type = link_type;

	sid_parse_state_init(&state, temp_buffer, sizeof(temp_buffer));
	sid_demo_app_action_notification_serialize(&state, &action_notify);
	if (state.ret_code != SID_ERROR_NONE) {
		LOG_ERR("Application action serialize failed -%d", state.ret_code);
		return;
	}

	struct sid_demo_msg_desc msg_desc = {
		.status_hdr_ind = false,
		.opc = SID_DEMO_MSG_TYPE_NOTIFY,
		.cmd_class = SID_DEMO_APP_CLASS,
		.cmd_id = SID_DEMO_APP_CLASS_CMD_ACTION,
	};

	struct sid_demo_msg demo_msg = { .payload = temp_buffer, .payload_size = state.offset };

	sid_parse_state_init(&state, app_context->buffer, PAYLOAD_MAX_SIZE);
	sid_demo_app_msg_serialize(&state, &msg_desc, &demo_msg);
	if (state.ret_code != SID_ERROR_NONE) {
		LOG_ERR("Demo msg notify sensor data serialize failed -%d", state.ret_code);
		return;
	}

	struct sid_msg msg = {
		.data = app_context->buffer,
		.size = state.offset,
	};
	struct sid_msg_desc desc = {
		.link_type = link_type,
		.type = SID_MSG_TYPE_NOTIFY,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

	if (button_pressed) {
		desc.msg_desc_attr.tx_attr.ttl_in_seconds = SID_DEMO_APP_TTL_MAX;
		desc.msg_desc_attr.tx_attr.num_retries = SID_DEMO_APP_RETRIES_MAX;
		desc.msg_desc_attr.tx_attr.request_ack = true;
		LOG_INF("Sending demo app message notify button press");
	} else {
		LOG_INF("Sending demo app message notify sensor data");
	}

	sm_send_msg(app_context, &desc, &msg);
}

void sm_check_button_press_notify(app_context_t *app_context)
{
	assert(app_context);

	struct sid_timespec curr_time = { 0 };
	uint16_t next_timer_schedule_secs = 0;

	sid_get_time(app_context->sidewalk_handle, SID_GET_GPS_TIME, &curr_time);

	for (size_t i = 0; i < DEMO_BUTTONS_MAX; i++) {
		if (sm_buttons_press_time_get(i)) {
			if (sm_buttons_notify_mask_bit_is_set(i)) {
				next_timer_schedule_secs =
					curr_time.tv_sec - sm_buttons_press_time_get(i);
				LOG_INF("Button press timeout pre check: button_notify_mask %x next_timer_schedule_secs %d",
					sm_buttons_notify_mask_get(), next_timer_schedule_secs);
				if (next_timer_schedule_secs >=
				    CONFIG_SM_TIMER_BUTTON_PRESS_CHECK_PERIOD_SECS) {
					sm_buttons_notify_mask_bit_clear(i);
					sm_buttons_press_mask_bit_clear(i);
					sm_buttons_press_time_set(i, 0);
					next_timer_schedule_secs = 0;
				}
			} else {
				sm_buttons_press_time_set(i, 0);
			}
		}
	}

	LOG_INF("Button press timeout post check: button_notify_mask %x next_timer_schedule_secs %d",
		sm_buttons_notify_mask_get(), next_timer_schedule_secs);

	if (next_timer_schedule_secs) {
		sm_btn_press_timer_set_and_run(K_SECONDS(next_timer_schedule_secs), true);
	}
}
