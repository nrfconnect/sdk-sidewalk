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

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

typedef struct {
	struct bt_conn *conn;
	struct bt_uuid *uuid;
	struct bt_gatt_service_static *service;
} sid_ble_srv_params_t;

/**
 * @brief Send data over BLE.
 *
 * @param params service parameters.
 * @param data buffer with data.
 * @param length data buffer length.
 * @return 0 in case of success, negative value otherwise.
 */
int sid_ble_send_data(sid_ble_srv_params_t *params,
		      uint8_t *data, uint16_t length);

#endif /* SID_PAL_BLE_SERVICE_H */
