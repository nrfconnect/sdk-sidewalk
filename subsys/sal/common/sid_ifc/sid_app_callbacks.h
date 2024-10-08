/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_PAL_APP_CALLBACKS_H
#define SID_PAL_APP_CALLBACKS_H

#include <stdbool.h>

/**
 * This callback is called when BT is about to be disabled. It lets the
 * callee decide whether the bt_disable() function should be called (as it
 * may impact other modules).
 *
 * @return true if the Bluetooth should be disabled, false otherwise.
 */
typedef bool (* sid_bt_disable_pred_callback_t)(void);

/**
 * This callback is called before loading the settings using the settings_load()
 * function. It lets the callee decide wheter the settings should be loaded as
 * it may have been already done at the moment if initialization.
 *
 * @return true if settings should be loaded, false otherwise.
 */
typedef bool (* sid_settings_load_pred_callback_t)(void);

/**
 * Set @ref sid_bt_disable_pred_callback_t.
 *
 * @param[in] cb  The callback.
 */
void sid_disable_pred_cb_set(sid_bt_disable_pred_callback_t cb);

/**
 * Set @ref sid_settings_load_pred_callback_t.
 *
 * @param[in] cb  The callback.
 */
void sid_settings_load_pred_cb_set(sid_settings_load_pred_callback_t cb);

#endif
