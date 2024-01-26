/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sidewalk.h>
#include <app_ble_config.h>
#if !defined(CONFIG_APP_BLE_ONLY)
#include <app_subGHz_config.h>
#endif
#include <sid_hal_reset_ifc.h>
#include <sidewalk_version.h>
#if defined(CONFIG_GPIO)
#include <state_notifier_gpio_backend.h>
#endif
#if defined(CONFIG_LOG)
#include <state_notifier_log_backend.h>
#endif
#if defined(CONFIG_APP_PERSISTENT_LINK_MASK)
#include <settings_utils.h>
#endif
#include <buttons.h>
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_SIDEWALK_LOG_LEVEL);

static uint8_t echo_payload[255];
static uint32_t persistent_link_mask;

static void on_sidewalk_event(bool in_isr, void *context)
{
	int err = sidewalk_event_send(SID_EVENT_SIDEWALK);
	if (err) {
		LOG_ERR("send event err %d", err);
	};
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	LOG_DBG("received message(type: %d, link_mode: %d, id: %u size %u)", (int)msg_desc->type,
		(int)msg_desc->link_mode, msg_desc->id, msg->size);
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");

	if (msg_desc->type == SID_MSG_TYPE_GET || msg_desc->type == SID_MSG_TYPE_SET) {
		memcpy(echo_payload, msg->data, msg->size);
		sidewalk_msg_t echo_msg = {
			.msg = {
				.data = echo_payload,
				.size = msg->size,
			},
			.desc = {
				.type = (msg_desc->type == SID_MSG_TYPE_GET)?SID_MSG_TYPE_RESPONSE : SID_MSG_TYPE_NOTIFY,
				.id = (msg_desc->type == SID_MSG_TYPE_GET)? msg_desc->id: msg_desc->id + 1,
				.link_type = SID_LINK_TYPE_ANY,
				.link_mode = SID_LINK_MODE_CLOUD,
			},
		};

		int err = sidewalk_msg_set(&echo_msg);
		if (err) {
			LOG_ERR("msg set err %d", err);
			return;
		};
		err = sidewalk_event_send(SID_EVENT_SEND_MSG);
		if (err) {
			LOG_ERR("send event err %d", err);
		};
	};
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	LOG_ERR("failed to send message(type: %d, id: %u), err:%d", (int)msg_desc->type,
		msg_desc->id, (int)error);
}

static void on_sidewalk_factory_reset(void *context)
{
	ARG_UNUSED(context);

	LOG_INF("factory reset notification received from sid api");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Reboot type not supported");
	}
}

static void on_control_event_notify(const struct sid_control_event_data *data, void *context)
{
	if (data == NULL || data->event_data == NULL) {
		LOG_ERR("on_control_event_notify called with invalid data");
		return;
	}
	LOG_INF("on_control_event_notify called");
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	int err = 0;
	uint32_t new_link_mask = status->detail.link_status_mask;

	switch (status->state) {
	case SID_STATE_READY:
	case SID_STATE_SECURE_CHANNEL_READY:
		err = sidewalk_event_send(SID_EVENT_STATE_READY);
		application_state_connected(&global_state_notifier, true);
		LOG_INF("status changed: ready");
		break;
	case SID_STATE_NOT_READY:
		err = sidewalk_event_send(SID_EVENT_STATE_NOT_READY);
		application_state_connected(&global_state_notifier, false);
		LOG_INF("status changed: not ready");
		break;
	case SID_STATE_ERROR:
		err = sidewalk_event_send(SID_EVENT_STATE_ERROR);
		application_state_error(&global_state_notifier, true);
		LOG_INF("status not changed: error");
		break;
	}

	if (err) {
		LOG_ERR("send event err %d", err);
	}

	application_state_registered(&global_state_notifier,
				     status->detail.registration_status == SID_STATUS_REGISTERED);
	application_state_time_sync(&global_state_notifier,
				    status->detail.time_sync_status == SID_STATUS_TIME_SYNCED);

#if defined(CONFIG_APP_PERSISTENT_LINK_MASK)
	if (new_link_mask && persistent_link_mask != new_link_mask) {
		err = settings_utils_link_mask_set(new_link_mask);
		if (err) {
			LOG_ERR("New link mask set err %d", err);
		} else {
			persistent_link_mask = new_link_mask;
			LOG_DBG("New link mask set 0x%x", persistent_link_mask);
		}
	}
#endif

	LOG_INF("Device %sregistered, Time Sync %s, Link status: {BLE: %s, FSK: %s, LoRa: %s}",
		(SID_STATUS_REGISTERED == status->detail.registration_status) ? "Is " : "Un",
		(SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) ? "Success" : "Fail",
		(new_link_mask & SID_LINK_TYPE_1) ? "Up" : "Down",
		(new_link_mask & SID_LINK_TYPE_2) ? "Up" : "Down",
		(new_link_mask & SID_LINK_TYPE_3) ? "Up" : "Down");

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

static void app_button_handler(uint32_t event)
{
	switch (event) {
	case SID_EVENT_SEND_MSG:
		static char payload[] = "hello";
		sidewalk_msg_t hello_msg = {
			.msg = {
				.data = payload,
				.size = sizeof(payload),
			},
			.desc = {
				.type = SID_MSG_TYPE_NOTIFY,
				.link_type = SID_LINK_TYPE_ANY,
				.link_mode = SID_LINK_MODE_CLOUD,
			},
		};
		int err = sidewalk_msg_set(&hello_msg);
		if (err) {
			LOG_ERR("msg set err %d", err);
			return;
		};
		break;
	case SID_EVENT_FACTORY_RESET:
#if defined(CONFIG_APP_PERSISTENT_LINK_MASK)
		(void)settings_utils_link_mask_set(0);
#endif
		break;
	default:
		break;
	}

	sidewalk_event_send((sidewalk_event_t)event);
}

static int app_buttons_init(void)
{
	button_set_action_short_press(DK_BTN1, app_button_handler, SID_EVENT_SEND_MSG);
	button_set_action_long_press(DK_BTN1, app_button_handler, SID_EVENT_NORDIC_DFU);
	button_set_action_short_press(DK_BTN2, app_button_handler, SID_EVENT_CONNECT);
	button_set_action_long_press(DK_BTN2, app_button_handler, SID_EVENT_FACTORY_RESET);
	button_set_action(DK_BTN3, app_button_handler, SID_EVENT_LINK_SWITCH);

	return buttons_init();
}

int main(void)
{
	PRINT_SIDEWALK_VERSION();

	if (app_buttons_init()) {
		LOG_ERR("Cannot init buttons");
	}

#if defined(CONFIG_GPIO)
	state_watch_init_gpio(&global_state_notifier);
#endif
#if defined(CONFIG_LOG)
	state_watch_init_log(&global_state_notifier);
#endif
	application_state_working(&global_state_notifier, true);

#if defined(CONFIG_APP_PERSISTENT_LINK_MASK)
	int err = settings_utils_link_mask_get(&persistent_link_mask);
	if (err) {
		LOG_ERR("link mask get err %d", err);
		persistent_link_mask = 0;
	}
#endif

	static sidewalk_ctx_t sid_ctx = { 0 };

	static struct sid_event_callbacks event_callbacks = {
		.context = &sid_ctx,
		.on_event = on_sidewalk_event,
		.on_msg_received = on_sidewalk_msg_received,
		.on_msg_sent = on_sidewalk_msg_sent,
		.on_send_error = on_sidewalk_send_error,
		.on_status_changed = on_sidewalk_status_changed,
		.on_factory_reset = on_sidewalk_factory_reset,
		.on_control_event_notify = on_control_event_notify,
	};

	sid_ctx.config = (struct sid_config)
	{
		.link_mask = persistent_link_mask, .callbacks = &event_callbacks,
		.link_config = app_get_ble_config(),
#if !defined(CONFIG_APP_BLE_ONLY)
		.sub_ghz_link_config = app_get_sub_ghz_config(),
#else
		.sub_ghz_link_config = NULL,
#endif /* CONFIG_APP_BLE_ONLY */
	};

	sidewalk_start(&sid_ctx);

	return 0;
}
