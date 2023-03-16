/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_ble_adapter_callbacks.c
 *  @brief Common callbacks implementation for Sidewalk.
 */

#include <sid_ble_adapter_callbacks.h>

#include <zephyr/types.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_ble_adapter_callbacks, CONFIG_SIDEWALK_BLE_ADAPTER_LOG_LEVEL);

#define CALLBACK_SET(__target_cb, __source_cb)	       \
	do {					       \
		if (NULL == __source_cb) {	       \
			return SID_ERROR_INVALID_ARGS; \
		}				       \
		__target_cb = __source_cb;	       \
	} while (0)

static sid_pal_ble_data_callback_t data_cb;
static sid_pal_ble_notify_callback_t notify_changed_cb;
static sid_pal_ble_indication_callback_t notify_sent_cb;
static sid_pal_ble_connection_callback_t connection_cb;
static sid_pal_ble_mtu_callback_t mtu_changed_cb;
static sid_pal_ble_adv_start_callback_t adv_start_cb;

sid_error_t sid_ble_adapter_notification_cb_set(sid_pal_ble_indication_callback_t cb)
{
	CALLBACK_SET(notify_sent_cb, cb);
	return SID_ERROR_NONE;
}

void sid_ble_adapter_notification_sent(void)
{
	LOG_DBG("BLE -> Sidewalk");
	if (notify_sent_cb) {
		notify_sent_cb(true);
	}
}

sid_error_t sid_ble_adapter_data_cb_set(sid_pal_ble_data_callback_t cb)
{
	CALLBACK_SET(data_cb, cb);
	return SID_ERROR_NONE;
}

void sid_ble_adapter_data_write(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
	LOG_DBG("BLE -> Sidewalk");
	if (data_cb) {
		data_cb(id, data, length);
	}
}

sid_error_t sid_ble_adapter_notification_changed_cb_set(sid_pal_ble_notify_callback_t cb)
{
	CALLBACK_SET(notify_changed_cb, cb);
	return SID_ERROR_NONE;
}

void sid_ble_adapter_notification_changed(sid_ble_cfg_service_identifier_t id, bool state)
{
	LOG_DBG("BLE -> Sidewalk");
	if (notify_changed_cb) {
		notify_changed_cb(id, state);
	}
}

sid_error_t sid_ble_adapter_conn_cb_set(sid_pal_ble_connection_callback_t cb)
{
	CALLBACK_SET(connection_cb, cb);
	return SID_ERROR_NONE;
}

void sid_ble_adapter_conn_connected(const uint8_t *ble_addr)
{
	LOG_DBG("BLE -> Sidewalk");
	if (connection_cb) {
		connection_cb(true, (uint8_t *)ble_addr);
	}
}

void sid_ble_adapter_conn_disconnected(const uint8_t *ble_addr)
{
	LOG_DBG("BLE -> Sidewalk");
	if (connection_cb) {
		connection_cb(false, (uint8_t *)ble_addr);
	}
}

sid_error_t sid_ble_adapter_mtu_cb_set(sid_pal_ble_mtu_callback_t cb)
{
	LOG_DBG("BLE -> Sidewalk");
	CALLBACK_SET(mtu_changed_cb, cb);
	return SID_ERROR_NONE;
}

void sid_ble_adapter_mtu_changed(uint16_t mtu_size)
{
	LOG_DBG("BLE -> Sidewalk");
	if (mtu_changed_cb) {
		mtu_changed_cb(mtu_size);
	}
}

sid_error_t sid_ble_adapter_adv_start_cb_set(sid_pal_ble_adv_start_callback_t cb)
{
	CALLBACK_SET(adv_start_cb, cb);
	return SID_ERROR_NONE;
}

void sid_ble_adapter_adv_started(void)
{
	LOG_DBG("BLE -> Sidewalk");
	if (adv_start_cb) {
		adv_start_cb();
	}
}
