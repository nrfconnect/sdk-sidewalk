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
#include <sid_hal_memory_ifc.h>
#include <zephyr/kernel.h>
#include <json_printer/sidTypes2Json.h>
#include <zephyr/logging/log.h>

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
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("Message send success");
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_msg_sent", JSON_OBJ(JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 0))))));
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
	LOG_ERR("Message send err %d", (int)error);
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_send_error",
		JSON_OBJ(JSON_LIST_2(JSON_VAL_sid_error_t("error", error),
				     JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 0)))))));
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
	LOG_INF("sid_set_factory_reset success");
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
	sidewalk_event_send(SID_EVENT_NEW_STATUS, new_status);

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

	if (err) {
		LOG_ERR("Send event err %d", err);
	}

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

void app_start(void)
{
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
