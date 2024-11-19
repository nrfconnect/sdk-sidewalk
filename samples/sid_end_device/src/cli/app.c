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

#include <sid_ble_uuid.h>
#include <bt_app_callbacks.h>
#include <zephyr/bluetooth/gatt.h>
#if defined(CONFIG_SIDEWALK_DFU)
#include <zephyr/mgmt/mcumgr/transport/smp_bt.h>
#endif //defined(CONFIG_SIDEWALK_DFU)

LOG_MODULE_REGISTER(app, CONFIG_SIDEWALK_LOG_LEVEL);

static uint32_t persistent_link_mask;

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
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message received success");
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_msg_received", JSON_OBJ(JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 1))))));
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("Message send success");
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_msg_sent", JSON_OBJ(JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 0))))));
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	LOG_ERR("Message send err %d", (int)error);
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_send_error",
		JSON_OBJ(JSON_LIST_2(JSON_VAL_sid_error_t("error", error),
				     JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 0)))))));
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
	sidewalk_event_send(sidewalk_event_new_status, new_status, sid_hal_free);

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

static bool gatt_authorize(struct bt_conn *conn, const struct bt_gatt_attr *attr)
{
	struct bt_conn_info cinfo = {};
	int ret = bt_conn_get_info(conn, &cinfo);
	if (ret != 0) {
		LOG_ERR("Failed to get id of connection err %d", ret);
		return false;
	}
	char uuid_s[50] = "";
	bt_uuid_to_str(attr->uuid, uuid_s, sizeof(uuid_s));
	LOG_DBG("GATT authorize : conn_id = %d, attr %s", cinfo.id, uuid_s);

	if (cinfo.id == BT_ID_SIDEWALK) {
		if (bt_uuid_cmp(attr->uuid, BT_UUID_DECLARE_128(SMP_BT_CHR_UUID_VAL)) == 0) {
			LOG_WRN("Block SMP_BT_CHR_UUID_VAL in Sidewalk connection");
			return false;
		}
	}

#if defined(CONFIG_SIDEWALK_DFU)
	if (cinfo.id == BT_ID_SMP_DFU) {
		if (bt_uuid_cmp(attr->uuid,
				BT_UUID_DECLARE_128(AMA_CHARACTERISTIC_UUID_VAL_WRITE)) == 0) {
			LOG_WRN("block AMA_CHARACTERISTIC_UUID_VAL_WRITE in DFU connection");
			return false;
		}
		if (bt_uuid_cmp(attr->uuid,
				BT_UUID_DECLARE_128(AMA_CHARACTERISTIC_UUID_VAL_NOTIFY)) == 0) {
			LOG_WRN("block AMA_CHARACTERISTIC_UUID_VAL_NOTIFY in DFU connection");
			return false;
		}

		if (bt_uuid_cmp(attr->uuid,
				BT_UUID_DECLARE_128(VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE)) ==
		    0) {
			LOG_WRN("block VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE in DFU connection");
			return false;
		}
		if (bt_uuid_cmp(attr->uuid,
				BT_UUID_DECLARE_128(VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY)) ==
		    0) {
			LOG_WRN("block VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY in DFU connection");
			return false;
		}

		if (bt_uuid_cmp(attr->uuid,
				BT_UUID_DECLARE_128(LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE)) ==
		    0) {
			LOG_WRN("block LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE in DFU connection");
			return false;
		}
		if (bt_uuid_cmp(attr->uuid,
				BT_UUID_DECLARE_128(LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY)) ==
		    0) {
			LOG_WRN("block LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY in DFU connection");
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
}
