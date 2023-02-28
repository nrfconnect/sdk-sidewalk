/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_log_service.h
 *  @brief Logging service.
 */

#ifndef SID_PAL_BLE_LOG_SERVICE_H
#define SID_PAL_BLE_LOG_SERVICE_H

#include <sid_ble_uuid.h>

/* LOGGING_SERVICE */
#define LOG_SID_BT_UUID_SERVICE             BT_UUID_DECLARE_16(LOG_EXAMPLE_SERVICE_UUID_VAL)
#define LOG_SID_BT_CHARACTERISTIC_WRITE     BT_UUID_DECLARE_128(LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE)
#define LOG_SID_BT_CHARACTERISTIC_NOTIFY    BT_UUID_DECLARE_128(LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY)

/**
 * @brief Get the logging service object.
 * 
 * @return struct bt_gatt_service_static* 
 */
const struct bt_gatt_service_static *sid_ble_get_log_service(void);

#endif /* SID_PAL_BLE_LOG_SERVICE_H */
