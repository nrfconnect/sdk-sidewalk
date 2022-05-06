/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_service.c
 *  @brief Bluetooth low energy service implementation.
 */

#include <sid_ble_service.h>
#include <sid_ble_adapter_callbacks.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(sid_ble_srv, CONFIG_SIDEWALK_LOG_LEVEL);

static struct bt_gatt_notify_params not_params;

static void notification_sent(struct bt_conn *conn, void *user_data)
{
	ARG_UNUSED(user_data);
	ARG_UNUSED(conn);

	LOG_DBG("Notification sent.");

	sid_ble_adapter_notification_sent();
}

int sid_ble_send_data(sid_ble_srv_params_t *params,
		      uint8_t *data, uint16_t length)
{
	int error_code;
	const struct bt_gatt_attr *srv_attrs = NULL;
	uint16_t srv_attr_count = 0;
	const struct bt_gatt_attr *attr = NULL;

	if (!params) {
		return -ENOENT;
	}

	if (NULL != params->service) {
		srv_attrs = params->service->attrs;
		srv_attr_count = params->service->attr_count;
	}

	attr = bt_gatt_find_by_uuid(srv_attrs,
				    srv_attr_count,
				    params->uuid);

	if (!attr) {
		LOG_ERR("Attribute not found.");
		return -ENOENT;
	}

	if (!data || !length || bt_gatt_get_mtu(params->conn) < length ||
	    !bt_gatt_is_subscribed(params->conn, attr, BT_GATT_CCC_NOTIFY)) {
		return -EINVAL;
	}

	memset(&not_params, 0, sizeof(not_params));

	not_params.attr = attr;
	not_params.data = data;
	not_params.len = length;
	not_params.func = notification_sent;

	error_code = bt_gatt_notify_cb(params->conn, &not_params);
	if (error_code) {
		LOG_ERR("Send err:%d.", error_code);
	}

	return error_code;
}
