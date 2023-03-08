/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <sid_callbacks.h>
#include <sid_thread.h>

#include <sid_api.h>
#include <sid_hal_reset_ifc.h>

#ifdef CONFIG_SIDEWALK_CLI
#include <sid_shell.h>
#endif

#ifdef CONFIG_STATE_NOTIFIER
#include <state_notifier.h>
#endif

static const uint8_t *link_mode_name[] = {
	"none", [SID_LINK_MODE_CLOUD] = "cloud", [SID_LINK_MODE_MOBILE] = "mobile"
};

static const uint8_t *link_mode_idx_name[] = {
	"ble", "fsk", "lora"
};

LOG_MODULE_REGISTER(sid_callbacks, CONFIG_SID_THREAD_LOG_LEVEL);
// ////////////////////////////////////////////////////////////////////////////

static void sidewalk_event_worker(struct k_work *work)
{
	sid_thread_ctx_t *ctx = CONTAINER_OF(work, sid_thread_ctx_t, sidewalk_event_work);

	sid_error_t e = sid_process(ctx->sidewalk_handle);

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sid process failed with error %d", e);
	}
}

// ////////////////////////////////////////////////////////////////////////////
static void on_sidewalk_event(bool in_isr, void *context)
{
	ARG_UNUSED(in_isr);
	sid_thread_ctx_t * ctx = (sid_thread_ctx_t*) context;
	if (ctx->sidewalk_event_work.handler == NULL) {
		k_work_init(&ctx->sidewalk_event_work, sidewalk_event_worker);
	}
	k_work_submit_to_queue(&ctx->sidewalk_work_q, &ctx->sidewalk_event_work);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_DBG("Sidewalk -> App");
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");

	#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_message_received(msg_desc->id);
	#endif
	#ifdef CONFIG_STATE_NOTIFIER
	application_state_receiving(&global_state_notifier, true);
	application_state_receiving(&global_state_notifier, false);
	#endif
}


static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("Sidewalk -> App");

	#ifdef CONFIG_STATE_NOTIFIER
	application_state_sending(&global_state_notifier, false);
	#endif
	#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_message_send();
	#endif
	LOG_INF("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("Sidewalk -> App: error %d", error);

	#ifdef CONFIG_STATE_NOTIFIER
	application_state_sending(&global_state_notifier, false);
	#endif
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
	sid_thread_ctx_t *ctx = (sid_thread_ctx_t *)context;

#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_sid_status(status);
#endif
	switch (status->state) {
	case SID_STATE_READY:
		#ifdef CONFIG_STATE_NOTIFIER
		application_state_connected(&global_state_notifier, true);
		#endif
		break;
	case SID_STATE_NOT_READY:
		#ifdef CONFIG_STATE_NOTIFIER
		application_state_connected(&global_state_notifier, false);
		#endif
		break;
	case SID_STATE_ERROR:
		LOG_ERR("Sidewalk error: %d", (int)sid_get_error(ctx->sidewalk_handle));
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		break;
	}

	#ifdef CONFIG_STATE_NOTIFIER
	application_state_registered(&global_state_notifier, status->detail.registration_status == SID_STATUS_REGISTERED);
	application_state_time_sync(&global_state_notifier, status->detail.time_sync_status == SID_STATUS_TIME_SYNCED);
	application_state_link(&global_state_notifier, status->detail.link_status_mask);
	#endif

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

// ////////////////////////////////////////////////////////////////////////////

int sid_callbacks_set(void *ctx, struct sid_event_callbacks *cb){
	if (!cb || !ctx) {
		return -1;
	}

	cb->context = ctx;
	cb->on_event = on_sidewalk_event;                          /* Called from ISR context */
	cb->on_msg_received = on_sidewalk_msg_received;            /* Called from sid_process() */
	cb->on_msg_sent = on_sidewalk_msg_sent;                    /* Called from sid_process() */
	cb->on_send_error = on_sidewalk_send_error;                /* Called from sid_process() */
	cb->on_status_changed = on_sidewalk_status_changed;        /* Called from sid_process() */
	cb->on_factory_reset = on_sidewalk_factory_reset;          /* Called from sid_process() */

	return 0;
}
