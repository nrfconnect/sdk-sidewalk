/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdint.h>

#include <json_printer.h>
#include <sidTypes2Json.h>
#include <sidewalk_version.h>

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

K_THREAD_STACK_DEFINE(sidewalk_work_q_stack, 1024); // TODO make configurable
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
	BUTTON_ACTION = 0,
	SIDEWALK_STATUS_CHANGED,
	SIDEWALK_MSG_RECEIVED,
	SIDEWALK_MSG_SEND,
	SIDEWALK_MSG_SEND_ERROR,
	SIDEWALK_FACTORY_RESET,

	EVENT_CNT
};

#if EVENT_CNT > 32
#err Events can hold up to 32 states
#endif

struct application_context {
	struct sid_handle **sid_handle;
	struct sid_config config;
	struct k_work_q sidewalk_work_q;
	struct k_work sidewalk_event_work;
	struct sid_status last_status;
	sid_error_t status_error;
	sid_error_t send_error;

	struct k_msgq received_messages;
	struct sid_msg message_storage[10]; // .data is stored on heap, free it after use.

	struct k_event event;
};

static struct application_context app_ctx = {};

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

	sid_error_t e = sid_process(*app_ctx->sid_handle);

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
	k_event_set(&ctx->event, BIT(SIDEWALK_MSG_RECEIVED));
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	struct application_context *ctx = (struct application_context *)context;
	JSON_WRITE_LOG(LOG_LEVEL_DBG, json_output, sizeof(json_output),
		       JSON_OBJ(JSON_NAME("on_msg_sent", JSON_OBJ(JSON_VAL_sid_msg_desc(
								 "sid_msg_desc", msg_desc, 0)))));
	k_event_set(&ctx->event, BIT(SIDEWALK_MSG_SEND));
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
	k_event_set(&ctx->event, BIT(SIDEWALK_MSG_SEND_ERROR));
}

static void on_sidewalk_factory_reset(void *context)
{
	struct application_context *ctx = (struct application_context *)context;
	LOG_DBG("Factory reset callback called");
	k_event_set(&ctx->event, BIT(SIDEWALK_FACTORY_RESET));
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	struct application_context *ctx = (struct application_context *)context;
	JSON_WRITE_LOG(LOG_LEVEL_DBG, json_output, sizeof(json_output),
		       JSON_VAL_sid_status(status));

	ctx->last_status = *status;
	if (status->state == SID_STATE_ERROR) {
		ctx->status_error = sid_get_error(*ctx->sid_handle);
	}
	k_event_set(&ctx->event, BIT(SIDEWALK_STATUS_CHANGED));
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

	app_ctx.config = (struct sid_config){ .callbacks = &event_callbacks,
					      .link_config = app_get_ble_config(),
					      .time_sync_periodicity_seconds = 7200,
					      .sub_ghz_link_config = app_get_sub_ghz_config() };

#if CONFIG_SIDEWALK_DUT_CLI
	initialize_sidewalk_shell(&app_ctx.config, app_ctx.sid_handle);
#endif
}

static sid_error_t application_setup(void)
{
	PRINT_SIDEWALK_VERSION();
	prepare_application_context();
	sid_error_t e = application_pal_init();
	if (e != SID_ERROR_NONE) {
		LOG_ERR("Failed to prepare application, error %s", SID_ERROR_T_STR(e));
	}
	return e;
}

static void application_main_loop(void)
{
	while (true) {
		/* Block until an event is detected */
		uint32_t events = k_event_wait(&app_ctx.event, UINT32_MAX, false, K_FOREVER);

		for (enum application_states event = 0; event < EVENT_CNT; event++) {
			if ((events && BIT(event)) == 0) {
				continue;
			}
			switch (event) {
			case BUTTON_ACTION: {
				// implement action for event
				k_event_clear(&app_ctx.event, BIT(BUTTON_ACTION));
				break;
			}
			case SIDEWALK_STATUS_CHANGED: {
				// implement action for event
				k_event_clear(&app_ctx.event, BIT(SIDEWALK_STATUS_CHANGED));
				break;
			}
			case SIDEWALK_MSG_RECEIVED: {
				// implement action for event
				k_event_clear(&app_ctx.event, BIT(SIDEWALK_MSG_RECEIVED));
				break;
			}
			case SIDEWALK_MSG_SEND: {
				// implement action for event
				k_event_clear(&app_ctx.event, BIT(SIDEWALK_MSG_SEND));
				break;
			}
			case SIDEWALK_MSG_SEND_ERROR: {
				// implement action for event
				k_event_clear(&app_ctx.event, BIT(SIDEWALK_MSG_SEND_ERROR));
				break;
			}
			case SIDEWALK_FACTORY_RESET: {
				// implement action for event
				k_event_clear(&app_ctx.event, BIT(SIDEWALK_FACTORY_RESET));
				break;
			}
			case EVENT_CNT: {
				LOG_ERR("EVENT_CNT is not sopoused to be used!");
				break;
			}
			default:
				LOG_ERR("unkonown application event!");
			}
		}
	}
}

// ////////////////////////////////////////////////////////////////////////////

int main()
{
	if (SID_ERROR_NONE != application_setup())
		return -1;
	application_main_loop();
	return 0;
}
