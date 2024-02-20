/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app.h>
#include <sidewalk.h>
#include <app_ble_config.h>
#include <app_subGHz_config.h>
#include <sid_hal_reset_ifc.h>
#if defined(CONFIG_GPIO)
#include <state_notifier_gpio_backend.h>
#endif
#if defined(CONFIG_LOG)
#include <state_notifier_log_backend.h>
#endif
#include <buttons.h>
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

#include <json_printer.h>
#include <sidTypes2Json.h>

LOG_MODULE_REGISTER(app, CONFIG_SIDEWALK_LOG_LEVEL);

static uint32_t persistent_link_mask;

static void on_sidewalk_event(bool in_isr, void *context)
{
	int err = sidewalk_event_send(SID_EVENT_SIDEWALK, NULL);
	if (err) {
		LOG_ERR("Send event err %d", err);
	};
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message received success");
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_msg_received", JSON_OBJ(JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 1))))));

	application_state_receiving(&global_state_notifier, true);
	application_state_receiving(&global_state_notifier, false);

#ifdef CONFIG_TEMPLATE_APP_ECHO_MSGS
	if (msg_desc->type == SID_MSG_TYPE_GET || msg_desc->type == SID_MSG_TYPE_SET) {
		LOG_INF("Send echo message");
		sidewalk_msg_t *echo = sidewalk_data_alloc(sizeof(sidewalk_msg_t));

		echo->msg.size = msg->size;
		echo->msg.data = sidewalk_data_alloc(echo->msg.size);
		memcpy(echo->msg.data, msg->data, echo->msg.size);

		echo->desc.type = (msg_desc->type == SID_MSG_TYPE_GET) ? SID_MSG_TYPE_RESPONSE :
									 SID_MSG_TYPE_NOTIFY;
		echo->desc.id =
			(msg_desc->type == SID_MSG_TYPE_GET) ? msg_desc->id : msg_desc->id + 1;
		echo->desc.link_type = SID_LINK_TYPE_ANY;
		echo->desc.link_mode = SID_LINK_MODE_CLOUD;

		int err = sidewalk_event_send(SID_EVENT_SEND_MSG, echo);
		if (err) {
			sidewalk_data_free(echo->msg.data);
			sidewalk_data_free(echo);
			LOG_ERR("Send event err %d", err);
		} else {
			application_state_sending(&global_state_notifier, true);
		}
	};
#endif /* CONFIG_TEMPLATE_APP_ECHO_MSGS */
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("Message send success");
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_msg_sent", JSON_OBJ(JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 0))))));

	application_state_sending(&global_state_notifier, false);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	LOG_ERR("Message send err %d", (int)error);
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_send_error",
		JSON_OBJ(JSON_LIST_2(JSON_VAL_sid_error_t("error", error),
				     JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 0)))))));

	application_state_sending(&global_state_notifier, false);
}

static void on_sidewalk_factory_reset(void *context)
{
	ARG_UNUSED(context);
#ifndef CONFIG_TEMPLATE_APP_CLI
	LOG_INF("Factory reset notification received from sid api");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Cannot reboot");
	}
#else
	LOG_INF("sid_set_factory_reset success");
#endif
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	int err = 0;
	uint32_t new_link_mask = status->detail.link_status_mask;
	struct sid_status *new_status = sidewalk_data_alloc(sizeof(struct sid_status));
	memcpy(new_status, status, sizeof(struct sid_status));
	sidewalk_event_send(SID_EVENT_NEW_STATUS, new_status);

	switch (status->state) {
	case SID_STATE_READY:
	case SID_STATE_SECURE_CHANNEL_READY:
		application_state_connected(&global_state_notifier, true);
		LOG_INF("Status changed: ready");
		break;
	case SID_STATE_NOT_READY:
		application_state_connected(&global_state_notifier, false);
		LOG_INF("Status changed: not ready");
		break;
	case SID_STATE_ERROR:
		application_state_error(&global_state_notifier, true);
		LOG_INF("Status not changed: error");
		break;
	}

	if (err) {
		LOG_ERR("Send event err %d", err);
	}

	application_state_registered(&global_state_notifier,
				     status->detail.registration_status == SID_STATUS_REGISTERED);
	application_state_time_sync(&global_state_notifier,
				    status->detail.time_sync_status == SID_STATUS_TIME_SYNCED);

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
	if (event == SID_EVENT_SEND_MSG) {
		LOG_INF("Send hello message");
		const char payload[] = "hello";
		sidewalk_msg_t *hello = sidewalk_data_alloc(sizeof(sidewalk_msg_t));

		hello->msg.size = sizeof(payload);
		hello->msg.data = sidewalk_data_alloc(hello->msg.size);
		memcpy(hello->msg.data, payload, hello->msg.size);

		hello->desc.type = SID_MSG_TYPE_NOTIFY;
		hello->desc.link_type = SID_LINK_TYPE_ANY;
		hello->desc.link_mode = SID_LINK_MODE_CLOUD;

		int err = sidewalk_event_send(SID_EVENT_SEND_MSG, hello);
		if (err) {
			sidewalk_data_free(hello->msg.data);
			sidewalk_data_free(hello);
			LOG_ERR("Send event err %d", err);
		} else {
			application_state_sending(&global_state_notifier, true);
		}
		return;
	}

	sidewalk_event_send((sidewalk_event_t)event, NULL);
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

void app_start(void)
{
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

	static sidewalk_ctx_t sid_ctx = { 0 };

	static struct sid_event_callbacks event_callbacks = {
		.context = &sid_ctx,
		.on_event = on_sidewalk_event,
		.on_msg_received = on_sidewalk_msg_received,
		.on_msg_sent = on_sidewalk_msg_sent,
		.on_send_error = on_sidewalk_send_error,
		.on_status_changed = on_sidewalk_status_changed,
		.on_factory_reset = on_sidewalk_factory_reset,
	};

	sid_ctx.config = (struct sid_config){
		.link_mask = persistent_link_mask,
		.callbacks = &event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	sidewalk_start(&sid_ctx);
}
