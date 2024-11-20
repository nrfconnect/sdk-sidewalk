/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <state_notifier/state_notifier.h>
#include <app.h>
#include <sidewalk.h>
#include <app_ble_config.h>
#include <app_subGHz_config.h>
#include <sid_hal_reset_ifc.h>
#include <sid_hal_memory_ifc.h>
#include <stdbool.h>
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
#include <sbdt/dfu_file_transfer.h>
#endif

#include <bt_app_callbacks.h>

#if defined(CONFIG_GPIO)
#include <state_notifier/notifier_gpio.h>
#endif
#if defined(CONFIG_LOG)
#include <state_notifier/notifier_log.h>
#endif
#include <sidewalk_dfu/nordic_dfu.h>
#include <buttons.h>
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

#include <json_printer/sidTypes2Json.h>
#include <json_printer/sidTypes2str.h>

LOG_MODULE_REGISTER(app, CONFIG_SIDEWALK_LOG_LEVEL);

#define PARAM_UNUSED (0U)

static uint32_t persistent_link_mask;

static void on_sidewalk_event(bool in_isr, void *context)
{
	int err = sidewalk_event_send(sidewalk_event_process, NULL, NULL);
	if (err) {
		LOG_ERR("Send event err %d", err);
	};
}

static void free_sid_echo_event_ctx(void *ctx)
{
	sidewalk_msg_t *echo = (sidewalk_msg_t *)ctx;
	if (echo == NULL) {
		return;
	}
	if (echo->msg.data) {
		sid_hal_free(echo->msg.data);
	}
	sid_hal_free(echo);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message received success");
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_msg_received", JSON_OBJ(JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 1))))));

	application_state_receiving(&global_state_notifier, true);
	application_state_receiving(&global_state_notifier, false);

#ifdef CONFIG_SID_END_DEVICE_ECHO_MSGS
	if (msg_desc->type == SID_MSG_TYPE_GET || msg_desc->type == SID_MSG_TYPE_SET) {
		LOG_INF("Send echo message");
		sidewalk_msg_t *echo = sid_hal_malloc(sizeof(sidewalk_msg_t));
		if (!echo) {
			LOG_ERR("Failed to allocate event context for echo message");
			return;
		}
		memset(echo, 0x0, sizeof(*echo));
		echo->msg.size = msg->size;
		echo->msg.data = sid_hal_malloc(echo->msg.size);
		if (!echo->msg.data) {
			LOG_ERR("Failed to allocate memory for message echo data");
			sid_hal_free(echo);
			return;
		}
		memcpy(echo->msg.data, msg->data, echo->msg.size);

		echo->desc.type = (msg_desc->type == SID_MSG_TYPE_GET) ? SID_MSG_TYPE_RESPONSE :
									 SID_MSG_TYPE_NOTIFY;
		echo->desc.id =
			(msg_desc->type == SID_MSG_TYPE_GET) ? msg_desc->id : msg_desc->id + 1;
		echo->desc.link_type = SID_LINK_TYPE_ANY;
		echo->desc.link_mode = SID_LINK_MODE_CLOUD;

		int err =
			sidewalk_event_send(sidewalk_event_send_msg, echo, free_sid_echo_event_ctx);
		if (err) {
			free_sid_echo_event_ctx(echo);
			LOG_ERR("Send event err %d", err);
		} else {
			application_state_sending(&global_state_notifier, true);
		}
	};
#endif /* CONFIG_SID_END_DEVICE_ECHO_MSGS */
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
	LOG_ERR("Message send err %d (%s)", (int)error, SID_ERROR_T_STR(error));
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_send_error",
		JSON_OBJ(JSON_LIST_2(JSON_VAL_sid_error_t("error", error),
				     JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 0)))))));

	application_state_sending(&global_state_notifier, false);
}

static void on_sidewalk_factory_reset(void *context)
{
	ARG_UNUSED(context);
#ifndef CONFIG_SID_END_DEVICE_CLI
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
	struct sid_status *new_status = sid_hal_malloc(sizeof(struct sid_status));
	if (!new_status) {
		LOG_ERR("Failed to allocate memory for new status value");
	} else {
		memcpy(new_status, status, sizeof(struct sid_status));
	}
	err = sidewalk_event_send(sidewalk_event_new_status, new_status, sid_hal_free);

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
static void free_sid_hello_event_ctx(void *ctx)
{
	sidewalk_msg_t *hello = (sidewalk_msg_t *)ctx;
	if (hello == NULL) {
		return;
	}
	if (hello->msg.data) {
		sid_hal_free(hello->msg.data);
	}
	sid_hal_free(hello);
}

static void app_btn_send_msg(uint32_t unused)
{
	ARG_UNUSED(unused);

	LOG_INF("Send hello message");
	const char payload[] = "hello";
	sidewalk_msg_t *hello = sid_hal_malloc(sizeof(sidewalk_msg_t));
	if (!hello) {
		LOG_ERR("Failed to alloc memory for message context");
		return;
	}
	memset(hello, 0x0, sizeof(*hello));

	hello->msg.size = sizeof(payload);
	hello->msg.data = sid_hal_malloc(hello->msg.size);
	if (!hello->msg.data) {
		sid_hal_free(hello);
		LOG_ERR("Failed to allocate memory for message data");
		return;
	}
	memcpy(hello->msg.data, payload, hello->msg.size);

	hello->desc.type = SID_MSG_TYPE_NOTIFY;
	hello->desc.link_type = SID_LINK_TYPE_ANY;
	hello->desc.link_mode = SID_LINK_MODE_CLOUD;

	int err = sidewalk_event_send(sidewalk_event_send_msg, hello, free_sid_hello_event_ctx);
	if (err) {
		free_sid_hello_event_ctx(hello);
		LOG_ERR("Send event err %d", err);
	} else {
		application_state_sending(&global_state_notifier, true);
	}
}

static void app_event_exit_dfu_mode(sidewalk_ctx_t *sid, void *ctx)
{
	int err = -ENOTSUP;
	// Exit from DFU state
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	err = nordic_dfu_ble_stop();
#endif
	if (err) {
		LOG_ERR("dfu stop err %d", err);
	}
}

static void app_event_enter_dfu_mode(sidewalk_ctx_t *sid, void *ctx)
{
	int err = -ENOTSUP;

	LOG_INF("Entering into DFU mode");
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	err = nordic_dfu_ble_start();
#endif
	if (err) {
		LOG_ERR("dfu start err %d", err);
	}
}

static void app_btn_dfu_state(uint32_t unused)
{
	ARG_UNUSED(unused);
	static bool go_to_dfu_state = true;
	if (go_to_dfu_state) {
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
		sidewalk_event_send(sidewalk_event_file_transfer_deinit, NULL, NULL);
#endif
		sidewalk_event_send(app_event_enter_dfu_mode, NULL, NULL);
	} else {
		sidewalk_event_send(app_event_exit_dfu_mode, NULL, NULL);
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
		sidewalk_event_send(sidewalk_event_file_transfer_init, NULL, NULL);
#endif
	}

	go_to_dfu_state = !go_to_dfu_state;
}

static void app_btn_connect(uint32_t unused)
{
	ARG_UNUSED(unused);
	(void)sidewalk_event_send(sidewalk_event_connect, NULL, NULL);
}

static void app_btn_factory_reset(uint32_t unused)
{
	ARG_UNUSED(unused);
	(void)sidewalk_event_send(sidewalk_event_factory_reset, NULL, NULL);
}

static void app_btn_link_switch(uint32_t unused)
{
	ARG_UNUSED(unused);
	(void)sidewalk_event_send(sidewalk_event_link_switch, NULL, NULL);
}

static int app_buttons_init(void)
{
	button_set_action_short_press(DK_BTN1, app_btn_send_msg, PARAM_UNUSED);
	button_set_action_long_press(DK_BTN1, app_btn_dfu_state, PARAM_UNUSED);
	button_set_action_short_press(DK_BTN2, app_btn_connect, PARAM_UNUSED);
	button_set_action_long_press(DK_BTN2, app_btn_factory_reset, PARAM_UNUSED);
	button_set_action(DK_BTN3, app_btn_link_switch, PARAM_UNUSED);

	return buttons_init();
}

static bool gatt_authorize(struct bt_conn *conn, const struct bt_gatt_attr *attr)
{
	struct bt_conn_info cinfo = {};
	int ret = bt_conn_get_info(conn, &cinfo);
	if (ret != 0) {
		LOG_ERR("Failed to get id of connection err %d", ret);
		return false;
	}

	if (cinfo.id == BT_ID_SIDEWALK) {
		if (sid_ble_bt_attr_is_SMP(attr)) {
			return false;
		}
	}

#if defined(CONFIG_SIDEWALK_DFU)
	if (cinfo.id == BT_ID_SMP_DFU) {
		if (sid_ble_bt_attr_is_SIDEWALK(attr)) {
			return false;
		}
	}
#endif //defined(CONFIG_SIDEWALK_DFU)
	return true;
}

static const struct bt_gatt_authorization_cb gatt_authorization_callbacks = {
	.read_authorize = gatt_authorize,
	.write_authorize = gatt_authorize,
};

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

	struct sid_end_device_characteristics dev_ch = {
		.type = SID_END_DEVICE_TYPE_STATIC,
		.power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
		.qualification_id = 0x0001,
	};

	sid_ctx.config = (struct sid_config){
		.link_mask = persistent_link_mask,
		.dev_ch = dev_ch,
		.callbacks = &event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	int err = bt_gatt_authorization_cb_register(&gatt_authorization_callbacks);
	if (err) {
		LOG_ERR("Registering GATT authorization callbacks failed (err %d)", err);
		return;
	}
	sidewalk_start(&sid_ctx);
	sidewalk_event_send(sidewalk_event_platform_init, NULL, NULL);
	sidewalk_event_send(sidewalk_event_autostart, NULL, NULL);
}
