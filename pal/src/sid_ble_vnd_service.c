/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_vnd_service.c
 *  @brief Bluetooth low energy vendor service implementation.
 */

#include <sid_ble_vnd_service.h>
#include <sid_ble_adapter_callbacks.h>

#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_ble_vnd_srv, CONFIG_SIDEWALK_LOG_LEVEL);

static void vnd_srv_notif_changed(const struct bt_gatt_attr *attr,
				  uint16_t value);
static ssize_t vnd_srv_on_write(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				const void *buf,
				uint16_t len,
				uint16_t offset,
				uint8_t flags);

/* VENDOR_SERVICE definition */
BT_GATT_SERVICE_DEFINE(
	vnd_service,
	BT_GATT_PRIMARY_SERVICE(VND_SID_BT_UUID_SERVICE),
	BT_GATT_CHARACTERISTIC(
		VND_SID_BT_CHARACTERISTIC_WRITE,
		BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		BT_GATT_PERM_WRITE,
		NULL, vnd_srv_on_write, NULL),
	BT_GATT_CHARACTERISTIC(
		VND_SID_BT_CHARACTERISTIC_NOTIFY,
		BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_NONE,
		NULL, NULL, NULL),
	BT_GATT_CCC(vnd_srv_notif_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	);

static void vnd_srv_notif_changed(const struct bt_gatt_attr *attr,
				  uint16_t value)
{
	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	ARG_UNUSED(attr);
	LOG_DBG("Notification for VENDOR_SERVICE is %s.",
		notif_enabled ? "enabled" : "disabled");
	sid_ble_adapter_notification_changed(VENDOR_SERVICE, notif_enabled);
}

static ssize_t vnd_srv_on_write(struct bt_conn *conn,
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

	LOG_DBG("Data received for VENDOR_SERVICE [len=%d].", len);

	sid_ble_adapter_data_write(VENDOR_SERVICE, (uint8_t *)buf, len);
	return len;
}

const struct bt_gatt_service_static *sid_ble_get_vnd_service(void)
{
	return &vnd_service;
}
