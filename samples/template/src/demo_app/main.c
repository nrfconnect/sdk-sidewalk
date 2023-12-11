/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "zephyr/sys/util.h"
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/smf.h>
#include <zephyr/sys/reboot.h>

#include <json_printer.h>
#include <sidTypes2Json.h>
#include <sidewalk_version.h>
#include <settings_utils.h>

#include <app_ble_config.h>
#include <app_subGHz_config.h>
#include <pal_init.h>
#include <sid_api.h>
#include <sid_api_delegated.h>
#include <sid_error.h>

#if CONFIG_SIDEWALK_DUT_CLI
#include <sid_dut_shell.h>
#endif

LOG_MODULE_REGISTER(sid_application, CONFIG_SIDEWALK_LOG_LEVEL);

K_THREAD_STACK_DEFINE(sidewalk_work_q_stack, KB(10)); // TODO make configurable
static char json_output[512] = { 0 }; // TODO make configurable for application

static void sidewalk_event_worker(struct k_work *work);
static void on_sidewalk_event(bool in_isr, void *context);
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context);
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context);
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context);
static void on_sidewalk_factory_reset(void *context);
static void on_sidewalk_status_changed(const struct sid_status *status, void *context);

// ////////////////////////////////////////////////////////////////////////////

enum application_states {
	EVENT_REGISTERED = 0,
	EVENT_TIME_SYNC,
	EVENT_TIME_SYNC_DROP,
	EVENT_BLE_LINK_UP,
	EVENT_BLE_LINK_DOWN,
	EVENT_FSK_LINK_UP,
	EVENT_FSK_LINK_DOWN,
	EVENT_LORA_LINK_UP,
	EVENT_LORA_LINK_DOWN,
	EVENT_MSG_RCV,
	EVENT_MSG_SEND,
	EVENT_MSG_SEND_ERROR,
	EVENT_REBOOT,

#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	EVENT_DFU,
#endif

	CUSTOM_EVENT_START_VALUE // start your custom events from this value
	// make sure that no event is bigger than 32.
};

struct s_object {
	/* This must be first */
	struct smf_ctx ctx;

	/* Events */
	struct k_event event;
	uint32_t events;
};

struct application_context {
	struct sid_handle *sid_handle;
	struct sid_config config;
	struct k_work_q sidewalk_work_q;
	struct k_work sidewalk_event_work;
	struct sid_status last_status;
	sid_error_t status_error;
	sid_error_t send_error;

	struct k_msgq received_messages;
	struct sid_msg message_storage[10]; // .data is stored on heap, free it after use.

	struct s_object application_state_machine;
};

static struct application_context app_ctx = { 0 };

// forward declaration of applicaiton states
static const struct smf_state application_states[];

static struct sid_event_callbacks event_callbacks = {
	.context = &app_ctx,
	.on_event = on_sidewalk_event, /* Called from ISR context */
	.on_msg_received = on_sidewalk_msg_received, /* Called from sid_process() */
	.on_msg_sent = on_sidewalk_msg_sent, /* Called from sid_process() */
	.on_send_error = on_sidewalk_send_error, /* Called from sid_process() */
	.on_status_changed = on_sidewalk_status_changed, /* Called from sid_process() */
	.on_factory_reset = on_sidewalk_factory_reset, /* Called from sid_process */
};

// ////////////////////////////////////////////////////////////////////////////

static void sidewalk_event_worker(struct k_work *work)
{
	struct application_context *app_ctx =
		CONTAINER_OF(work, struct application_context, sidewalk_event_work);

	sid_error_t e = sid_process(app_ctx->sid_handle);

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sid process failed with error %s", SID_ERROR_T_STR(e));
	}
}

static void on_sidewalk_event(bool in_isr, void *context)
{
	struct application_context *ctx = (struct application_context *)context;

	if (ctx->sidewalk_event_work.handler == NULL) {
		k_work_init(&ctx->sidewalk_event_work, sidewalk_event_worker);
	}
	int e = k_work_submit_to_queue(&ctx->sidewalk_work_q, &ctx->sidewalk_event_work);
	switch (e) {
	case 0:
		break;
	case 1:
		LOG_DBG("Work not submitted, has been queued");
		break;
	case 2:
		LOG_DBG("Work was running and has been submited");
		break;
	case EINVAL:
		LOG_ERR("Sidewalk work queue is NULL! - initialize work queue before use");
		break;
	case ENODEV:
		LOG_ERR("Sidewalk work queue has not been started!");
		break;
	default:
		LOG_ERR("Unknown error occured during submition work to queue err %d", e);
	}
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	struct application_context *ctx = (struct application_context *)context;
	JSON_WRITE_LOG(LOG_LEVEL_DBG, json_output, sizeof(json_output),
		       JSON_NEW_LINE(JSON_OBJ(
			       JSON_NAME("on_msg_received",
					 JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 1)))));

	LOG_HEXDUMP_INF(msg->data, msg->size, "");

	struct sid_msg save_message = { .size = msg->size };
	save_message.data = k_malloc(save_message.size);
	memcpy(save_message.data, msg->data, msg->size);
	int e = k_msgq_put(&ctx->received_messages, &save_message, K_NO_WAIT);
	switch (e) {
	case 0:
		break; // message has succesfully been saved
	case -ENOMSG:
		break; // Returned without waiting or queue purged.
	case -EAGAIN:
		LOG_INF("message queue is full, can not append new message");
		break;
	default:
		LOG_ERR("Unknown error for k_msgq_put %d", e);
	}
	LOG_DBG("event post");
	k_event_post(&ctx->application_state_machine.event, BIT(EVENT_MSG_RCV));
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	struct application_context *ctx = (struct application_context *)context;
	JSON_WRITE_LOG(LOG_LEVEL_DBG, json_output, sizeof(json_output),
		       JSON_OBJ(JSON_NAME("on_msg_sent", JSON_OBJ(JSON_VAL_sid_msg_desc(
								 "sid_msg_desc", msg_desc, 0)))));
	LOG_DBG("event post");
	k_event_post(&ctx->application_state_machine.event, BIT(EVENT_MSG_SEND));
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	struct application_context *ctx = (struct application_context *)context;
	JSON_WRITE_LOG(
		LOG_LEVEL_DBG, json_output, sizeof(json_output),
		JSON_OBJ(JSON_NAME("on_send_error",
				   JSON_OBJ(JSON_LIST_2(JSON_VAL_sid_error_t("error", error),
							JSON_VAL_sid_msg_desc("sid_msg_desc",
									      msg_desc, 0))))));
	ctx->send_error = error;
	LOG_DBG("event post");
	k_event_post(&ctx->application_state_machine.event, BIT(EVENT_MSG_SEND_ERROR));
}

static void on_sidewalk_factory_reset(void *context)
{
	struct application_context *ctx = (struct application_context *)context;
	LOG_DBG("Factory reset callback called");
	LOG_DBG("event post");
	k_event_post(&ctx->application_state_machine.event, BIT(EVENT_REBOOT));
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	struct application_context *ctx = (struct application_context *)context;
	JSON_WRITE_LOG(LOG_LEVEL_DBG, json_output, sizeof(json_output),
		       JSON_VAL_sid_status(status));

	// save previous for compare and event generation
	const struct sid_status prev_state = ctx->last_status;

	// update state in context
	ctx->last_status = *status;
	if (status->state == SID_STATE_ERROR) {
		ctx->status_error = sid_get_error(ctx->sid_handle);
	}

	// generate events from status change
	if (prev_state.detail.registration_status == SID_STATUS_NOT_REGISTERED &&
	    status->detail.registration_status == SID_STATUS_REGISTERED) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_REGISTERED));
	}
	if (prev_state.detail.time_sync_status == SID_STATUS_NO_TIME &&
	    status->detail.time_sync_status == SID_STATUS_TIME_SYNCED) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_TIME_SYNC));
	}
	if (prev_state.detail.time_sync_status == SID_STATUS_TIME_SYNCED &&
	    status->detail.time_sync_status == SID_STATUS_NO_TIME) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_TIME_SYNC_DROP));
	}

	if ((prev_state.detail.link_status_mask & SID_LINK_TYPE_1) == 0 &&
	    (status->detail.link_status_mask & SID_LINK_TYPE_1) != 0) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_BLE_LINK_UP));
	}
	if ((prev_state.detail.link_status_mask & SID_LINK_TYPE_1) != 0 &&
	    (status->detail.link_status_mask & SID_LINK_TYPE_1) == 0) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_BLE_LINK_DOWN));
	}

	if ((prev_state.detail.link_status_mask & SID_LINK_TYPE_2) == 0 &&
	    (status->detail.link_status_mask & SID_LINK_TYPE_2) != 0) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_FSK_LINK_UP));
	}
	if ((prev_state.detail.link_status_mask & SID_LINK_TYPE_2) != 0 &&
	    (status->detail.link_status_mask & SID_LINK_TYPE_2) == 0) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_FSK_LINK_DOWN));
	}

	if ((prev_state.detail.link_status_mask & SID_LINK_TYPE_3) == 0 &&
	    (status->detail.link_status_mask & SID_LINK_TYPE_3) != 0) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_LORA_LINK_UP));
	}
	if ((prev_state.detail.link_status_mask & SID_LINK_TYPE_3) != 0 &&
	    (status->detail.link_status_mask & SID_LINK_TYPE_3) == 0) {
		LOG_DBG("event post");
		k_event_post(&ctx->application_state_machine.event, BIT(EVENT_LORA_LINK_DOWN));
	}
}

// ////////////////////////////////////////////////////////////////////////////

static void prepare_application_context(void)
{
	k_work_queue_init(&app_ctx.sidewalk_work_q);
	static struct k_work_queue_config cfg = { .name = "sidewalk_thread", .no_yield = false };

	k_work_queue_start(&app_ctx.sidewalk_work_q, sidewalk_work_q_stack,
			   K_THREAD_STACK_SIZEOF(sidewalk_work_q_stack),
			   CONFIG_SIDEWALK_THREAD_PRIORITY, &cfg);
	sid_api_delegated(&app_ctx.sidewalk_work_q);

	k_msgq_init(&app_ctx.received_messages, (uint8_t *)app_ctx.message_storage,
		    sizeof(app_ctx.message_storage[0]), ARRAY_SIZE((app_ctx.message_storage)));

	app_ctx.config = (struct sid_config){
		.callbacks = &event_callbacks,
		.link_config = app_get_ble_config(),
		.time_sync_periodicity_seconds = 7200,

		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	app_ctx.last_status =
		(struct sid_status){ .state = SID_STATE_NOT_READY,
				     .detail = (struct sid_status_detail){
					     .time_sync_status = SID_STATUS_NO_TIME,
					     .registration_status = SID_STATUS_NOT_REGISTERED,
					     .link_status_mask = 0,
					     .supported_link_modes = { 0 } } };
	k_event_init(&app_ctx.application_state_machine.event);
	smf_set_initial(SMF_CTX(&app_ctx.application_state_machine), &application_states[0]);

#if CONFIG_SIDEWALK_DUT_CLI
	initialize_sidewalk_shell(&app_ctx.config, &app_ctx.sid_handle);
#endif
}

static sid_error_t application_setup(void)
{
	prepare_application_context();
	sid_error_t e = application_pal_init();
	if (e != SID_ERROR_NONE) {
		LOG_ERR("Failed to prepare application, error %s", SID_ERROR_T_STR(e));
	}

	return e;
}

static void application_main_loop(void)
{
	LOG_DBG("START LOOP");
	int ret = smf_run_state(SMF_CTX(&app_ctx.application_state_machine));
	LOG_INF("smf state return %d", ret);
	while (ret == 0) {
		LOG_INF("wait for event");
		app_ctx.application_state_machine.events |= k_event_wait(
			&app_ctx.application_state_machine.event, UINT32_MAX, true, K_FOREVER);
		LOG_INF("event received");
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
		if (app_ctx.application_state_machine.events & BIT(EVENT_DFU)) {
			bool DFU_mode = true;
			(void)settings_save_one(CONFIG_DFU_FLAG_SETTINGS_KEY,
						(const void *)&DFU_mode, sizeof(DFU_mode));

			WRITE_BIT(app_ctx.application_state_machine.events, EVENT_DFU, 0);
			LOG_DBG("event post");
			k_event_post(&app_ctx.application_state_machine.event, BIT(EVENT_REBOOT));
		}
#endif
		if (app_ctx.application_state_machine.events & BIT(EVENT_REBOOT)) {
			WRITE_BIT(app_ctx.application_state_machine.events, EVENT_REBOOT, 0);
			sys_reboot(SYS_REBOOT_COLD);
		}
		LOG_INF("Run State machine");
		// some event has been generated, run the state machine to handle the event
		// state handlers should cancel events that has been handled
		ret = smf_run_state(SMF_CTX(&app_ctx.application_state_machine));
	}
	LOG_ERR("Exited the state machine");
}

// ////////////////////////////////////////////////////////////////////////////

int main()
{
	PRINT_SIDEWALK_VERSION();
	switch (application_to_start()) {
	case SIDEWALK_APPLICATION: {
		if (SID_ERROR_NONE != application_setup()) {
			return -1;
		}
		application_main_loop();
		break;
	};
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	case DFU_APPLICATION: {
		const int ret = nordic_dfu_ble_start();
		LOG_INF("DFU service started, return value %d", ret);
		break;
	}
#endif
	default:
		LOG_ERR("Unknown application to start.");
	}

	return 0;
}

// ////////////////////////////////////////////////////////////////////////////
// State machine code can be extracted to different file.
// ////////////////////////////////////////////////////////////////////////////

// custom events for the application
// for example the CUSTOM_EVENT_SEND_HELLO can be triggered by button press
enum CustomEvents {
	CUSTOM_EVENT_SEND_HELLO = CUSTOM_EVENT_START_VALUE,
};

// Application states, create as many or as little as you like to design behaviour of your application.
// make sure to document the application with states diagram
enum ApplicationStates {
	Startup = 0,
	Connecting,
	Connected,
};

static void s1_run(void *o)
{
	LOG_DBG("state 1");
	struct s_object *sm = (struct s_object *)o;
	struct application_context *ctx =
		CONTAINER_OF(sm, struct application_context, application_state_machine);
	ctx->config.link_mask =
		SID_LINK_TYPE_1; // TODO currently it starts BLE, make it configurable

	sid_error_t e = sid_init_delegated(&ctx->config, &ctx->sid_handle);
	switch (e) {
	case SID_ERROR_NONE:
		break;
	default: {
		LOG_ERR("sid init failed with error %s", SID_ERROR_T_STR(e));
		return;
	}
	}
	e = sid_start_delegated(ctx->sid_handle, ctx->config.link_mask);
	switch (e) {
	case SID_ERROR_NONE:
		break;
	default: {
		LOG_ERR("sid start failed with error %s", SID_ERROR_T_STR(e));
		return;
	}
	}
	LOG_DBG("event post");
	k_event_post(
		&sm->event,
		BIT(CUSTOM_EVENT_SEND_HELLO)); // post event to send message even it is imposible yet. The event will be handled when application will enter into proper state and than it will be cleared.
	smf_set_state(
		SMF_CTX(sm),
		&application_states
			[Connecting]); // move to connecting state because we called sid_start and we expect the applicaiton to connect with cloud
}

static void s2_run(void *o)
{
	LOG_DBG("state 2");
	struct s_object *sm = (struct s_object *)o;
	struct application_context *ctx =
		CONTAINER_OF(sm, struct application_context, application_state_machine);

	if (sm->events & BIT(CUSTOM_EVENT_SEND_HELLO) &&
	    ctx->last_status.detail.time_sync_status == SID_STATUS_TIME_SYNCED &&
	    ctx->config.link_mask & SID_LINK_TYPE_1_IDX) {
		// TODO make separate state for connection request
		sid_error_t e = sid_ble_bcn_connection_request(ctx->sid_handle, true);
		if (e != SID_ERROR_NONE) {
			LOG_ERR("can not conn_req");
		}
	}
	if (sm->events & BIT(EVENT_MSG_RCV)) {
		// if we received message we have to be connected, altho maybe this should be separate state ...
		smf_set_state(SMF_CTX(sm), &application_states[Connected]);
	}
	if (sm->events & BIT(EVENT_BLE_LINK_UP) || sm->events & BIT(EVENT_FSK_LINK_UP) ||
	    sm->events & BIT(EVENT_LORA_LINK_UP)) {
		WRITE_BIT(app_ctx.application_state_machine.events, EVENT_BLE_LINK_UP, 0);
		WRITE_BIT(app_ctx.application_state_machine.events, EVENT_FSK_LINK_UP, 0);
		WRITE_BIT(app_ctx.application_state_machine.events, EVENT_LORA_LINK_UP, 0);

		smf_set_state(SMF_CTX(sm), &application_states[Connected]);
	}
}

static void s3_run(void *o)
{
	LOG_DBG("state 3");
	struct s_object *sm = (struct s_object *)o;
	struct application_context *ctx =
		CONTAINER_OF(sm, struct application_context, application_state_machine);

	if ((sm->events & BIT(CUSTOM_EVENT_SEND_HELLO)) &&
	    (sm->events & BIT(EVENT_BLE_LINK_DOWN))) {
		sid_error_t e = sid_ble_bcn_connection_request(ctx->sid_handle, true);
		if (e != SID_ERROR_NONE) {
			LOG_ERR("can not conn_req");
		}

		WRITE_BIT(app_ctx.application_state_machine.events, EVENT_BLE_LINK_DOWN, 0);
		smf_set_state(SMF_CTX(sm), &application_states[Connecting]);
		return;
	}

	if (sm->events & BIT(EVENT_BLE_LINK_DOWN)) {
		WRITE_BIT(app_ctx.application_state_machine.events, EVENT_BLE_LINK_DOWN, 0);
		smf_set_state(SMF_CTX(sm), &application_states[Connecting]);
		return;
	}

	if (sm->events & BIT(CUSTOM_EVENT_SEND_HELLO)) {
		static struct sid_msg msg;
		static struct sid_msg_desc desc;
#define HELLO_MESSAGE "Hello from the template"
		msg = (struct sid_msg){ .data = HELLO_MESSAGE, .size = strlen(HELLO_MESSAGE) };
		desc = (struct sid_msg_desc){
			.type = SID_MSG_TYPE_NOTIFY,
			.link_type = SID_LINK_TYPE_ANY,
			.link_mode = SID_LINK_MODE_CLOUD,
		};
		LOG_ERR("put message@@@@@@@@@@@");
		sid_error_t e = sid_put_msg_delegated(ctx->sid_handle, &msg, &desc);
		if (e == SID_ERROR_NONE) {
			WRITE_BIT(app_ctx.application_state_machine.events, CUSTOM_EVENT_SEND_HELLO,
				  0);
		} else {
			LOG_ERR("put message return err %s", SID_ERROR_T_STR(e));
		}
	}

	if (sm->events & BIT(EVENT_MSG_RCV)) {
		struct sid_msg msg;
		while (k_msgq_get(&ctx->received_messages, &msg, K_NO_WAIT) == 0) {
			static struct sid_msg_desc desc;
			desc = (struct sid_msg_desc){
				.type = SID_MSG_TYPE_NOTIFY,
				.link_type = SID_LINK_TYPE_ANY,
				.link_mode = SID_LINK_MODE_CLOUD,
			};
			sid_put_msg_delegated(ctx->sid_handle, &msg, &desc);
			k_free(msg.data); // important to free memory
		}

		WRITE_BIT(app_ctx.application_state_machine.events, EVENT_MSG_RCV, 0);
	}
}

// the actual states handlers, before and after functions can be added to track the state changes in time.
static const struct smf_state application_states[] = {
	[Startup] = SMF_CREATE_STATE(NULL, s1_run, NULL),
	[Connecting] = SMF_CREATE_STATE(NULL, s2_run, NULL),
	[Connected] = SMF_CREATE_STATE(NULL, s3_run, NULL),
};

int shell_get_state(const struct shell *shell, int32_t argc, const char **argv)
{
	int state_id =
		ARRAY_INDEX(application_states, app_ctx.application_state_machine.ctx.current);

	shell_info(shell, "current state %d", state_id);
	shell_info(shell, "current events 0x%x", app_ctx.application_state_machine.events);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_services,
			       SHELL_CMD_ARG(get_state, NULL,
					     "<0;1> - enable/disable ping messages",
					     shell_get_state, 1, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(app, &sub_services, "application cli", NULL);
