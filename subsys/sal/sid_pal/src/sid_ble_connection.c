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

static sid_ble_conn_data_t conn_data;
static sid_ble_conn_data_t *conn_data_ptr;
static struct bt_le_conn_param conn_params_next;
static struct bt_le_conn_param conn_params_prev = {
	.interval_min = CONFIG_BT_PERIPHERAL_PREF_MIN_INT,
	.interval_max = CONFIG_BT_PERIPHERAL_PREF_MAX_INT,
	.latency = CONFIG_BT_PERIPHERAL_PREF_LATENCY,
	.timeout = CONFIG_BT_PERIPHERAL_PREF_TIMEOUT,
};

static struct bt_conn_cb conn_callbacks = {
	.connected = ble_connect_cb,
	.disconnected = ble_disconnect_cb,
};

static struct bt_gatt_cb gatt_callbacks = { .att_mtu_updated = ble_mtu_cb };

static int ble_conn_param_get(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	struct bt_conn_info info = { 0 };
	int err = bt_conn_get_info(conn, &info);
	if (err) {
		return err;
	}

	param->interval_max = info.le.interval;
	param->interval_min = info.le.interval;
	param->latency = info.le.latency;
	param->timeout = info.le.timeout;

	return 0;
}

static bool ble_conn_is_valid(struct bt_conn *conn)
{
	struct bt_conn_info conn_info = {};

	if (!conn || bt_conn_get_info(conn, &conn_info) || conn_info.id != BT_ID_SIDEWALK) {
		return false;
	}

	return true;
}

static void ble_connect_cb(struct bt_conn *conn, uint8_t conn_err)
{
	const bt_addr_le_t *bt_addr_le = NULL;
	int err = 0;

	if (!ble_conn_is_valid(conn)) {
		return;
	}

	if (conn_err) {
		LOG_ERR("Connection failed (err %u)\n", conn_err);
		return;
	}

	sid_ble_advert_notify_connection();

	bt_addr_le = bt_conn_get_dst(conn);
	if (bt_addr_le) {
		memcpy(conn_data.addr, bt_addr_le->a.val, BT_ADDR_SIZE);
	} else {
		LOG_ERR("Connection bt address not found.");
		memset(conn_data.addr, 0x00, BT_ADDR_SIZE);
	}

	k_mutex_lock(&bt_conn_mutex, K_FOREVER);
	conn_data.conn = bt_conn_ref(conn);

	sid_ble_adapter_conn_connected((const uint8_t *)conn_data.addr);
	k_mutex_unlock(&bt_conn_mutex);

	err = bt_conn_le_param_update(conn, &conn_params_next);
	if (err) {
		LOG_WRN("bt_conn_le_param_update failed with error: %d = %s", err, strerror(err));
	}

	LOG_INF("BT Connected");
}

static void ble_disconnect_cb(struct bt_conn *conn, uint8_t reason)
{
	if (!ble_conn_is_valid(conn) || conn_data.conn != conn) {
		return;
	}

	int err = ble_conn_param_get(conn, &conn_params_prev);
	if (err) {
		LOG_ERR("Connection param get failed (err=%d)", err);
		return;
	}

	sid_ble_adapter_conn_disconnected((const uint8_t *)conn_data.addr);

	k_mutex_lock(&bt_conn_mutex, K_FOREVER);
	bt_conn_unref(conn_data.conn);
	conn_data.conn = NULL;
	k_mutex_unlock(&bt_conn_mutex);

	LOG_INF("BT Disconnected Reason: 0x%x = %s", reason, HCI_err_to_str(reason));
}

static void ble_mtu_cb(struct bt_conn *conn, uint16_t tx_mtu, uint16_t rx_mtu)
{
	ARG_UNUSED(rx_mtu);

	if (!conn_data.conn || conn_data.conn == conn) {
		sid_ble_adapter_mtu_changed(MIN(tx_mtu, rx_mtu));
	}
}

int sid_ble_conn_param_get(struct bt_le_conn_param *param)
{
	if (!param) {
		return -EINVAL;
	}

	if (conn_data.conn) {
		int err = ble_conn_param_get(conn_data.conn, &conn_params_prev);
		if (err) {
			LOG_ERR("Connection param get failed (err=%d)", err);
			return err;
		}
	}

	memcpy(param, &conn_params_prev, sizeof(conn_params_prev));

	return 0;
}

int sid_ble_conn_param_update(const struct bt_le_conn_param *param)
{
	int err = 0;

	if (!param) {
		return -EINVAL;
	}

	if (conn_data.conn) {
		err = bt_conn_le_param_update(conn_data.conn, param);
		if (err) {
			LOG_WRN("bt_conn_le_param_update failed with error: %d = %s", err,
				strerror(err));
		}
	}

	memcpy(&conn_params_next, param, sizeof(struct bt_le_conn_param));

	return 0;
}

const sid_ble_conn_data_t *sid_ble_conn_data_get(void)
{
	return (const sid_ble_conn_data_t *)conn_data_ptr;
}

void sid_ble_conn_init(void)
{
	conn_data_ptr = &conn_data;
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
	if (!conn_data.conn) {
		return -ENOENT;
	}

	k_mutex_lock(&bt_conn_mutex, K_FOREVER);
	int err = bt_conn_disconnect(conn_data.conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);

	k_mutex_unlock(&bt_conn_mutex);

	return err;
}

void sid_ble_conn_deinit(void)
{
	conn_data_ptr = NULL;
}
