/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sm_callbacks.h>
#include <sm_task.h>
#include <sm_utils.h>
#include <sm_timers.h>
#include <sid_hal_reset_ifc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_callbacks, CONFIG_SIDEWALK_LOG_LEVEL);

static const uint8_t *status_name[] = {
	"ready", "not ready", "Error", "secure channel ready"
};

static const uint8_t *link_mode_name[] = {
	"none", [SID_LINK_MODE_CLOUD] = "cloud", [SID_LINK_MODE_MOBILE] = "mobile"
};

static const uint8_t *link_mode_idx_name[] = {
	"ble", "fsk", "lora"
};

static void cb_sid_event(bool in_isr, void *context)
{
	LOG_DBG("Event from %s, context %p", in_isr ? "ISR" : "App", context);
	sm_main_task_msg_q_write(EVENT_TYPE_SIDEWALK);
}

static void cb_sid_msg_received(const struct sid_msg_desc *msg_desc,
				const struct sid_msg *msg, void *context)
{
	LOG_INF("Received message(type: %d, link_mode: %d, id: %u size %u rssi %d snr %d)",
		(int)msg_desc->type, (int)msg_desc->link_mode, msg_desc->id, msg->size,
		(int)msg_desc->msg_desc_attr.rx_attr.rssi, (int)msg_desc->msg_desc_attr.rx_attr.snr);
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");

	if (msg_desc->type == SID_MSG_TYPE_RESPONSE && msg_desc->msg_desc_attr.rx_attr.is_msg_ack) {
		LOG_INF("Received Ack for msg id %d", msg_desc->id);
	} else {
		struct app_demo_rx_msg rx_msg = {
			.msg_id = msg_desc->id,
			.pld_size = msg->size,
		};
		memcpy(rx_msg.rx_payload, msg->data, MIN(msg->size, PAYLOAD_MAX_SIZE));
		sm_rx_task_msg_q_write(&rx_msg);
	}
}

static void cb_sid_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("Sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void cb_sid_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_ERR("Failed to send message(type: %d, id: %u), err:%d",
		(int)msg_desc->type, msg_desc->id, (int)error);
}

static void cd_sid_status_changed(const struct sid_status *status, void *context)
{
	LOG_INF("Status changed: %s", status_name[status->state]);

	app_context_t *app_context = (app_context_t *)context;

	switch (status->state) {
	case SID_STATE_READY:
		app_context->sidewalk_state = STATE_SIDEWALK_READY;
		break;
	case SID_STATE_NOT_READY:
		app_context->sidewalk_state = STATE_SIDEWALK_NOT_READY;
		break;
	case SID_STATE_ERROR:
		LOG_ERR("Sidewalk error: %d", (int)sid_get_error(app_context->sidewalk_handle));
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		app_context->sidewalk_state = STATE_SIDEWALK_SECURE_CONNECTION;
		break;
	}

	LOG_INF("Device %sregistered, Time Sync %s, Link Status Mask = %x",
		(status->detail.registration_status == SID_STATUS_REGISTERED) ? "Is " : "Un",
		(status->detail.time_sync_status == SID_STATUS_TIME_SYNCED) ? "Success" : "Fail",
		status->detail.link_status_mask);

	app_context->link_status.link_mask = status->detail.link_status_mask;
	app_context->link_status.time_sync_status = status->detail.time_sync_status;

	for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
		enum sid_link_mode mode = (enum sid_link_mode)status->detail.supported_link_modes[i];
		app_context->link_status.supported_link_mode[i] = mode;

		if ((mode == SID_LINK_MODE_CLOUD) || (mode == SID_LINK_MODE_MOBILE)) {
			LOG_INF("Link mode %s, on %s", link_mode_name[mode], link_mode_idx_name[i]);
		}
	}

	if (app_context->app_state == DEMO_APP_STATE_INIT &&
	    status->detail.registration_status == SID_STATUS_REGISTERED) {
		app_context->app_state = DEMO_APP_STATE_REGISTERED;
	}

	if (app_context->sidewalk_state == STATE_SIDEWALK_READY) {
		k_timeout_t delay = K_MSEC(DEMO_CAPABILITY_PERIOD_MS);

		if (app_context->app_state == DEMO_APP_STATE_REGISTERED) {
			app_context->app_state = DEMO_APP_STATE_NOTIFY_CAPABILITY;
		} else if (app_context->app_state == DEMO_APP_STATE_NOTIFY_SENSOR_DATA) {
			delay = K_MSEC(DEMO_NOTIFY_SENSOR_DATA_PERIOD_MS);
		}
		sm_cap_timer_set_and_run(delay);
	}

	if ((status->detail.registration_status == SID_STATUS_REGISTERED)
	    && (status->detail.time_sync_status == SID_STATUS_TIME_SYNCED)) {
		if (BUILT_IN_LM == SID_LINK_TYPE_1 &&
		    !(status->detail.link_status_mask & SID_LINK_TYPE_1)) {
			sm_cap_timer_set_and_run(K_MSEC(CONNECT_LINK_TYPE_1_INIT_DELAY_MS));
		}

		if (BUILT_IN_LM == SID_LINK_TYPE_2 &&
		    status->detail.link_status_mask & SID_LINK_TYPE_2) {
			sm_device_profile_timer_set_and_run(K_MSEC(PROFILE_CHECK_TIMER_DELAY_MS));
		}
	}
}

static void cb_sid_factory_reset(void *context)
{
	ARG_UNUSED(context);

	LOG_DBG("Factory reset notification received from sid api");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Reboot type not supported");
	}
}

sid_error_t sm_callbacks_set(void *ctx, struct sid_event_callbacks *cb)
{
	if (!cb || !ctx) {
		return SID_ERROR_NULL_POINTER;
	}

	cb->context = (app_context_t *)ctx;
	cb->on_event = cb_sid_event;                            /* Called from ISR context */
	cb->on_msg_received = cb_sid_msg_received;              /* Called from sid_process() */
	cb->on_msg_sent = cb_sid_msg_sent;                      /* Called from sid_process() */
	cb->on_send_error = cb_sid_send_error;                  /* Called from sid_process() */
	cb->on_status_changed = cd_sid_status_changed;          /* Called from sid_process() */
	cb->on_factory_reset = cb_sid_factory_reset;            /* Called from sid_process() */

	return SID_ERROR_NONE;
}
