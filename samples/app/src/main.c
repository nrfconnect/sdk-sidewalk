/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>
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

#define APP_EVENT_SIDEWALK BIT(0)
#define APP_EVENT_STATE_UPDATE BIT(1)
#define APP_EVENT_SEND_HELLO BIT(2)
#define APP_EVENT_FACTORY_RESET BIT(3)
#define APP_EVENT_FSK_CSS_SWITCH BIT(4)
#define APP_EVENT_SET_DEVICE_PROFILE BIT(5)
#define APP_EVENT_ALL                                                                              \
	(APP_EVENT_SIDEWALK | APP_EVENT_STATE_UPDATE | APP_EVENT_SEND_HELLO |                      \
	 APP_EVENT_FACTORY_RESET | APP_EVENT_FSK_CSS_SWITCH | APP_EVENT_SET_DEVICE_PROFILE)

enum app_state {
	STATE_INIT,
	STATE_SIDEWALK_READY,
	STATE_SIDEWALK_NOT_READY,
	STATE_SIDEWALK_SECURE_CONNECTION
};

struct app_sm {
	struct smf_ctx ctx;
	struct k_event app_event;
	int32_t events;
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

static const struct smf_state app_states[] = {
	[STATE_INIT] = SMF_CREATE_STATE(NULL, state_init_run, NULL),
	[STATE_SIDEWALK_READY] = SMF_CREATE_STATE(NULL, state_ready_run, NULL),
	[STATE_SIDEWALK_NOT_READY] = SMF_CREATE_STATE(NULL, state_not_ready_run, NULL),
	[STATE_SIDEWALK_SECURE_CONNECTION] =
		SMF_CREATE_STATE(NULL, state_secure_connection_run, NULL),
};

static struct sid_ctx_s sid_ctx;

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_INF("%s", __func__);

	struct sid_ctx_s *ctx = (struct sid_ctx_s *)context;

	(void)k_event_post(&ctx->sm.app_event, APP_EVENT_SIDEWALK);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	LOG_INF("%s", __func__);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("%s", __func__);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	LOG_INF("%s", __func__);
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_INF("%s", __func__);
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	LOG_INF("%s", __func__);

	struct sid_ctx_s *ctx = (struct sid_ctx_s *)context;

	switch (status->state) {
	case SID_STATE_READY:
		smf_set_state(SMF_CTX(&ctx->sm), &app_states[SID_STATE_READY]);
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

	(void)k_event_post(&ctx->sm.app_event, APP_EVENT_STATE_UPDATE);
}

static void state_init_run(void *o)
{
	LOG_INF("state: init");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	if (ctx->sm.events & APP_EVENT_SEND_HELLO) {
		k_event_clear(&ctx->sm.app_event, APP_EVENT_SEND_HELLO);
		ctx->sm.events &= !APP_EVENT_SEND_HELLO;

		LOG_INF("event: hello");
	}

	if (ctx->sm.events & APP_EVENT_SIDEWALK) {
		k_event_clear(&ctx->sm.app_event, APP_EVENT_SIDEWALK);
		ctx->sm.events &= !APP_EVENT_SIDEWALK;

		LOG_INF("event: sidewalk");

		e = sid_process(ctx->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
	}

	if (ctx->sm.events & APP_EVENT_STATE_UPDATE) {
		k_event_clear(&ctx->sm.app_event, APP_EVENT_STATE_UPDATE);
		ctx->sm.events &= !APP_EVENT_STATE_UPDATE;

		LOG_INF("event: state update");
	}
}

static void state_ready_run(void *o)
{
	LOG_INF("state: ready");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	if (ctx->sm.events & APP_EVENT_SIDEWALK) {
		k_event_clear(&ctx->sm.app_event, APP_EVENT_SIDEWALK);
		ctx->sm.events &= !APP_EVENT_SIDEWALK;

		LOG_INF("event: sidewalk");

		e = sid_process(ctx->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
	}

	if (ctx->sm.events & APP_EVENT_STATE_UPDATE) {
		k_event_clear(&ctx->sm.app_event, APP_EVENT_STATE_UPDATE);
		ctx->sm.events &= !APP_EVENT_STATE_UPDATE;

		LOG_INF("event: state update");
	}
}

static void state_not_ready_run(void *o)
{
	LOG_INF("state: not ready");

	struct sid_ctx_s *ctx = CONTAINER_OF(o, struct sid_ctx_s, sm);
	sid_error_t e = SID_ERROR_NONE;

	if (ctx->sm.events & APP_EVENT_SIDEWALK) {
		k_event_clear(&ctx->sm.app_event, APP_EVENT_SIDEWALK);
		ctx->sm.events &= !APP_EVENT_SIDEWALK;

		LOG_INF("event: sidewalk");

		e = sid_process(ctx->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
	}

	if (ctx->sm.events & APP_EVENT_STATE_UPDATE) {
		k_event_clear(&ctx->sm.app_event, APP_EVENT_STATE_UPDATE);
		ctx->sm.events &= !APP_EVENT_STATE_UPDATE;

		LOG_INF("event: state update");
	}

	if (ctx->sm.events & APP_EVENT_SEND_HELLO) {
		k_event_clear(&ctx->sm.app_event, APP_EVENT_SEND_HELLO);
		ctx->sm.events &= !APP_EVENT_SEND_HELLO;

		LOG_INF("event: hello");
	}
}

static void state_secure_connection_run(void *o)
{
	LOG_INF("state: secure connection");
}

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	if (buttons & DK_BTN1_MSK) {
		LOG_INF("button 1");
		(void)k_event_post(&sid_ctx.sm.app_event, APP_EVENT_SEND_HELLO);
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

	k_event_init(&ctx->sm.app_event);
	smf_set_initial(SMF_CTX(&ctx->sm), &app_states[STATE_INIT]);

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

	while (1) {
		LOG_INF("waiting for event 0x%x...", ctx->sm.events);
		ctx->sm.events = k_event_wait(&ctx->sm.app_event, APP_EVENT_ALL, false, K_FOREVER);
		LOG_INF("processing event 0x%x", ctx->sm.events);

		if (smf_run_state(SMF_CTX(&ctx->sm))) {
			break;
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
