/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_vnd_service.h
 *  @brief Vendor service.
 */

#ifndef SID_PAL_BLE_VND_SERVICE_H
#define SID_PAL_BLE_VND_SERVICE_H

#include <sid_ble_uuid.h>

/* VENDOR_SERVICE */
#define VND_SID_BT_UUID_SERVICE             BT_UUID_DECLARE_16(VND_EXAMPLE_SERVICE_UUID_VAL)
#define VND_SID_BT_CHARACTERISTIC_WRITE     BT_UUID_DECLARE_128(VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE)
#define VND_SID_BT_CHARACTERISTIC_NOTIFY    BT_UUID_DECLARE_128(VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY)

/**
 * @brief Get the vendor service object.
 * 
 * @return struct bt_gatt_service_static* 
 */
const struct bt_gatt_service_static *sid_ble_get_vnd_service(void);

#endif /* SID_PAL_BLE_VND_SERVICE_H */
