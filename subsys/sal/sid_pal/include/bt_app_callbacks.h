/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_PAL_APP_CALLBACKS_H
#define SID_PAL_APP_CALLBACKS_H

#include <stdbool.h>
#include <zephyr/bluetooth/bluetooth.h>

#if defined(CONFIG_BT_APP_IFC)

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
 * This function removes internal reference.
 * If the internal reference counter shows 0, real @bt_disable is called
 * 
 * @return int result from @bt_disable or 0 if sid_ble_bt_enable has been called more than sid_ble_bt_disable
 */
int sid_ble_bt_disable();

/**
 * @brief BT ids used for extended advertising. 
 * This allows to identify connections from different extended adverticements.
 */
enum sid_ble_id_values {
	_BT_ID_DEFAULT = BT_ID_DEFAULT,
	BT_ID_SIDEWALK,
#if defined(CONFIG_SIDEWALK_DFU)
	BT_ID_SMP_DFU,
#endif
	_BT_ID_MAX
};

BUILD_ASSERT(_BT_ID_MAX <= CONFIG_BT_ID_MAX,
	     "Too many BT Ids! increase CONFIG_BT_ID_MAX, to match _BT_ID_MAX");
#endif
#endif
