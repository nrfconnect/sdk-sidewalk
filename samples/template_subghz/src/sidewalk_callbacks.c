/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>
#include <sid_error.h>
#include <sid_hal_reset_ifc.h>

#include <state_notifier.h>
#if defined(CONFIG_SIDEWALK_CLI)
#include <sid_shell.h>
#endif

#include <application_thread.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(callbacks, CONFIG_SIDEWALK_LOG_LEVEL);

static const uint8_t *status_name[] = {
	"ready", "not ready", "Error", "secure channel ready"
};
static const uint8_t *link_mode_name[] = {
	"none", [SID_LINK_MODE_CLOUD] = "cloud", [SID_LINK_MODE_MOBILE] = "mobile"
};

static const uint8_t *link_mode_idx_name[] = {
	"ble", "fsk", "lora"
};

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_DBG("on event, from %s, context %p", in_isr ? "ISR" : "App", context);
	app_event_send(SIDEWALK_EVENT);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_message_received(msg_desc->id);
	#endif
	application_state_receiving(&global_state_notifier, true);
	application_state_receiving(&global_state_notifier, false);
	LOG_DBG("received message(type: %d, link_mode: %d, id: %u size %u)", (int)msg_desc->type,
		(int)msg_desc->link_mode, msg_desc->id, msg->size);
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	application_state_sending(&global_state_notifier, false);
	#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_message_send();
	#endif
	LOG_INF("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	application_state_sending(&global_state_notifier, false);
	#ifdef CONFIG_SIDEWALK_CLI
	CLI_register_message_not_send();
	#endif
	LOG_ERR("failed to send message(type: %d, id: %u), err:%d", (int)msg_desc->type, msg_desc->id, (int)error);
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	LOG_INF("status changed: %s", status_name[status->state]);

	app_ctx_t *app_ctx = (app_ctx_t *)context;

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
		LOG_ERR("Sidewalk error: %d", (int)sid_get_error(app_ctx->handle));
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		break;
	}

	application_state_registered(&global_state_notifier,
				     status->detail.registration_status == SID_STATUS_REGISTERED);
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

static void on_sidewalk_factory_reset(void *context)
{
	ARG_UNUSED(context);

	LOG_DBG("factory reset notification received from sid api");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Reboot type not supported");
	}
}

sid_error_t sidewalk_callbacks_set(void *context, struct sid_event_callbacks *callbacks)
{
	if (!callbacks) {
		return SID_ERROR_INVALID_ARGS;
	}
	callbacks->context = context;
	callbacks->on_event = on_sidewalk_event;
	callbacks->on_msg_received = on_sidewalk_msg_received;                          /* Called from sid_process() */
	callbacks->on_msg_sent = on_sidewalk_msg_sent;                                  /* Called from sid_process() */
	callbacks->on_send_error = on_sidewalk_send_error;                              /* Called from sid_process() */
	callbacks->on_status_changed = on_sidewalk_status_changed;                      /* Called from sid_process() */
	callbacks->on_factory_reset = on_sidewalk_factory_reset;                        /* Called from sid_process() */

	return SID_ERROR_NONE;
}
