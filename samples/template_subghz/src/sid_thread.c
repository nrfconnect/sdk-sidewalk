/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_error.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <sid_thread.h>

#include <sid_api.h>

#include <app_ble_config.h>
#include <state_notifier.h>
#include <sid_hal_reset_ifc.h>
#include <app_subGHz_config.h>

#ifdef CONFIG_SIDEWALK_CLI
#include <sid_shell.h>
#endif

static const uint8_t *link_mode_name[] = {
	"none", [SID_LINK_MODE_CLOUD] = "cloud", [SID_LINK_MODE_MOBILE] = "mobile"
};

static const uint8_t *link_mode_idx_name[] = {
	"ble", "fsk", "lora"
};
struct notifier_ctx global_state_notifier;

LOG_MODULE_REGISTER(sid_thread, CONFIG_SID_THREAD_LOG_LEVEL);

// ////////////////////////////////////////////////////////////////////////////
K_THREAD_STACK_DEFINE(sidewalk_work_q_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);
static struct k_work_q sidewalk_work_q;

typedef struct app_context {
	struct sid_handle *sidewalk_handle;
	struct sid_config sidewalk_config;
	struct k_work sidewalk_event_work;
} app_context_t;

static app_context_t g_app_context;

// ////////////////////////////////////////////////////////////////////////////

static void sidewalk_event_worker(struct k_work *work)
{
	struct app_context *app_ctx = CONTAINER_OF(work, struct app_context, sidewalk_event_work);

	sid_error_t e = sid_process(app_ctx->sidewalk_handle);

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sid process failed with error %d", e);
	}
}

// ////////////////////////////////////////////////////////////////////////////
static void on_sidewalk_event(bool in_isr, void *context)
{
	ARG_UNUSED(in_isr);
	ARG_UNUSED(context);
	if (g_app_context.sidewalk_event_work.handler == NULL) {
		k_work_init(&g_app_context.sidewalk_event_work, sidewalk_event_worker);
	}
	k_work_submit_to_queue(&sidewalk_work_q, &g_app_context.sidewalk_event_work);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_DBG("Sidewalk -> App");
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");

	#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_message_received(msg_desc->id);
	#endif
	application_state_receiving(&global_state_notifier, true);
	application_state_receiving(&global_state_notifier, false);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("Sidewalk -> App");

	application_state_sending(&global_state_notifier, false);
	#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_message_send();
	#endif
	LOG_INF("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("Sidewalk -> App: error %d", error);

	application_state_sending(&global_state_notifier, false);
	#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_message_not_send();
	#endif
	LOG_ERR("failed to send message(type: %d, id: %u), err:%d", (int)msg_desc->type, msg_desc->id, (int)error);
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_DBG("Sidewalk -> App");

	LOG_DBG("factory reset notification received from sid api");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Reboot type not supported");
	}
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	app_context_t *app_ctx = (app_context_t *)context;

#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_sid_status(status);
#endif
	switch (status->state) {
	case SID_STATE_READY:
		application_state_connected(&global_state_notifier, true);
		break;
	case SID_STATE_NOT_READY:
		application_state_connected(&global_state_notifier, false);
		break;
	case SID_STATE_ERROR:
		LOG_ERR("Sidewalk error: %d", (int)sid_get_error(app_ctx->sidewalk_handle));
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		break;
	}

	application_state_registered(&global_state_notifier, status->detail.registration_status == SID_STATUS_REGISTERED);
	application_state_time_sync(&global_state_notifier, status->detail.time_sync_status == SID_STATUS_TIME_SYNCED);
	application_state_link(&global_state_notifier, status->detail.link_status_mask);

	LOG_INF("Device %sregistered, Time Sync %s, Link status %s",
		(SID_STATUS_REGISTERED == status->detail.registration_status) ? "Is " : "Un",
		(SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) ? "Success" : "Fail",
		status->detail.link_status_mask ? "Up" : "Down");
	for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
		enum sid_link_mode mode = (enum sid_link_mode)status->detail.supported_link_modes[i];

		if (mode) {
			LOG_INF("Link mode %s, on %s", link_mode_name[mode], link_mode_idx_name[i]);
		}
	}
}

static struct sid_event_callbacks event_callbacks = {
	.context = &g_app_context,
	.on_event = on_sidewalk_event,                          /* Called from ISR context */
	.on_msg_received = on_sidewalk_msg_received,            /* Called from sid_process() */
	.on_msg_sent = on_sidewalk_msg_sent,                    /* Called from sid_process() */
	.on_send_error = on_sidewalk_send_error,                /* Called from sid_process() */
	.on_status_changed = on_sidewalk_status_changed,        /* Called from sid_process() */
	.on_factory_reset = on_sidewalk_factory_reset,          /* Called from sid_process() */
};



///////////////////////////////////////////////////////////////////////////////

struct k_work_q * sid_thread_init(void)
{
	k_work_queue_init(&sidewalk_work_q);
	static struct k_work_queue_config cfg = { .name = "sidewalk_thread", .no_yield = false };

	k_work_queue_start(&sidewalk_work_q, sidewalk_work_q_stack,
			   K_THREAD_STACK_SIZEOF(sidewalk_work_q_stack), CONFIG_SIDEWALK_THREAD_PRIORITY,
			   &cfg);
	return &sidewalk_work_q;
}

struct sid_config *get_sidewalk_config()
{
	static bool config_initialized = false;

	if (!config_initialized) {
		config_initialized = true;
		g_app_context.sidewalk_config = (struct sid_config){
			.link_mask = 0,
			.callbacks = &event_callbacks,
			.link_config = app_get_ble_config(),
			.time_sync_periodicity_seconds = 7200,
			.sub_ghz_link_config = app_get_sub_ghz_config(),
		};

	}
	return &g_app_context.sidewalk_config;
}

struct sid_handle **get_sidewalk_handle()
{
	return &g_app_context.sidewalk_handle;
}
