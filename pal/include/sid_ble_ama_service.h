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
extern struct bt_uuid_16 AMA_SID_BT_UUID_SERVICE_val;
extern struct bt_uuid_128 AMA_SID_BT_CHARACTERISTIC_WRITE_val;
extern struct bt_uuid_128 AMA_SID_BT_CHARACTERISTIC_NOTIFY_val;

#define AMA_SID_BT_UUID_SERVICE             (struct bt_uuid *)&AMA_SID_BT_UUID_SERVICE_val
#define AMA_SID_BT_CHARACTERISTIC_WRITE     (struct bt_uuid *)&AMA_SID_BT_CHARACTERISTIC_WRITE_val
#define AMA_SID_BT_CHARACTERISTIC_NOTIFY    (struct bt_uuid *)&AMA_SID_BT_CHARACTERISTIC_NOTIFY_val

/**
 * @brief Get the Amazon service object.
 *
 * @return struct bt_gatt_service_static*
 */
const struct bt_gatt_service_static *sid_ble_get_ama_service(void);

#endif /* SID_PAL_BLE_AMA_SERVICE_H */
