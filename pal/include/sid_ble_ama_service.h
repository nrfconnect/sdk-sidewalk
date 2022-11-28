/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_ama_service.h
 *  @brief Amazon service.
 */

#ifndef SID_PAL_BLE_AMA_SERVICE_H
#define SID_PAL_BLE_AMA_SERVICE_H

#include <sid_ble_uuid.h>

/* AMA_SERVICE */
#define AMA_SID_BT_UUID_SERVICE             BT_UUID_DECLARE_16(AMA_SERVICE_UUID_VAL)
#define AMA_SID_BT_CHARACTERISTIC_WRITE     BT_UUID_DECLARE_128(AMA_CHARACTERISTIC_UUID_VAL_WRITE)
#define AMA_SID_BT_CHARACTERISTIC_NOTIFY    BT_UUID_DECLARE_128(AMA_CHARACTERISTIC_UUID_VAL_NOTIFY)

/**
 * @brief Get the Amazon service object.
 *
 * @return struct bt_gatt_service_static*
 */
const struct bt_gatt_service_static *sid_ble_get_ama_service(void);

#endif /* SID_PAL_BLE_AMA_SERVICE_H */
