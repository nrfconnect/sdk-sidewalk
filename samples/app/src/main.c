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

/* Sidewalk Thread*/
K_THREAD_STACK_DEFINE(sidewalk_work_q_stack, KB(10));

struct sid_ctx_s {
	struct k_work sid_event_work;
	struct k_work_q sid_work_q;
	struct sid_handle *sid_handle;
	struct sid_config config;
};

static struct sid_ctx_s sid_ctx = { 0 };

static void on_sidewalk_event(bool in_isr, void *context);
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context);
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context);
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context);
static void on_sidewalk_factory_reset(void *context);
static void on_sidewalk_status_changed(const struct sid_status *status, void *context);

static struct sid_event_callbacks event_callbacks = {
	.context = &sid_ctx,
	.on_event = on_sidewalk_event,
	.on_msg_received = on_sidewalk_msg_received,
	.on_msg_sent = on_sidewalk_msg_sent,
	.on_send_error = on_sidewalk_send_error,
	.on_status_changed = on_sidewalk_status_changed,
	.on_factory_reset = on_sidewalk_factory_reset,
};

static void sidewalk_event_worker(struct k_work *work)
{
	struct sid_ctx_s *sid_ctx = CONTAINER_OF(work, struct sid_ctx_s, sid_event_work);

	sid_error_t e = sid_process(sid_ctx->sid_handle);

	if (e) {
		LOG_ERR("sid process error %d", e);
	}
}

static void on_sidewalk_event(bool in_isr, void *context)
{
	struct sid_ctx_s *ctx = (struct sid_ctx_s *)context;

	int e = k_work_submit_to_queue(&ctx->sid_work_q, &ctx->sid_event_work);

	if (e) {
		LOG_ERR("sid process queue error %d", e);
	}
}

int sidewalk_thread_init(void *context)
{
	struct sid_ctx_s *ctx = (struct sid_ctx_s *)context;

	k_work_queue_init(&ctx->sid_work_q);
	k_work_queue_start(&ctx->sid_work_q, sidewalk_work_q_stack,
			   K_THREAD_STACK_SIZEOF(sidewalk_work_q_stack),
			   CONFIG_SIDEWALK_THREAD_PRIORITY, NULL);
	k_work_init(&ctx->sid_event_work, sidewalk_event_worker);

	ctx->config = (struct sid_config){
		.callbacks = &event_callbacks,
		.time_sync_periodicity_seconds = 7200,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	sid_error_t e = application_pal_init();
	if (e) {
		LOG_ERR("sid pal init error %d", e);
	}

	return e;
}

/* Sidewalk Events */
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
}

/* Application State Machine */
#define EVENT_BTN_PRESS BIT(0)

static void state_unregistered_run(void *o);
static void state_registered_run(void *o);
static void state_time_synced_run(void *o);
static void state_ready_run(void *o);

enum app_state { STATE_UNREGISTERED, STATE_REGISTERED, STATE_TIME_SYNCED, STATE_READY };

static const struct smf_state app_states[] = {
	[STATE_UNREGISTERED] = SMF_CREATE_STATE(NULL, state_unregistered_run, NULL),
	[STATE_REGISTERED] = SMF_CREATE_STATE(NULL, state_registered_run, NULL),
	[STATE_TIME_SYNCED] = SMF_CREATE_STATE(NULL, state_time_synced_run, NULL),
	[STATE_READY] = SMF_CREATE_STATE(NULL, state_ready_run, NULL),
};

struct s_object {
	struct smf_ctx ctx;
	struct k_event smf_event;
	int32_t events;
} s_obj;

static void state_unregistered_run(void *o)
{
	struct s_object *s = (struct s_object *)o;

	LOG_INF("state: unregistered");

	smf_set_state(SMF_CTX(s), &app_states[STATE_REGISTERED]);
}

static void state_registered_run(void *o)
{
	struct s_object *s = (struct s_object *)o;

	LOG_INF("state: registered");

	smf_set_state(SMF_CTX(s), &app_states[STATE_TIME_SYNCED]);
}

static void state_time_synced_run(void *o)
{
	struct s_object *s = (struct s_object *)o;

	LOG_INF("state: time synced");

	if (s->events & EVENT_BTN_PRESS) {
		smf_set_state(SMF_CTX(s), &app_states[STATE_READY]);
		k_event_clear(&s->smf_event, EVENT_BTN_PRESS);
	}
}

static void state_ready_run(void *o)
{
	struct s_object *s = (struct s_object *)o;

	LOG_INF("state: ready");

	if (s->events & EVENT_BTN_PRESS) {
		LOG_INF("Success!");
		k_event_clear(&s->smf_event, EVENT_BTN_PRESS);
	}
}

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	if (buttons & DK_BTN1_MSK) {
		LOG_INF("button 1");
		k_event_post(&s_obj.smf_event, EVENT_BTN_PRESS);
	}
}

int main(void)
{
	LOG_INF("Hello World! %s", CONFIG_BOARD);

	if (dk_buttons_init(button_changed)) {
		LOG_ERR("Cannot init buttons");
	}

	if (sidewalk_thread_init(&sid_ctx)) {
		LOG_ERR("Cannot init sid thread");
	}

	k_event_init(&s_obj.smf_event);
	smf_set_initial(SMF_CTX(&s_obj), &app_states[STATE_UNREGISTERED]);
	while (1) {
		s_obj.events = k_event_wait(&s_obj.smf_event, EVENT_BTN_PRESS, false, K_MSEC(1000));
		if (smf_run_state(SMF_CTX(&s_obj))) {
			break;
		}
	}

	return 0;
}
