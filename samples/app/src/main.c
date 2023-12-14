/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>
#include <sid_hal_reset_ifc.h>
#include <pal_init.h>
#include <app_ble_config.h>
#include <app_subGHz_config.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);

static struct k_thread app_thread;
K_THREAD_STACK_DEFINE(app_thread_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);

typedef enum app_events {
	APP_EVENT_SIDEWALK,
	APP_EVENT_SEND_HELLO,
	APP_EVENT_FACTORY_RESET,
	APP_EVENT_FSK_CSS_SWITCH,
	APP_EVENT_SET_DEVICE_PROFILE
} app_event_t;

enum app_state {
	STATE_RUNNING,
	STATE_INIT,
	STATE_SIDEWALK_READY,
	STATE_SIDEWALK_NOT_READY,
	STATE_SIDEWALK_SECURE_CONNECTION
};

struct app_sm {
	struct smf_ctx ctx;
	struct k_msgq msgq;
	app_event_t event;
};

struct sid_ctx_s {
	struct app_sm sm;
	struct sid_handle *handle;
	struct sid_config config;
};

static void state_init_run(void *o);
static void state_ready_run(void *o);
static void state_not_ready_run(void *o);
static void state_secure_connection_run(void *o);
static void state_running_entry(void *o);
static void state_running_run(void *o);
static void state_running_exit(void *o);

static const struct smf_state app_states[] = {
	[STATE_RUNNING] =
		SMF_CREATE_STATE(state_running_entry, state_running_run, state_running_exit, NULL),
	[STATE_INIT] = SMF_CREATE_STATE(NULL, state_init_run, NULL, &app_states[STATE_RUNNING]),
	[STATE_SIDEWALK_READY] =
		SMF_CREATE_STATE(NULL, state_ready_run, NULL, &app_states[STATE_RUNNING]),
	[STATE_SIDEWALK_NOT_READY] =
		SMF_CREATE_STATE(NULL, state_not_ready_run, NULL, &app_states[STATE_RUNNING]),
	[STATE_SIDEWALK_SECURE_CONNECTION] = SMF_CREATE_STATE(NULL, state_secure_connection_run,
							      NULL, &app_states[STATE_RUNNING]),
};

uint8_t __aligned(4) msgq_buffer[CONFIG_SIDEWALK_THREAD_QUEUE_SIZE * sizeof(app_event_t)];

static struct sid_ctx_s sid_ctx;

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_INF("%s", __func__);

	struct sid_ctx_s *ctx = (struct sid_ctx_s *)context;
	static app_event_t sid_event = APP_EVENT_SIDEWALK;

	if (k_msgq_put(&ctx->sm.msgq, (void *)&sid_event, K_NO_WAIT)) {
		LOG_ERR("Cannot put message");
	}
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	LOG_INF("%s", __func__);

	LOG_DBG("received message(type: %d, link_mode: %d, id: %u size %u)", (int)msg_desc->type,
		(int)msg_desc->link_mode, msg_desc->id, msg->size);
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("%s", __func__);
	LOG_INF("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	LOG_INF("%s", __func__);
	LOG_ERR("failed to send message(type: %d, id: %u), err:%d", (int)msg_desc->type,
		msg_desc->id, (int)error);
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_INF("%s", __func__);

	ARG_UNUSED(context);

	LOG_INF("factory reset notification received from sid api");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Reboot type not supported");
	}
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	LOG_INF("%s", __func__);

	struct sid_ctx_s *ctx = (struct sid_ctx_s *)context;
	static app_event_t sid_status_event = APP_EVENT_SIDEWALK;

	switch (status->state) {
	case SID_STATE_READY:
		smf_set_state(SMF_CTX(&ctx->sm), &app_states[STATE_SIDEWALK_READY]);
		break;
	case SID_STATE_NOT_READY:
		smf_set_state(SMF_CTX(&ctx->sm), &app_states[STATE_SIDEWALK_NOT_READY]);
		break;
	case SID_STATE_ERROR:
		LOG_ERR("sidewalk error: %d", (int)sid_get_error(ctx->handle));
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		smf_set_state(SMF_CTX(&ctx->sm), &app_states[STATE_SIDEWALK_SECURE_CONNECTION]);
		break;
	default:
		LOG_ERR("sidewalk unknow state: %d", status->state);
		break;
	}

	if (k_msgq_put(&ctx->sm.msgq, (void *)&sid_status_event, K_NO_WAIT)) {
		LOG_ERR("Cannot put message");
	}

	LOG_INF("Device %sregistered, Time Sync %s, Link status: {BLE: %s, FSK: %s, LoRa: %s}",
		(SID_STATUS_REGISTERED == status->detail.registration_status) ? "Is " : "Un",
		(SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) ? "Success" : "Fail",
		(status->detail.link_status_mask & SID_LINK_TYPE_1) ? "Up" : "Down",
		(status->detail.link_status_mask & SID_LINK_TYPE_2) ? "Up" : "Down",
		(status->detail.link_status_mask & SID_LINK_TYPE_3) ? "Up" : "Down");

	for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
		enum sid_link_mode mode =
			(enum sid_link_mode)status->detail.supported_link_modes[i];

		if (mode) {
			LOG_INF("Link mode on %s = {Cloud: %s, Mobile: %s}",
				(SID_LINK_TYPE_1_IDX == i) ? "BLE" :
				(SID_LINK_TYPE_2_IDX == i) ? "FSK" :
				(SID_LINK_TYPE_3_IDX == i) ? "LoRa" :
							     "unknow",
				(mode & SID_LINK_MODE_CLOUD) ? "True" : "False",
				(mode & SID_LINK_MODE_MOBILE) ? "True" : "False");
		}
	}
}

static void state_running_entry(void *o)
{
	LOG_INF("state: running (entry)");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	e = application_pal_init();
	if (e) {
		LOG_ERR("pal init err %d", (int)e);
	}
	e = sid_init(&ctx->config, &ctx->handle);
	if (e) {
		LOG_ERR("sid init err %d", (int)e);
	}
	e = sid_start(ctx->handle, SID_LINK_TYPE_1);
	if (e) {
		LOG_ERR("sid start err %d", (int)e);
	}
}

static void state_running_run(void *o)
{
	LOG_INF("state: running");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	switch (ctx->sm.event) {
	case APP_EVENT_SIDEWALK:
		LOG_INF("event: sidewalk");

		e = sid_process(ctx->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
		break;
	case APP_EVENT_FACTORY_RESET:
		LOG_INF("event: factory reset");

		e = sid_set_factory_reset(ctx->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
		break;
	default:
		LOG_ERR("event: unknow %d", ctx->sm.event);
		break;
	}
}

static void state_running_exit(void *o)
{
	LOG_INF("state: running (exit)");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	e = sid_deinit(ctx->handle);
	if (e) {
		LOG_ERR("sid deinit err %d", (int)e);
	}
}

static void state_init_run(void *o)
{
	LOG_INF("state: init");
}

static void state_ready_run(void *o)
{
	LOG_INF("state: ready");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	switch (ctx->sm.event) {
	case APP_EVENT_SIDEWALK:
		break;
	case APP_EVENT_SEND_HELLO:
		LOG_INF("event: hello");

		static uint8_t counter = 0;
		static struct sid_msg msg =
			(struct sid_msg){ .data = (uint8_t *)&counter, .size = sizeof(uint8_t) };
		static struct sid_msg_desc desc = (struct sid_msg_desc){
			.type = SID_MSG_TYPE_NOTIFY,
			.link_type = SID_LINK_TYPE_ANY,
			.link_mode = SID_LINK_MODE_CLOUD,
		};

		e = sid_put_msg(ctx->handle, &msg, &desc);
		if (e) {
			LOG_ERR("sid send err %d", (int)e);
		}
		break;
	default:
		LOG_ERR("event: unknow %d", ctx->sm.event);
		break;
	}
}

static void state_not_ready_run(void *o)
{
	LOG_INF("state: not ready");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	switch (ctx->sm.event) {
	case APP_EVENT_SIDEWALK:
		break;
	case APP_EVENT_SEND_HELLO:
		LOG_INF("event: hello");

		e = sid_ble_bcn_connection_request(ctx->handle, true);
		if (e) {
			LOG_ERR("conn req err %d", (int)e);
		}
		break;
	default:
		LOG_ERR("event: unknow %d", ctx->sm.event);
		break;
	}
}

static void state_secure_connection_run(void *o)
{
	LOG_INF("state: secure connection");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	switch (ctx->sm.event) {
	case APP_EVENT_SIDEWALK:
		break;
	case APP_EVENT_SEND_HELLO:
		LOG_INF("event: hello");

		static uint8_t counter = 0;
		static struct sid_msg msg =
			(struct sid_msg){ .data = (uint8_t *)&counter, .size = sizeof(uint8_t) };
		static struct sid_msg_desc desc = (struct sid_msg_desc){
			.type = SID_MSG_TYPE_NOTIFY,
			.link_type = SID_LINK_TYPE_ANY,
			.link_mode = SID_LINK_MODE_CLOUD,
		};

		e = sid_put_msg(ctx->handle, &msg, &desc);
		if (e) {
			LOG_ERR("sid send err %d", (int)e);
		}
		break;
	default:
		LOG_ERR("event: unknow %d", ctx->sm.event);
		break;
	}
}

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	if (buttons & DK_BTN1_MSK) {
		LOG_INF("button 1");
		static app_event_t hello_event = APP_EVENT_SEND_HELLO;
		if (k_msgq_put(&sid_ctx.sm.msgq, (void *)&hello_event, K_NO_WAIT)) {
			LOG_ERR("Cannot put message");
		}
	}
	if (buttons & DK_BTN4_MSK) {
		LOG_INF("button 4");
		static app_event_t reset_event = APP_EVENT_FACTORY_RESET;
		if (k_msgq_put(&sid_ctx.sm.msgq, (void *)&reset_event, K_NO_WAIT)) {
			LOG_ERR("Cannot put message");
		}
	}
}

static void app_thread_entry(void *context, void *unused, void *unused2)
{
	LOG_INF("App");

	ARG_UNUSED(unused);
	ARG_UNUSED(unused2);

	struct sid_ctx_s *ctx = (struct sid_ctx_s *)context;

	struct sid_event_callbacks event_callbacks = {
		.context = ctx,
		.on_event = on_sidewalk_event,
		.on_msg_received = on_sidewalk_msg_received,
		.on_msg_sent = on_sidewalk_msg_sent,
		.on_send_error = on_sidewalk_send_error,
		.on_status_changed = on_sidewalk_status_changed,
		.on_factory_reset = on_sidewalk_factory_reset,
	};

	ctx->config = (struct sid_config){
		.link_mask = SID_LINK_TYPE_1,
		.callbacks = &event_callbacks,
		.time_sync_periodicity_seconds = 7200,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	k_msgq_init(&ctx->sm.msgq, msgq_buffer, sizeof(app_event_t),
		    CONFIG_SIDEWALK_THREAD_QUEUE_SIZE);
	smf_set_initial(SMF_CTX(&ctx->sm), &app_states[STATE_INIT]);

	while (1) {
		LOG_INF("waiting for event...");
		int err = k_msgq_get(&ctx->sm.msgq, &ctx->sm.event, K_FOREVER);
		if (!err) {
			if (smf_run_state(SMF_CTX(&ctx->sm))) {
				break;
			}
		} else {
			LOG_ERR("msgq err %d", err);
		}
	}

	LOG_ERR("Application failed. You should never see this message.");
}

int main(void)
{
	LOG_INF("Main");

	if (dk_buttons_init(button_changed)) {
		LOG_ERR("Cannot init buttons");
	}

	(void)k_thread_create(&app_thread, app_thread_stack,
			      K_THREAD_STACK_SIZEOF(app_thread_stack), app_thread_entry, &sid_ctx,
			      NULL, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&app_thread, "app");

	return 0;
}
