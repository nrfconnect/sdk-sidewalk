/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_ble_connection.h>
#include <sid_ble_adapter_callbacks.h>
#include <sid_ble_advert.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

#include <errno.h>
#include <hci_utils.h>
#include <bt_app_callbacks.h>

#include <zephyr/kernel.h>
K_MUTEX_DEFINE(bt_conn_mutex);

LOG_MODULE_REGISTER(sid_ble_conn, CONFIG_SIDEWALK_LOG_LEVEL);

static void ble_connect_cb(struct bt_conn *conn, uint8_t err);
static void ble_disconnect_cb(struct bt_conn *conn, uint8_t reason);
static void ble_mtu_cb(struct bt_conn *conn, uint16_t tx_mtu, uint16_t rx_mtu);

static sid_ble_conn_params_t conn_params;
static sid_ble_conn_params_t *p_conn_params_out;

static struct bt_conn_cb conn_callbacks = {
	.connected = ble_connect_cb,
	.disconnected = ble_disconnect_cb,
};

static struct bt_gatt_cb gatt_callbacks = { .att_mtu_updated = ble_mtu_cb };

/**
 * @brief Check if the connection came from right adv, and if it is valid.
 * 
 * @param conn connection to check
 * @return true if the connection should be handled by Sidewalk
 * @return false connection is not for Sidewlak
 */
static bool is_connection_valid(struct bt_conn *conn)
{
	struct bt_conn_info conn_info = {};

	if (!conn || bt_conn_get_info(conn, &conn_info) || conn_info.id != BT_ID_SIDEWALK) {
		return false;
	}

	return true;
}

/**
 * @brief The function is called when a new connection is established.
 *
 * @param conn new connection object.
 * @param err HCI error, zero for success, non-zero otherwise.
 */
static void ble_connect_cb(struct bt_conn *conn, uint8_t err)
{
	const bt_addr_le_t *bt_addr_le = NULL;

	if (!is_connection_valid(conn)) {
		return;
	}

	if (err) {
		LOG_ERR("Connection failed (err %u)\n", err);
		return;
	}
	sid_ble_advert_notify_connection();
	bt_addr_le = bt_conn_get_dst(conn);

	if (bt_addr_le) {
		memcpy(conn_params.addr, bt_addr_le->a.val, BT_ADDR_SIZE);
	} else {
		LOG_ERR("Connection bt address not found.");
		memset(conn_params.addr, 0x00, BT_ADDR_SIZE);
	}

	k_mutex_lock(&bt_conn_mutex, K_FOREVER);
	conn_params.conn = bt_conn_ref(conn);

	sid_ble_adapter_conn_connected((const uint8_t *)conn_params.addr);
	k_mutex_unlock(&bt_conn_mutex);

	LOG_INF("BT Connected");
}

/**
 * @brief The function is called when a connection has been disconnected.
 *
 * @param conn connection object.
 * @param err HCI disconnection reason.
 */
static void ble_disconnect_cb(struct bt_conn *conn, uint8_t reason)
{
	if (!is_connection_valid(conn) || conn_params.conn != conn) {
		return;
	}

	sid_ble_adapter_conn_disconnected((const uint8_t *)conn_params.addr);

	k_mutex_lock(&bt_conn_mutex, K_FOREVER);
	bt_conn_unref(conn_params.conn);
	conn_params.conn = NULL;
	k_mutex_unlock(&bt_conn_mutex);

	LOG_INF("BT Disconnected Reason: 0x%x = %s", reason, HCI_err_to_str(reason));
}

static void ble_mtu_cb(struct bt_conn *conn, uint16_t tx_mtu, uint16_t rx_mtu)
{
	ARG_UNUSED(rx_mtu);

	if (!conn_params.conn || conn_params.conn == conn) {
		sid_ble_adapter_mtu_changed(MIN(tx_mtu, rx_mtu));
	}
}

const sid_ble_conn_params_t *sid_ble_conn_params_get(void)
{
	return (const sid_ble_conn_params_t *)p_conn_params_out;
}

void sid_ble_conn_init(void)
{
	p_conn_params_out = &conn_params;
	static bool bt_conn_registered;

	if (!bt_conn_registered) {
		int e = bt_conn_cb_register(&conn_callbacks);
		switch (e) {
		case 0:
		case -EEXIST:
			break;
		default: {
			LOG_ERR("bt_conn_cb_register failed with error: %d = %s", e, strerror(e));
			return;
		}
		}
		bt_gatt_cb_register(&gatt_callbacks);
		bt_conn_registered = true;
	}
}

int sid_ble_conn_disconnect(void)
{
	if (!conn_params.conn) {
		return -ENOENT;
	}

	k_mutex_lock(&bt_conn_mutex, K_FOREVER);
	int err = bt_conn_disconnect(conn_params.conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);

	k_mutex_unlock(&bt_conn_mutex);

	return err;
}

void sid_ble_conn_deinit(void)
{
	p_conn_params_out = NULL;
}
