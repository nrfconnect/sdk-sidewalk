/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sensor_monitoring/app_tx.h>
#include <sensor_monitoring/app_buttons.h>
#include <sensor_monitoring/app_leds.h>
#include <sensor_monitoring/app_sensor.h>
#include <sidewalk.h>
#include <sid_demo_parser.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_hal_memory_ifc.h>
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_tx, CONFIG_SIDEWALK_LOG_LEVEL);

#define MSG_PAYLOAD_SIZE_MAX (32)
#define DEMO_MSG_PAYLOAD_MAX_SIXE (255)
#define APP_SID_MSG_TTL_MAX (60)
#define APP_SID_MSG_RETRIES_MAX (3)
#define APP_NOTIFY_BUTTON_PERIOD_MS (1000)
#define APP_DUMMY_SENSOR_DATA (20)

typedef struct app_sm_s {
	struct smf_ctx ctx;
	struct k_msgq msgq;
	app_event_t event;
} app_sm_t;

static app_sm_t app_sm;
static uint32_t last_link_mask;

enum state {
	STATE_APP_INIT,
	STATE_APP_NOTIFY_CAPABILITY,
	STATE_APP_NOTIFY_DATA,
};

static void state_init(void *o);
static void state_notify_capability(void *o);
static void state_notify_data(void *o);
static void button_timer_cb(struct k_timer *timer_id);

static const struct smf_state app_states[] = {
	[STATE_APP_INIT] = SMF_CREATE_STATE(NULL, state_init, NULL, NULL, NULL),
	[STATE_APP_NOTIFY_CAPABILITY] =
		SMF_CREATE_STATE(NULL, state_notify_capability, NULL, NULL, NULL),
	[STATE_APP_NOTIFY_DATA] = SMF_CREATE_STATE(NULL, state_notify_data, NULL, NULL, NULL),
};

static uint8_t __aligned(4)
	app_msgq_buff[CONFIG_SID_END_DEVICE_TX_THREAD_QUEUE_SIZE * sizeof(app_event_t)];

K_TIMER_DEFINE(button_timer, button_timer_cb, NULL);

static void button_timer_cb(struct k_timer *timer_id)
{
	ARG_UNUSED(timer_id);
	if (app_btn_pending_flag_get()) {
		app_tx_event_send(APP_EVENT_NOTIFY_BUTTON);
	}
}

static uint32_t time_in_sec_get(void)
{
	struct sid_timespec curr_time = { 0 };
	sid_error_t e = sid_pal_uptime_now(&curr_time);
	if (e) {
		LOG_INF("Uptime get failed %d", e);
	}

	return curr_time.tv_sec;
}

static uint32_t last_link_mask_get(void)
{
	return last_link_mask;
}

static int app_tx_demo_msg_send(struct sid_parse_state *state, uint8_t *buffer,
				struct sid_demo_msg_desc *demo_desc, struct sid_msg_desc *sid_desc)
{
	// Serialize demo message
	struct sid_demo_msg demo_msg = { .payload = buffer, .payload_size = state->offset };
	uint8_t msg_buffer[DEMO_MSG_PAYLOAD_MAX_SIXE] = { 0 };
	sid_parse_state_init(state, msg_buffer, sizeof(msg_buffer));
	sid_demo_app_msg_serialize(state, demo_desc, &demo_msg);
	if (state->ret_code != SID_ERROR_NONE) {
		LOG_DBG("Demo msg serialize failed -%d", state->ret_code);
		return -EINVAL;
	}

	// Send sidewalk message
	sidewalk_msg_t *sid_msg = sid_hal_malloc(sizeof(sidewalk_msg_t));
	if (!sid_msg) {
		LOG_ERR("Failed to alloc memory for message context");
		return -ENOMEM;
	}
	memset(sid_msg, 0x0, sizeof(*sid_msg));
	sid_msg->msg.size = state->offset;
	sid_msg->msg.data = sid_hal_malloc(sid_msg->msg.size);
	if (!sid_msg->msg.data) {
		sid_hal_free(sid_msg);
		LOG_ERR("Failed to allocate memory for message data");
		return -ENOMEM;
	}
	memcpy(sid_msg->msg.data, msg_buffer, sid_msg->msg.size);
	memcpy(&sid_msg->desc, sid_desc, sizeof(struct sid_msg_desc));

	int err = sidewalk_event_send(SID_EVENT_SEND_MSG, sid_msg);
	if (err) {
		sid_hal_free(sid_msg->msg.data);
		sid_hal_free(sid_msg);
		LOG_ERR("Event send err %d", err);
		return -EIO;
	};

	return 0;
}

static int app_tx_response_send(struct sid_demo_action_resp *resp)
{
	// Prepare message
	struct sid_demo_msg_desc resp_desc = {
		.status_hdr_ind = true,
		.opc = SID_DEMO_MSG_TYPE_RESP,
		.cmd_class = SID_DEMO_APP_CLASS,
		.cmd_id = SID_DEMO_APP_CLASS_CMD_ACTION,
		.status_code = SID_ERROR_NONE,
	};

	struct sid_msg_desc resp_sid_desc = { .link_type = last_link_mask_get(),
					      .type = SID_MSG_TYPE_NOTIFY,
					      .link_mode = SID_LINK_MODE_CLOUD,
					      .msg_desc_attr = {
						      .tx_attr = {
							      .ttl_in_seconds = APP_SID_MSG_TTL_MAX,
							      .num_retries =
								      APP_SID_MSG_RETRIES_MAX,
							      .request_ack = true,
						      } } };

	// Serialize and send
	struct sid_parse_state state = { 0 };
	uint8_t msg_buffer[MSG_PAYLOAD_SIZE_MAX] = { 0 };
	sid_parse_state_init(&state, msg_buffer, sizeof(msg_buffer));
	sid_demo_app_action_resp_serialize(&state, resp);
	if (state.ret_code != SID_ERROR_NONE) {
		LOG_DBG("Action response serialize failed -%d", state.ret_code);
		return -EINVAL;
	}

	return app_tx_demo_msg_send(&state, msg_buffer, &resp_desc, &resp_sid_desc);
}

static void state_init(void *o)
{
	app_sm_t *sm = (app_sm_t *)o;

	switch (sm->event) {
	case APP_EVENT_TIME_SYNC_SUCCESS:
		smf_set_state(SMF_CTX(sm), &app_states[STATE_APP_NOTIFY_CAPABILITY]);
		break;
	case APP_EVENT_CAPABILITY_SUCCESS:
	case APP_EVENT_TIME_SYNC_FAIL:
	case APP_EVENT_NOTIFY_BUTTON:
	case APP_EVENT_NOTIFY_SENSOR:
	case APP_EVENT_RESP_LED_ON:
	case APP_EVENT_RESP_LED_OFF:
		break;
	}
}

static void state_notify_capability(void *o)
{
	app_sm_t *sm = (app_sm_t *)o;

	switch (sm->event) {
	case APP_EVENT_NOTIFY_SENSOR: {
		// Prepare message
		struct sid_demo_capability_discovery cap = {
			.link_type = last_link_mask_get(),
			.temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS,
			.button_id_arr = app_btn_id_array_get(),
			.num_buttons = APP_BUTTONS_MAX,
			.led_id_arr = app_led_id_array_get(),
			.num_leds = APP_LEDS_MAX,
		};

		struct sid_demo_msg_desc cap_desc = {
			.status_hdr_ind = false,
			.opc = SID_DEMO_MSG_TYPE_NOTIFY,
			.cmd_class = SID_DEMO_APP_CLASS,
			.cmd_id = SID_DEMO_APP_CLASS_CMD_CAP_DISCOVERY_ID,
		};

		struct sid_msg_desc cap_sid_desc = {
			.link_type = last_link_mask_get(),
			.type = SID_MSG_TYPE_NOTIFY,
			.link_mode = SID_LINK_MODE_CLOUD,
		};

		// Serialize and send
		struct sid_parse_state state = { 0 };
		uint8_t msg_buffer[MSG_PAYLOAD_SIZE_MAX] = { 0 };
		sid_parse_state_init(&state, msg_buffer, sizeof(msg_buffer));
		sid_demo_app_capability_discovery_notification_serialize(&state, &cap);
		if (state.ret_code != SID_ERROR_NONE) {
			LOG_ERR("Capability serialize failed %d", state.ret_code);
			break;
		}

		int err = app_tx_demo_msg_send(&state, msg_buffer, &cap_desc, &cap_sid_desc);
		if (err) {
			LOG_ERR("Capability send failed %d", err);
		}

		LOG_INF("Capability send");
	} break;
	case APP_EVENT_CAPABILITY_SUCCESS:
		smf_set_state(SMF_CTX(sm), &app_states[STATE_APP_NOTIFY_DATA]);
		break;
	case APP_EVENT_NOTIFY_BUTTON:
	case APP_EVENT_RESP_LED_ON:
	case APP_EVENT_RESP_LED_OFF:
		LOG_WRN("Operation not supported, waiting for capability response.");
		break;
	case APP_EVENT_TIME_SYNC_SUCCESS:
	case APP_EVENT_TIME_SYNC_FAIL:
		break;
	}
}

static void state_notify_data(void *o)
{
	app_sm_t *sm = (app_sm_t *)o;

	struct sid_parse_state state = { 0 };
	uint8_t msg_buffer[MSG_PAYLOAD_SIZE_MAX] = { 0 };
	int err = 0;

	switch (sm->event) {
	case APP_EVENT_NOTIFY_BUTTON: {
		// Read button state
		uint8_t button_arr[APP_BUTTONS_MAX] = { 0 };
		uint8_t num_buttons = 0;
		for (size_t i = 0; i < APP_BUTTONS_MAX; i++) {
			if (app_btn_press_mask_bit_is_set(i) &&
			    !app_btn_notify_mask_bit_is_set(i)) {
				uint8_t *btn_id_array = app_btn_id_array_get();
				button_arr[num_buttons] = btn_id_array[i];
				num_buttons += 1;
			}
		}

		// Prepare message
		struct sid_demo_action_notification notify_btn = {
			.gps_time_in_seconds = time_in_sec_get(),
			.link_type = last_link_mask_get(),
			.temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_NOT_SUPPORTED,
			.button_action_notify.action_resp = SID_DEMO_ACTION_BUTTON_PRESSED,
			.button_action_notify.button_id_arr = button_arr,
			.button_action_notify.num_buttons = num_buttons,
		};

		struct sid_demo_msg_desc notify_btn_desc = {
			.status_hdr_ind = false,
			.opc = SID_DEMO_MSG_TYPE_NOTIFY,
			.cmd_class = SID_DEMO_APP_CLASS,
			.cmd_id = SID_DEMO_APP_CLASS_CMD_ACTION,
		};

		struct sid_msg_desc notify_btn_sid_desc = {
			.link_type = last_link_mask_get(),
			.type = SID_MSG_TYPE_NOTIFY,
			.link_mode = SID_LINK_MODE_CLOUD,
			.msg_desc_attr.tx_attr.ttl_in_seconds = APP_SID_MSG_TTL_MAX,
			.msg_desc_attr.tx_attr.num_retries = APP_SID_MSG_RETRIES_MAX,
			.msg_desc_attr.tx_attr.request_ack = true,
		};

		// Serialize and send
		sid_parse_state_init(&state, msg_buffer, sizeof(msg_buffer));
		sid_demo_app_action_notification_serialize(&state, &notify_btn);
		if (state.ret_code != SID_ERROR_NONE) {
			LOG_ERR("Notify button serialize failed -%d", state.ret_code);
			break;
		}

		err = app_tx_demo_msg_send(&state, msg_buffer, &notify_btn_desc,
					   &notify_btn_sid_desc);
		if (err) {
			LOG_ERR("Notify button send failed %d", err);
			break;
		}

		app_btn_pending_flag_clear();
		LOG_INF("Notify button send");
	} break;
	case APP_EVENT_NOTIFY_SENSOR: {
		// Read sensor data
		int16_t temp = 0;
		err = app_sensor_temperature_get(&temp);
		if (err) {
			LOG_INF("Temperature get err %d, use dummy value: %d", err,
				APP_DUMMY_SENSOR_DATA);
			temp = APP_DUMMY_SENSOR_DATA;
		}

		// Prepare message
		struct sid_demo_action_notification notify_temp = {
			.gps_time_in_seconds = time_in_sec_get(),
			.link_type = last_link_mask_get(),
			.temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS,
			.temperature = temp,
			.button_action_notify.action_resp = SID_DEMO_ACTION_BUTTON_NOT_PRESSED,
		};

		struct sid_demo_msg_desc notify_desc = {
			.status_hdr_ind = false,
			.opc = SID_DEMO_MSG_TYPE_NOTIFY,
			.cmd_class = SID_DEMO_APP_CLASS,
			.cmd_id = SID_DEMO_APP_CLASS_CMD_ACTION,
		};

		struct sid_msg_desc notify_sid_desc = {
			.link_type = last_link_mask_get(),
			.type = SID_MSG_TYPE_NOTIFY,
			.link_mode = SID_LINK_MODE_CLOUD,
		};

		// Serialize and send
		sid_parse_state_init(&state, msg_buffer, sizeof(msg_buffer));
		sid_demo_app_action_notification_serialize(&state, &notify_temp);
		if (state.ret_code != SID_ERROR_NONE) {
			LOG_ERR("Notify sensor serialize failed -%d", state.ret_code);
			break;
		}

		err = app_tx_demo_msg_send(&state, msg_buffer, &notify_desc, &notify_sid_desc);
		if (err) {
			LOG_ERR("Notify sensor send failed %d", err);
		}

		LOG_INF("Notify sensor send");
	} break;
	case APP_EVENT_RESP_LED_ON: {
		// Read led status
		uint8_t led_on_arr[APP_BUTTONS_MAX] = { 0 };
		uint8_t num_leds_on = 0;
		for (uint8_t i = 0; i < APP_LEDS_MAX; i++) {
			bool is_on = app_led_is_on((enum leds_id_t)i);
			if (is_on) {
				led_on_arr[num_leds_on] = i;
				num_leds_on++;
			}
		}

		// Prepare message
		struct sid_demo_action_resp resp_led_on = {
			.gps_time_in_seconds = time_in_sec_get(),
			.resp_type = SID_DEMO_ACTION_TYPE_LED,
			.led_action_resp.action_resp = SID_DEMO_ACTION_LED_ON,
			.led_action_resp.num_leds = num_leds_on,
			.led_action_resp.led_id_arr = led_on_arr,
		};

		err = app_tx_response_send(&resp_led_on);
		if (err) {
			LOG_ERR("Response LED ON send failed %d", err);
		}

		LOG_INF("Response LED ON send");
	} break;
	case APP_EVENT_RESP_LED_OFF: {
		// Read led status
		uint8_t led_off_arr[APP_BUTTONS_MAX] = { 0 };
		uint8_t num_leds_off = 0;
		for (uint8_t i = 0; i < APP_LEDS_MAX; i++) {
			bool is_off = !app_led_is_on((enum leds_id_t)i);
			if (is_off) {
				led_off_arr[num_leds_off] = i;
				num_leds_off++;
			}
		}

		// Prepare response message
		struct sid_demo_action_resp resp_led_off = {
			.gps_time_in_seconds = time_in_sec_get(),
			.resp_type = SID_DEMO_ACTION_TYPE_LED,
			.led_action_resp.action_resp = SID_DEMO_ACTION_LED_OFF,
			.led_action_resp.num_leds = num_leds_off,
			.led_action_resp.led_id_arr = led_off_arr,
		};
		err = app_tx_response_send(&resp_led_off);
		if (err) {
			LOG_ERR("Response LED ON send failed %d", err);
		}

		LOG_INF("Response LED OFF send");
	} break;
	case APP_EVENT_TIME_SYNC_FAIL:
		smf_set_state(SMF_CTX(sm), &app_states[STATE_APP_NOTIFY_CAPABILITY]);
	case APP_EVENT_TIME_SYNC_SUCCESS:
	case APP_EVENT_CAPABILITY_SUCCESS:
		break;
	}
}

void app_tx_last_link_mask_set(uint32_t link_mask)
{
	if (link_mask) {
		last_link_mask = link_mask;
	}
}

int app_tx_event_send(app_event_t event)
{
	return k_msgq_put(&app_sm.msgq, (void *)&event, K_NO_WAIT);
}

void app_tx_task(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	k_timer_start(&button_timer, K_MSEC(APP_NOTIFY_BUTTON_PERIOD_MS),
		      K_MSEC(APP_NOTIFY_BUTTON_PERIOD_MS));

	k_msgq_init(&app_sm.msgq, (char *)app_msgq_buff, sizeof(app_event_t),
		    CONFIG_SID_END_DEVICE_TX_THREAD_QUEUE_SIZE);
	smf_set_initial(SMF_CTX(&app_sm), &app_states[STATE_APP_INIT]);

	while (1) {
		int err = k_msgq_get(&app_sm.msgq, &app_sm.event, K_FOREVER);
		if (!err) {
			if (smf_run_state(SMF_CTX(&app_sm))) {
				LOG_ERR("App TX state machine termination");
				break;
			}
		} else {
			LOG_ERR("App TX msgq err %d", err);
		}
	}

	LOG_ERR("App TX thread ends. You should never see this message.");
}
