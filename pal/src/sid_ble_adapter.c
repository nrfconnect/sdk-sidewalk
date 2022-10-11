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
#include <sid_ble_ama_service.h>
#if defined(CONFIG_SIDEWALK_VENDOR_SERVICE)
#include <sid_ble_vnd_service.h>
#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */
#if defined(CONFIG_SIDEWALK_LOGGING_SERVICE)
#include <sid_ble_log_service.h>
#endif /* CONFIG_SIDEWALK_LOGGING_SERVICE */
#include <sid_ble_adapter_callbacks.h>
#include <sid_ble_advert.h>
#include <sid_ble_connection.h>

#if defined(CONFIG_MAC_ADDRESS_TYPE_PUBLIC)
#include <bluetooth/controller.h>
#endif /* CONFIG_MAC_ADDRESS_TYPE_PUBLIC */

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

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg)
{
	ARG_UNUSED(cfg);

	LOG_DBG("Enable BT");

#if defined(CONFIG_MAC_ADDRESS_TYPE_PUBLIC)
	bt_addr_t pub_addr;
	bt_addr_from_str(CONFIG_SID_BT_PUB_ADDR, &pub_addr);
	bt_ctlr_set_public_addr(pub_addr.val);
#endif /* CONFIG_MAC_ADDRESS_TYPE_PUBLIC */

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
	} else {
		return SID_ERROR_INVALID_ARGS;
	}

	sid_ble_conn_init();

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
	struct bt_uuid *uuid = NULL;
	sid_ble_srv_params_t srv_params;

	switch (id) {
	case AMA_SERVICE:
	{
		uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;
		srv = sid_ble_get_ama_service();
		break;
	}
	#if defined(CONFIG_SIDEWALK_VENDOR_SERVICE)
	case VENDOR_SERVICE:
	{
		uuid = VND_SID_BT_CHARACTERISTIC_NOTIFY;
		srv = sid_ble_get_vnd_service();
		break;
	}
	#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */
	#if defined(CONFIG_SIDEWALK_LOGGING_SERVICE)
	case LOGGING_SERVICE:
	{
		uuid = LOG_SID_BT_CHARACTERISTIC_NOTIFY;
		srv = sid_ble_get_log_service();
		break;
	}
	#endif /* CONFIG_SIDEWALK_LOGGING_SERVICE */
	default:
		return SID_ERROR_NOSUPPORT;
	}

	const sid_ble_conn_params_t *params = sid_ble_conn_params_get();

	srv_params.uuid = uuid;
	srv_params.service = (struct bt_gatt_service_static *)srv;
	srv_params.conn = params->conn;

	int err_code = sid_ble_send_data(&srv_params, data, length);
	if (-EINVAL == err_code) {
		return SID_ERROR_INVALID_ARGS;
	} else if (0 > err_code) {
		return SID_ERROR_GENERIC;
	}
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb)
{
	sid_error_t err_code;

	if (!cb) {
		return SID_ERROR_NULL_POINTER;
	}

	err_code = sid_ble_adapter_notification_cb_set(cb->ind_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_data_cb_set(cb->data_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_notification_changed_cb_set(cb->notify_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_conn_cb_set(cb->conn_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_mtu_cb_set(cb->mtu_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_adv_start_cb_set(cb->adv_start_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_disconnect(void)
{
	int err = sid_ble_conn_disconnect();

	if (err) {
		LOG_ERR("Disconnection failed (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_deinit(void)
{
	int err = bt_disable();
	if (err) {
		LOG_ERR("BT disable failed (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	sid_ble_conn_deinit();

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
