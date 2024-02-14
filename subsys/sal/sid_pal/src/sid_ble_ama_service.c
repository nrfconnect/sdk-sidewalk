/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_ble_ama_service.c
 *  @brief Bluetooth low energy Amazon service implementation.
 */
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/hci.h>
#include <sdc_hci_vs.h>

#include <sid_ble_ama_service.h>
#include <sid_ble_adapter_callbacks.h>

#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_ble_ama_srv, CONFIG_SIDEWALK_LOG_LEVEL);

static void ama_srv_notif_changed(const struct bt_gatt_attr *attr, uint16_t value);
static ssize_t ama_srv_on_write(struct bt_conn *conn, const struct bt_gatt_attr *attr,
				const void *buf, uint16_t len, uint16_t offset, uint8_t flags);

/* AMA_SERVICE definition */
BT_GATT_SERVICE_DEFINE(ama_service, BT_GATT_PRIMARY_SERVICE(AMA_SID_BT_UUID_SERVICE),
		       BT_GATT_CHARACTERISTIC(AMA_SID_BT_CHARACTERISTIC_WRITE,
					      BT_GATT_CHRC_WRITE_WITHOUT_RESP, BT_GATT_PERM_WRITE,
					      NULL, ama_srv_on_write, NULL),
		       BT_GATT_CHARACTERISTIC(AMA_SID_BT_CHARACTERISTIC_NOTIFY, BT_GATT_CHRC_NOTIFY,
					      BT_GATT_PERM_NONE, NULL, NULL, NULL),
		       BT_GATT_CCC(ama_srv_notif_changed,
				   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), );

static void ama_srv_notif_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	ARG_UNUSED(attr);
	LOG_DBG("Notification %s", notif_enabled ? "enabled" : "disabled");
	sid_ble_adapter_notification_changed(AMA_SERVICE, notif_enabled);
}

static ssize_t ama_srv_on_write(struct bt_conn *conn, const struct bt_gatt_attr *attr,
				const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	ARG_UNUSED(attr);
	ARG_UNUSED(conn);
	ARG_UNUSED(offset);
	ARG_UNUSED(flags);

	// avg[n] = gamma * avg[n - 1] + (1 - gamma) * rssi[n]
	// Here, gamma equals beta/4096, and rssi[n] equals the current RSSI.
	// leave only the last rssi in the average.
	const sdc_hci_cmd_vs_set_power_control_request_params_t parameters_for_rssi_calculation = {
		.beta = 4096
	};
	sdc_hci_cmd_vs_set_power_control_request_params(&parameters_for_rssi_calculation);

	uint16_t conn_handle = 0;
	bt_hci_get_conn_handle(conn, &conn_handle);
	const sdc_hci_cmd_vs_read_average_rssi_t p_params = { .conn_handle = conn_handle };
	sdc_hci_cmd_vs_read_average_rssi_return_t p_return;
	sdc_hci_cmd_vs_read_average_rssi(&p_params, &p_return);

	LOG_DBG("BLE RSSI = %d", p_return.avg_rssi);
	LOG_DBG("Data received for AMA_SERVICE [len=%d].", len);

	sid_ble_adapter_data_write(AMA_SERVICE, (uint8_t *)buf, len);
	return len;
}

const struct bt_gatt_service_static *sid_ble_get_ama_service(void)
{
	return &ama_service;
}
