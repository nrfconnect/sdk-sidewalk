/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_PAL_APP_CALLBACKS_H
#define SID_PAL_APP_CALLBACKS_H

#include <stdbool.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief check if attr is for SMP service
 * 
 * @param attr 
 * @return true if attr is for SMP
 * @return false in othre cases
 */
bool sid_ble_bt_attr_is_SMP(const struct bt_gatt_attr *attr);

/**
 * @brief check if attr is for one of Sidewlak services
 * 
 * @param attr 
 * @return true if attr is
 * @return false in othre cases
 */
bool sid_ble_bt_attr_is_SIDEWALK(const struct bt_gatt_attr *attr);

/**
 * @brief Wrapper for @bt_enable, with reference tracking.
 * Real @bt_enable is called only of first call to app_bt_enable
 * 
 * @param cb callback passed to @bt_enable
 * @return int result from @bt_enable call or 0 if called multiple times
 */
int sid_ble_bt_enable(bt_ready_cb_t cb);

/**
 * @brief Wrapper for @bt_disable.
 * This function removes an internal reference. 
 * If the internal reference counter reaches 0, the real @bt_disable is called.
 *
 * @return int 0 on successful disable,
 *        Negative error codes as returned by @bt_disable,
 *        Positive number indicating the number of connections left.
 */
int sid_ble_bt_disable();

/**
 * @brief BT ids used for extended advertising. 
 * This allows to identify connections from different extended advertisements.
 */
enum sid_ble_id_values {
	_BT_ID_DEFAULT = BT_ID_DEFAULT,
	BT_ID_SIDEWALK,
#if defined(CONFIG_SIDEWALK_DFU)
	BT_ID_SMP_DFU,
#endif
	_BT_ID_MAX
};

#if defined(CONFIG_BT_ID_MAX)
BUILD_ASSERT(_BT_ID_MAX <= CONFIG_BT_ID_MAX,
	     "Too many BT Ids! increase CONFIG_BT_ID_MAX, to match _BT_ID_MAX");
#endif

#ifdef __cplusplus
}
#endif
#endif
