/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_adapter.c
 *  @brief Bluetooth low energy adapter implementation.
 */

#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_service.h>
#include <sid_ble_advert.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>
#include <settings/settings.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(sid_ble, CONFIG_SIDEWALK_LOG_LEVEL);

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg);
static sid_error_t ble_adapter_start_service(void);
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length);
static sid_error_t ble_adapter_start_advertisement(void);
static sid_error_t ble_adapter_stop_advertisement(void);
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length);
static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb);
static sid_error_t ble_adapter_disconnect(void);
static sid_error_t ble_adapter_deinit(void);

static void ble_ev_connected(struct bt_conn *conn, uint8_t err);
static void ble_ev_disconnected(struct bt_conn *conn, uint8_t reason);

static struct sid_pal_ble_adapter_interface ble_ifc = {
	.init = ble_adapter_init,
	.start_service = ble_adapter_start_service,
	.set_adv_data = ble_adapter_set_adv_data,
	.start_adv = ble_adapter_start_advertisement,
	.stop_adv = ble_adapter_stop_advertisement,
	.send = ble_adapter_send_data,
	.set_callback = ble_adapter_set_callback,
	.disconnect = ble_adapter_disconnect,
	.deinit = ble_adapter_deinit,
};

static sid_pal_ble_connection_callback_t connection_callback;

typedef struct {
	const sid_ble_config_t *cfg;
	struct bt_conn *curr_conn;
	uint8_t curr_conn_addr[BLE_ADDR_MAX_LEN];
} sid_pal_ble_adapter_ctx_t;

static sid_pal_ble_adapter_ctx_t ctx;

/* BLE connection callbacks. */
static struct bt_conn_cb conn_callbacks = {
	.connected = ble_ev_connected,
	.disconnected = ble_ev_disconnected,
};

/**
 * @brief The function is called when a new connection is established.
 *
 * @param conn new connection object.
 * @param err HCI error, zero for success, non-zero otherwise.
 */
static void ble_ev_connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Connection failed (err %u)\n", err);
		return;
	}

	memcpy(ctx.curr_conn_addr, bt_conn_get_dst(conn)->a.val, BLE_ADDR_MAX_LEN);
	LOG_DBG("Connected: %02X:%02X:%02X:%02X:%02X:%02X",
		ctx.curr_conn_addr[5], ctx.curr_conn_addr[4], ctx.curr_conn_addr[3],
		ctx.curr_conn_addr[2], ctx.curr_conn_addr[1], ctx.curr_conn_addr[0]);

	ctx.curr_conn = bt_conn_ref(conn);

	if (connection_callback) {
		connection_callback(true, ctx.curr_conn_addr);
	}
}

/**
 * @brief The function is called when a connection has been disconnected.
 *
 * @param conn connection object.
 * @param err HCI disconnection reason.
 */
static void ble_ev_disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_DBG("Disconnected: %s (reason %u)\n", addr, reason);

	if (ctx.curr_conn) {
		bt_conn_unref(ctx.curr_conn);
		ctx.curr_conn = NULL;
		memset(ctx.curr_conn_addr, 0x00, BLE_ADDR_MAX_LEN);
	}

	if (connection_callback) {
		connection_callback(false, ctx.curr_conn_addr);
	}
}

sid_error_t sid_ble_set_connection_cb(sid_pal_ble_connection_callback_t cb)
{
	if (NULL == cb) {
		return SID_ERROR_INVALID_ARGS;
	}

	connection_callback = cb;
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg)
{
	LOG_DBG("Enable BT");

	if (!cfg) {
		return SID_ERROR_INVALID_ARGS;
	}

	ctx.cfg = cfg;

	int err_code;
	err_code = bt_enable(NULL);
	if (err_code) {
		LOG_ERR("BT init failed (err %d)", err_code);
		return SID_ERROR_GENERIC;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		err_code = settings_load();
		if (err_code) {
			LOG_ERR("settings load failed (err %d)", err_code);
			return SID_ERROR_GENERIC;
		}
	}

	bt_conn_cb_register(&conn_callbacks);

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_service(void)
{
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length)
{
	if (!data || 0 == length) {
		return SID_ERROR_INVALID_ARGS;
	}

	int err = sid_ble_advert_update(data, length);

	if (err) {
		LOG_ERR("Advertising failed to update (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_advertisement(void)
{
	int err = sid_ble_advert_start();

	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_stop_advertisement(void)
{
	int err = sid_ble_advert_stop();

	if (err) {
		LOG_ERR("Advertising failed to stop (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
	const struct bt_gatt_service_static *srv = NULL;
	struct bt_uuid *uuid;

	switch (id) {
	case AMA_SERVICE:
	{
		uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;
		srv = get_ama_service();
		break;
	}
	case VENDOR_SERVICE:
	case LOGGING_SERVICE:
	default:
		return SID_ERROR_NOSUPPORT;
	}

	return sid_ble_send_data(ctx.curr_conn, uuid, srv, data, length);
}

static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb)
{
	sid_error_t erc;

	if (!cb) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!cb->conn_callback   ||
	    !cb->mtu_callback    ||
	    !cb->adv_start_callback) {
		return SID_ERROR_INVALID_ARGS;
	}

	erc = sid_ble_set_notification_cb(cb->ind_callback);
	if (SID_ERROR_NONE != erc) {
		return erc;
	}

	erc = sid_ble_set_data_cb(cb->data_callback);
	if (SID_ERROR_NONE != erc) {
		return erc;
	}

	erc = sid_ble_set_notification_changed_cb(cb->notify_callback);
	if (SID_ERROR_NONE != erc) {
		return erc;
	}

	erc = sid_ble_set_connection_cb(cb->conn_callback);
	if (SID_ERROR_NONE != erc) {
		return erc;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_disconnect(void)
{
	return SID_ERROR_NOSUPPORT;
}

static sid_error_t ble_adapter_deinit(void)
{
	memset(&ctx, 0x00, sizeof(ctx));
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle)
{
	if (!handle) {
		return SID_ERROR_INVALID_ARGS;
	}

	*handle = &ble_ifc;

	return SID_ERROR_NONE;
}
