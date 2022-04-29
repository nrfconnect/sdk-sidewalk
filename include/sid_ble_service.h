/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_service.h
 *  @brief Bluetooth low energy service API.
 */

#ifndef SID_PAL_BLE_SERVICE_H
#define SID_PAL_BLE_SERVICE_H

#include <sid_ble_uuid.h>
#include <sid_error.h>
#include <sid_ble_config_ifc.h>
#include <sid_pal_ble_adapter_ifc.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>

/* AMA_SERVICE */
#define AMA_SID_BT_UUID_SERVICE             BT_UUID_DECLARE_16(AMA_SERVICE_UUID_VAL)
#define AMA_SID_BT_CHARACTERISTIC_WRITE     BT_UUID_DECLARE_128(AMA_CHARACTERISTIC_UUID_VAL_WRITE)
#define AMA_SID_BT_CHARACTERISTIC_NOTIFY    BT_UUID_DECLARE_128(AMA_CHARACTERISTIC_UUID_VAL_NOTIFY)

/**
 * @brief Get the Amazon service object.
 * 
 * @return struct bt_gatt_service_static* 
 */
const struct bt_gatt_service_static *get_ama_service(void);

/**
 * @brief Send data over BLE.
 *
 * @param conn BLE connection.
 * @param uuid service identifier.
 * @param service pointer to service structure.
 * @param data buffer with data.
 * @param length data buffer length.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_send_data(struct bt_conn *conn, const struct bt_uuid *uuid,
			      const struct bt_gatt_service_static *service,
			      uint8_t *data, uint16_t length);

/**
 * @brief Set a callback for notification sent.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_set_notification_cb(sid_pal_ble_indication_callback_t cb);

/**
 * @brief Set a callback for a data handling.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_set_data_cb(sid_pal_ble_data_callback_t cb);

/**
 * @brief Set a callback for notification subscription change.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_set_notification_changed_cb(sid_pal_ble_notify_callback_t cb);

#endif /* SID_PAL_BLE_SERVICE_H */
