/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sm_leds.h>
#include <assert.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_leds, CONFIG_SIDEWALK_LOG_LEVEL);

#define DEMO_LEDS_MAX 4
#define LED_ACTION_REPONSE_PAYLOAD_SIZE_MAX 32

static int leds_state;
static uint8_t led_id_arr[DEMO_LEDS_MAX];

enum leds_id_t {
	LED_ID_0        = 0,
	LED_ID_1        = 1,
	LED_ID_2        = 2,
	LED_ID_3        = 3,
	LED_ID_LAST,
};

static void turn_on_all_leds()
{
	dk_set_leds(DK_ALL_LEDS_MSK);
	leds_state = DK_ALL_LEDS_MSK;
}

static void turn_off_all_leds()
{
	dk_set_leds(DK_NO_LEDS_MSK);
	leds_state = DK_NO_LEDS_MSK;
}

static void turn_on_led(enum leds_id_t id)
{
	if (id < LED_ID_LAST) {
		dk_set_led_on(id);
		WRITE_BIT(leds_state, id, true);
	}
}

static void turn_off_led(enum leds_id_t id)
{
	if (id < LED_ID_LAST) {
		dk_set_led_off(id);
		WRITE_BIT(leds_state, id, false);
	}
}

static bool is_led_on(enum leds_id_t id)
{
	if (id < LED_ID_LAST) {
		return (leds_state & BIT(id));
	}
	return false;
}

static void send_led_response(app_context_t *app_context, struct sid_demo_action_resp *resp)
{
	struct sid_parse_state state;
	enum sid_link_type link_type = sm_active_link_type_get(app_context);
	uint8_t temp_buffer[LED_ACTION_REPONSE_PAYLOAD_SIZE_MAX] = { 0 };

	memset(&state, 0, sizeof(state));

	sid_parse_state_init(&state, temp_buffer, sizeof(temp_buffer));
	sid_demo_app_action_resp_serialize(&state, resp);
	if (state.ret_code != SID_ERROR_NONE) {
		LOG_ERR("Action response serialize failed -%d", state.ret_code);
		return;
	}

	struct sid_demo_msg_desc msg_desc = {
		.status_hdr_ind = true,
		.opc = SID_DEMO_MSG_TYPE_RESP,
		.cmd_class = SID_DEMO_APP_CLASS,
		.cmd_id = SID_DEMO_APP_CLASS_CMD_ACTION,
		.status_code = SID_ERROR_NONE,
	};

	struct sid_demo_msg demo_msg = { .payload = temp_buffer, .payload_size = state.offset };

	sid_parse_state_init(&state, app_context->buffer, PAYLOAD_MAX_SIZE);
	sid_demo_app_msg_serialize(&state, &msg_desc, &demo_msg);
	if (state.ret_code != SID_ERROR_NONE) {
		LOG_ERR("Data serialize failed -%d", state.ret_code);
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
		.msg_desc_attr = {
			.tx_attr = {
				.ttl_in_seconds = SID_DEMO_APP_TTL_MAX,
				.num_retries = SID_DEMO_APP_RETRIES_MAX,
				.request_ack = true,
			}
		}
	};

	LOG_INF("Sending led response");
	sm_send_msg(app_context, &desc, &msg);
}

static void turn_all_leds_process(app_context_t *app_context, enum sid_demo_led_action led_action_req,
				  struct sid_demo_action_resp *action_resp)
{
	if (led_action_req == SID_DEMO_ACTION_LED_ON) {
		turn_on_all_leds();
	} else {
		turn_off_all_leds();
	}

	action_resp->led_action_resp.action_resp = led_action_req;

	for (size_t i = 0; i < DEMO_LEDS_MAX; i++) {
		bool result = (led_action_req == SID_DEMO_ACTION_LED_ON) ? is_led_on((enum leds_id_t)led_id_arr[i]) :
			      !is_led_on((enum leds_id_t)led_id_arr[i]);
		if (result) {
			action_resp->led_action_resp.num_leds += 1;
			action_resp->led_action_resp.led_id_arr[i] = led_id_arr[i];
		}
	}

	if (action_resp->led_action_resp.num_leds) {
		send_led_response(app_context, action_resp);
	} else {
		LOG_ERR("LED response invalid num leds %d", action_resp->led_action_resp.num_leds);
	}
}

static void turn_leds_process(app_context_t *app_context, struct sid_demo_led_action_req *led_req,
			      struct sid_demo_action_resp *action_resp)
{
	for (uint8_t i = 0; i < led_req->num_leds; i++) {
		if (led_req->action_req == SID_DEMO_ACTION_LED_ON) {
			turn_on_led((enum leds_id_t)led_req->led_id_arr[i]);
		} else {
			turn_off_led((enum leds_id_t)led_req->led_id_arr[i]);
		}
	}

	action_resp->led_action_resp.action_resp = led_req->action_req;

	for (uint8_t i = 0; i < led_req->num_leds; i++) {
		bool result = (led_req->action_req == SID_DEMO_ACTION_LED_ON) ? is_led_on(
			(enum leds_id_t)led_req->led_id_arr[i]) :
			      !is_led_on((enum leds_id_t)led_req->led_id_arr[i]);
		if (result) {
			action_resp->led_action_resp.num_leds += 1;
			action_resp->led_action_resp.led_id_arr[i] = led_req->led_id_arr[i];
		}
	}

	if (action_resp->led_action_resp.num_leds) {
		send_led_response(app_context, action_resp);
	} else {
		LOG_ERR("LED response invalid num leds %d", action_resp->led_action_resp.num_leds);
	}
}

void sm_leds_action_request_process(app_context_t *app_context, struct sid_parse_state *state)
{
	assert(app_context);
	assert(state);

	uint8_t temp_led_id_arr[DEMO_LEDS_MAX] = { 0 };
	struct sid_demo_led_action_req led_req = { .led_id_arr = temp_led_id_arr };

	sid_demo_app_action_req_deserialize(state, &led_req);

	if (state->ret_code != SID_ERROR_NONE) {
		LOG_ERR("de-serialize led action req failed %d", state->ret_code);
	} else if (led_req.action_req == SID_DEMO_ACTION_LED_ON || led_req.action_req == SID_DEMO_ACTION_LED_OFF) {
		uint8_t temp_led_id_arr_resp[DEMO_LEDS_MAX] = { 0 };
		struct sid_demo_action_resp action_resp = { .resp_type = SID_DEMO_ACTION_TYPE_LED, };
		action_resp.led_action_resp.led_id_arr = temp_led_id_arr_resp;
		struct sid_timespec curr_time = { 0 };

		sid_get_time(app_context->sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
		action_resp.gps_time_in_seconds = curr_time.tv_sec;

		if ((curr_time.tv_sec - led_req.gps_time_in_seconds) > 0) {
			action_resp.down_link_latency_secs = curr_time.tv_sec - led_req.gps_time_in_seconds;
		}

		if (led_req.num_leds == 0xFF) {
			turn_all_leds_process(app_context, led_req.action_req, &action_resp);
		} else if (led_req.num_leds <= DEMO_LEDS_MAX) {
			turn_leds_process(app_context, &led_req, &action_resp);
		} else {
			LOG_ERR("Invalid led action req max allowed %d received %d", DEMO_LEDS_MAX,
				led_req.num_leds);
		}
	} else {
		LOG_ERR("Invalid led action request %d", led_req.action_req);
	}
}

size_t sm_leds_id_array_size_get()
{
	return ARRAY_SIZE(led_id_arr);
}

uint8_t *sm_leds_id_array_get()
{
	return led_id_arr;
}

int sm_leds_init()
{
	for (size_t i = 0; i < DEMO_LEDS_MAX; i++) {
		led_id_arr[i] = i;
	}
	return dk_leds_init();
}
