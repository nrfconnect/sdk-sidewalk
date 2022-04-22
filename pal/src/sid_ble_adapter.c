/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_adapter.c
 *  @brief Bluetooth low energy adapter implementation.
 */

#include <sid_pal_ble_adapter_ifc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
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

#define BT_UUID_AMA_VAL 0x03FE

struct sid_manuf_data{
	uint16_t vendor_id;
	uint8_t sidewalk_app_id;
	uint8_t device_state;
	uint8_t frame_indicator;
	uint8_t tx_uuid[5];
};

/* Advertising parameters. */
#define SIDEWALK_BT_LE_ADV_PARAM		\
	BT_LE_ADV_PARAM(			\
		BT_LE_ADV_OPT_CONNECTABLE |	\
		BT_LE_ADV_OPT_USE_NAME |	\
		BT_LE_ADV_OPT_FORCE_NAME_IN_AD,	\
		BT_GAP_ADV_FAST_INT_MIN_2,	\
		BT_GAP_ADV_FAST_INT_MAX_2, NULL	\
		)

static const struct sid_manuf_data manufacturing_data = {
	.vendor_id = 0x0171,
	.sidewalk_app_id = 0x21,
	.device_state = 0x13,
	.frame_indicator = 0x80,
	.tx_uuid = {0xA0, 0x01, 0x02, 0x03, 0x04}
};
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_AMA_VAL)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, (uint8_t*)&manufacturing_data, sizeof(manufacturing_data))
};
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

typedef struct {
	const sid_ble_config_t *cfg;
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
	if (0 == err) {
		LOG_DBG("BLE connected.");
	} else {
		LOG_ERR("Connection failed (err %u).", err);
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
	LOG_DBG("BLE disconnected, reason=%d.", reason);
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
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length)
{
	return SID_ERROR_NOSUPPORT;
}

static sid_error_t ble_adapter_start_advertisement(void)
{
	int err = bt_le_adv_start(SIDEWALK_BT_LE_ADV_PARAM, ad, ARRAY_SIZE(ad), NULL, 0);

	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}
static sid_error_t ble_adapter_stop_advertisement(void)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb)
{
	return SID_ERROR_NOSUPPORT;
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
