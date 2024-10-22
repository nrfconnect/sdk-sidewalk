/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sidewalk.h>
#include <sensor_monitoring/app_tx.h>
#include <sensor_monitoring/app_rx.h>
#include <sensor_monitoring/app_buttons.h>
#include <sensor_monitoring/app_leds.h>
#include <app_ble_config.h>
#include <sidewalk_dfu/nordic_dfu.h>
#include <app_subGHz_config.h>
#include <sid_hal_reset_ifc.h>
#include <sid_hal_memory_ifc.h>
#include <buttons.h>
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>
#include <sid_demo_parser.h>
#include <json_printer/sidTypes2str.h>

LOG_MODULE_REGISTER(app, CONFIG_SIDEWALK_LOG_LEVEL);

#define PARAM_UNUSED (0U)
#define NOTIFY_TIMER_DURATION_MS (500)

K_THREAD_STACK_DEFINE(app_tx_stack, CONFIG_SID_END_DEVICE_TX_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(app_rx_stack, CONFIG_SID_END_DEVICE_RX_THREAD_STACK_SIZE);

static struct k_thread app_main;
static struct k_thread app_rx;

static void notify_timer_cb(struct k_timer *timer_id);
K_TIMER_DEFINE(notify_timer, notify_timer_cb, NULL);

static void notify_timer_cb(struct k_timer *timer_id)
{
	ARG_UNUSED(timer_id);
	app_tx_event_send(APP_EVENT_NOTIFY_SENSOR);
}

static sidewalk_ctx_t sid_ctx;

static void on_sidewalk_event(bool in_isr, void *context)
{
	int err = sidewalk_event_send(sidewalk_event_process, NULL, NULL);
	if (err) {
		LOG_ERR("Send event err %d", err);
	};
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	LOG_DBG("Received message(type: %d, link_mode: %d, id: %u size %u)", (int)msg_desc->type,
		(int)msg_desc->link_mode, msg_desc->id, msg->size);
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Received message: ");

	if (msg_desc->type == SID_MSG_TYPE_RESPONSE && msg_desc->msg_desc_attr.rx_attr.is_msg_ack) {
		LOG_DBG("Received Ack for msg id %d", msg_desc->id);
	} else {
		struct app_rx_msg rx_msg = { 0 };
		rx_msg.pld_size = MIN(msg->size, APP_RX_PAYLOAD_MAX_SIZE);
		memcpy(rx_msg.rx_payload, msg->data, rx_msg.pld_size);
		int err = app_rx_msg_received(&rx_msg);
		if (err) {
			LOG_ERR("Rx msg err %d", err);
		}
	}
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
	sidewalk_msg_t *message = get_message_buffer(msg_desc->id);
	if (message == NULL) {
		LOG_ERR("failed to find message buffer to clean");
		return;
	}
	sid_hal_free(message->msg.data);
	sid_hal_free(message);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	LOG_ERR("Send message err %d (%s)", (int)error, SID_ERROR_T_STR(error));
	LOG_DBG("Failed to send message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
	sidewalk_msg_t *message = get_message_buffer(msg_desc->id);
	if (message == NULL) {
		LOG_ERR("failed to find message buffer to clean");
		return;
	}
	sid_hal_free(message->msg.data);
	sid_hal_free(message);
}

static void on_sidewalk_factory_reset(void *context)
{
	ARG_UNUSED(context);

	LOG_INF("Factory reset notification received from sid api");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Cannot reboot");
	}
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	struct sid_status *new_status = sid_hal_malloc(sizeof(struct sid_status));
	if (!new_status) {
		LOG_ERR("Failed to allocate memory for new status value");
	} else {
		memcpy(new_status, status, sizeof(struct sid_status));
	}
	sidewalk_event_send(sidewalk_event_new_status, new_status, sid_hal_free);

	int err = 0;
	switch (status->state) {
	case SID_STATE_READY:
	case SID_STATE_SECURE_CHANNEL_READY:
		LOG_INF("Status changed: ready");
		break;
	case SID_STATE_NOT_READY:
		LOG_INF("Status changed: not ready");
		break;
	case SID_STATE_ERROR:
		LOG_INF("Status not changed: error");
		break;
	}

	app_tx_last_link_mask_set(status->detail.link_status_mask);

	if (SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) {
		err = app_tx_event_send(APP_EVENT_TIME_SYNC_SUCCESS);
	} else {
		err = app_tx_event_send(APP_EVENT_TIME_SYNC_FAIL);
	}

	if (err) {
		LOG_ERR("Send event err %d", err);
	}

	LOG_INF("Device %sregistered, Time Sync %s, Link status: {BLE: %s, FSK: %s, LoRa: %s}",
		(SID_STATUS_REGISTERED == status->detail.registration_status) ? "Is " : "Un",
		(SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) ? "Success" : "Fail",
		(status->detail.link_status_mask & SID_LINK_TYPE_1) ? "Up" : "Down",
		(status->detail.link_status_mask & SID_LINK_TYPE_2) ? "Up" : "Down",
		(status->detail.link_status_mask & SID_LINK_TYPE_3) ? "Up" : "Down");

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
		sidewalk_event_send(sidewalk_event_exit, NULL, NULL);
		sidewalk_event_send(app_event_enter_dfu_mode, NULL, NULL);
	} else {
		sidewalk_event_send(app_event_exit_dfu_mode, NULL, NULL);
		sidewalk_event_send(sidewalk_event_autostart, NULL, NULL);
	}

	go_to_dfu_state = !go_to_dfu_state;
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
	button_set_action_short_press(DK_BTN1, app_btn_event_handler, DEMO_BTN_ID_0);
	button_set_action_short_press(DK_BTN2, app_btn_event_handler, DEMO_BTN_ID_1);
	button_set_action_short_press(DK_BTN3, app_btn_event_handler, DEMO_BTN_ID_2);
	button_set_action_short_press(DK_BTN4, app_btn_event_handler, DEMO_BTN_ID_3);

	button_set_action_long_press(DK_BTN1, app_btn_dfu_state, PARAM_UNUSED);
	button_set_action_long_press(DK_BTN2, app_btn_factory_reset, PARAM_UNUSED);
	button_set_action_long_press(DK_BTN3, app_btn_link_switch, PARAM_UNUSED);

	return buttons_init();
}

void app_start_tasks(void)
{
	(void)k_thread_create(&app_main, app_tx_stack, K_THREAD_STACK_SIZEOF(app_tx_stack),
			      app_tx_task, NULL, NULL, NULL,
			      CONFIG_SID_END_DEVICE_TX_THREAD_PRIORITY, 0, K_NO_WAIT);

	(void)k_thread_create(&app_rx, app_rx_stack, K_THREAD_STACK_SIZEOF(app_rx_stack),
			      app_rx_task, NULL, NULL, NULL,
			      CONFIG_SID_END_DEVICE_RX_THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(&app_main, "app_main");
	k_thread_name_set(&app_rx, "app_rx");
}

void app_start(void)
{
	if (app_buttons_init()) {
		LOG_ERR("Cannot init buttons");
	}

	if (app_led_init()) {
		LOG_ERR("Cannot init leds");
	}

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
		.link_mask = 0,
		.dev_ch = dev_ch,
		.callbacks = &event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	app_start_tasks();
	sidewalk_start(&sid_ctx);
	sidewalk_event_send(sidewalk_event_platform_init, NULL, NULL);
	sidewalk_event_send(sidewalk_event_autostart, NULL, NULL);

	k_timer_start(&notify_timer, K_MSEC(NOTIFY_TIMER_DURATION_MS),
		      K_MSEC(CONFIG_SID_END_DEVICE_NOTIFY_DATA_PERIOD_MS));
}
