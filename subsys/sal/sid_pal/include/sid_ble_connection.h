/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NRF_BLE_CONNECTION_H
#define NRF_BLE_CONNECTION_H

#include <zephyr/bluetooth/conn.h>

/**
 * @brief Struct with bluetooth connection paramters.
 */
typedef struct {
	struct bt_conn *conn;
	uint8_t addr[BT_ADDR_SIZE];
} sid_ble_conn_data_t;

/**
 * @brief Initialize ble connection module.
 */
void sid_ble_conn_init(void);

/**
 * @brief Disconnect from a remote device or cancel pending connection.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_conn_disconnect(void);

/**
 * @brief Deinitialize ble connection module.
 */
void sid_ble_conn_deinit(void);

/**
 * @brief The function returns current connection paramters.
 *
 * @return connection data as defined in @ref sid_ble_conn_data_t.
 */
const sid_ble_conn_data_t *sid_ble_conn_data_get(void);

/**
 * @brief Request LE connection parameter update.
 *
 * @param param Requested connection parameters.
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_conn_param_get(struct bt_le_conn_param *param);

/**
 * @brief Request LE connection parameter update.
 *
 * @param param Requested connection parameters.
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_conn_param_update(const struct bt_le_conn_param *param);

#endif /* NRF_BLE_CONNECTION_H */
