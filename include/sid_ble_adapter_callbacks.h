/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_adapter_callbacks.h
 *  @brief Common callbacks API for Sidewalk.
 */

#ifndef SID_PAL_BLE_ADAPTER_CALLBACKS_H
#define SID_PAL_BLE_ADAPTER_CALLBACKS_H

#include <sid_error.h>
#include <sid_pal_ble_adapter_ifc.h>

/**
 * @brief Set a callback for notification sent.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_notification_cb_set(sid_pal_ble_indication_callback_t cb);

/**
 * @brief Execution notification callback.
 *
 */
void sid_ble_adapter_notification_sent(void);

/**
 * @brief Set a callback for a data handling.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_data_cb_set(sid_pal_ble_data_callback_t cb);

/**
 * @brief Execute callback after data receiving.
 *
 * @param id service identifier.
 * @param data buffer with data.
 * @param length data buffer length.
 */
void sid_ble_adapter_data_write(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length);

/**
 * @brief Set a callback for notification subscription change.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_notification_changed_cb_set(sid_pal_ble_notify_callback_t cb);

/**
 * @brief Execute callback when notification has been changed.
 *
 * @param id service identifier.
 * @param state notification state.
 */
void sid_ble_adapter_notification_changed(sid_ble_cfg_service_identifier_t id, bool state);

/**
 * @brief Set a callback for connection change.
 *
 * @param cb a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_conn_cb_set(sid_pal_ble_connection_callback_t cb);

/**
 * @brief Execute callback after BLE device connected.
 *
 * @param ble_addr BLE address.
 */
void sid_ble_adapter_conn_connected(const uint8_t *ble_addr);

/**
 * @brief Execute callback after BLE device disconnected.
 * 
 * @param ble_addr BLE address.
 */
void sid_ble_adapter_conn_disconnected(const uint8_t *ble_addr);

#endif /* SID_PAL_BLE_ADAPTER_CALLBACKS_H */
