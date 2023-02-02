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
#include <sid_ble_adapter.h>
#include <sid_ble_advert.h>
#include <sid_ble_connection.h>

#if defined(CONFIG_MAC_ADDRESS_TYPE_PUBLIC)
#include <zephyr/bluetooth/controller.h>
#endif /* CONFIG_MAC_ADDRESS_TYPE_PUBLIC */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(sid_ble, CONFIG_SIDEWALK_LOG_LEVEL);

K_SEM_DEFINE(ble_disconnect_complete, 0, 1);
static atomic_t manual_disconnect = ATOMIC_INIT(false);

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
	switch (err_code) {
	case -EALREADY:
		LOG_INF("BT already initialized");
	case 0:
		break;
	default:
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
	sid_ble_srv_params_t srv_params = {};

	switch (id) {
	case AMA_SERVICE:
	{
		srv_params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;
		srv_params.service = (struct bt_gatt_service_static *)sid_ble_get_ama_service();
		break;
	}
	#if defined(CONFIG_SIDEWALK_VENDOR_SERVICE)
	case VENDOR_SERVICE:
	{
		srv_params.uuid = VND_SID_BT_CHARACTERISTIC_NOTIFY;
		srv_params.service = (struct bt_gatt_service_static *)sid_ble_get_vnd_service();
		break;
	}
	#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */
	#if defined(CONFIG_SIDEWALK_LOGGING_SERVICE)
	case LOGGING_SERVICE:
	{
		srv_params.uuid = LOG_SID_BT_CHARACTERISTIC_NOTIFY;
		srv_params.service = (struct bt_gatt_service_static *)sid_ble_get_log_service();
		break;
	}
	#endif /* CONFIG_SIDEWALK_LOGGING_SERVICE */
	default:
		return SID_ERROR_NOSUPPORT;
	}

	srv_params.conn = sid_ble_conn_params_get()->conn;

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

void ble_adapter_disconnect_completed(void){
	if (atomic_test_bit(&manual_disconnect, 1)) {
		k_sem_give(&ble_disconnect_complete);
	}
}

static sid_error_t ble_adapter_disconnect(void)
{
	atomic_set_bit(&manual_disconnect, 1);
	int err = sid_ble_conn_disconnect();

	if (err) {
		LOG_ERR("Disconnection failed (err %d)", err);
		atomic_clear_bit(&manual_disconnect, 1);
		return SID_ERROR_GENERIC;
	}

	k_sem_take(&ble_disconnect_complete, K_FOREVER);
	atomic_clear_bit(&manual_disconnect, 1);
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_deinit(void)
{
	int err = bt_disable();

	if (err < 0 && -ENOTSUP != err) {
		LOG_ERR("BT disable failed (err %d)", err);
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
