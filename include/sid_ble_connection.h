/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef NRF_BLE_CONNECTION_H
#define NRF_BLE_CONNECTION_H

#include <sid_pal_ble_adapter_ifc.h>
#include <bluetooth/conn.h>

/**
 * @brief Struct with bluetooth connection paramters.
 */
typedef struct {
	struct bt_conn *conn;
	uint8_t addr[BT_ADDR_SIZE];
} sid_ble_conn_params_t;

/**
 * @brief Initialize ble connection module.
 */
void sid_ble_conn_init(void);

/**
 * @brief Deinitialize ble connection module.
 */
void sid_ble_conn_deinit(void);

/**
 * @brief The function returns current connection paramters.
 *
 * @return connection paramters as defined in @ref sid_ble_conn_params_t.
 */
const sid_ble_conn_params_t *sid_ble_conn_params_get(void);


/**
 * @brief Set the custom connection callback.
 *
 * @note Callback will be called on connect and disconnect event.
 * 
 * @param cb callback function pointer.
 */
void sid_ble_conn_cb_set(sid_pal_ble_connection_callback_t cb);

#endif /* NRF_BLE_CONNECTION_H */
