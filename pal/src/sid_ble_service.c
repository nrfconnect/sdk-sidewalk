/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_service.c
 *  @brief Bluetooth low energy service implementation.
 */

#include <sid_ble_service.h>

#include <zephyr/types.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/bluetooth.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(sid_ble_srv, CONFIG_SIDEWALK_LOG_LEVEL);

static struct bt_gatt_notify_params not_params;
static sid_pal_ble_data_callback_t data_cb;
static sid_pal_ble_notify_callback_t notify_changed_cb;
static sid_pal_ble_indication_callback_t notify_send_cb;

static void ama_srv_notif_changed(const struct bt_gatt_attr *attr,
				  uint16_t value);
static ssize_t ama_srv_on_write(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				const void *buf,
				uint16_t len,
				uint16_t offset,
				uint8_t flags);

/* AMA_SERVICE definition */
BT_GATT_SERVICE_DEFINE(
	ama_service,
	BT_GATT_PRIMARY_SERVICE(AMA_SID_BT_UUID_SERVICE),
	BT_GATT_CHARACTERISTIC(
		AMA_SID_BT_CHARACTERISTIC_WRITE,
		BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		BT_GATT_PERM_WRITE,
		NULL, ama_srv_on_write, NULL),
	BT_GATT_CHARACTERISTIC(
		AMA_SID_BT_CHARACTERISTIC_NOTIFY,
		BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_NONE,
		NULL, NULL, NULL),
	BT_GATT_CCC(ama_srv_notif_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	);

static void ama_srv_notif_changed(const struct bt_gatt_attr *attr,
				  uint16_t value)
{
	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	ARG_UNUSED(attr);
	LOG_DBG("Notification %s", notif_enabled ? "enabled" : "disabled");
	if (notify_changed_cb) {
		notify_changed_cb(AMA_SERVICE, notif_enabled);
	}
}

static ssize_t ama_srv_on_write(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				const void *buf,
				uint16_t len,
				uint16_t offset,
				uint8_t flags)
{
	ARG_UNUSED(attr);
	ARG_UNUSED(conn);
	ARG_UNUSED(offset);
	ARG_UNUSED(flags);

	LOG_DBG("Data received for AMA_SERVICE [len=%d]", len);

	if (data_cb) {
		data_cb(AMA_SERVICE, (uint8_t *)buf, len);
	}
	return len;
}

static void notification_sent(struct bt_conn *conn, void *user_data)
{
	ARG_UNUSED(user_data);
	ARG_UNUSED(conn);

	LOG_DBG("Notification sent.");

	if (notify_send_cb) {
		notify_send_cb(true);
	}
}

const struct bt_gatt_service_static *sid_ble_get_ama_service(void)
{
	return &ama_service;
}

sid_error_t sid_ble_send_data(struct bt_conn *conn, const struct bt_uuid *uuid,
			      const struct bt_gatt_service_static *service,
			      uint8_t *data, uint16_t length)
{
	int error_code;
	const struct bt_gatt_attr *srv_attrs = NULL;
	uint16_t srv_attr_count = 0;
	const struct bt_gatt_attr *attr = NULL;

	if (NULL != service) {
		srv_attrs = service->attrs;
		srv_attr_count = service->attr_count;
	}

	attr = bt_gatt_find_by_uuid(srv_attrs,
				    srv_attr_count,
				    uuid);

	if (!attr) {
		LOG_ERR("Attribute not found");
		return SID_ERROR_NULL_POINTER;
	}

	if (!data || !length || bt_gatt_get_mtu(conn) < length ||
	    !bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY)) {
		return SID_ERROR_INVALID_ARGS;
	}

	memset(&not_params, 0, sizeof(not_params));

	not_params.attr = attr;
	not_params.data = data;
	not_params.len = length;
	not_params.func = notification_sent;

	error_code = bt_gatt_notify_cb(conn, &not_params);
	if (error_code) {
		LOG_ERR("Send err:%d", error_code);
		return SID_ERROR_GENERIC;
	}
	return SID_ERROR_NONE;
}

sid_error_t sid_ble_set_notification_cb(sid_pal_ble_indication_callback_t cb)
{
	if (NULL == cb) {
		return SID_ERROR_INVALID_ARGS;
	}

	notify_send_cb = cb;
	return SID_ERROR_NONE;
}

sid_error_t sid_ble_set_data_cb(sid_pal_ble_data_callback_t cb)
{
	if (NULL == cb) {
		return SID_ERROR_INVALID_ARGS;
	}

	data_cb = cb;
	return SID_ERROR_NONE;
}

sid_error_t sid_ble_set_notification_changed_cb(sid_pal_ble_notify_callback_t cb)
{
	if (NULL == cb) {
		return SID_ERROR_INVALID_ARGS;
	}

	notify_changed_cb = cb;
	return SID_ERROR_NONE;
}
