/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BLE_CALLBACKS_MOCK_H
#define BLE_CALLBACKS_MOCK_H

#include <zephyr/fff.h>

#include <sid_ble_adapter_callbacks.h>

FAKE_VALUE_FUNC(sid_error_t, sid_ble_adapter_notification_cb_set,
		sid_pal_ble_indication_callback_t);
FAKE_VALUE_FUNC(sid_error_t, sid_ble_adapter_data_cb_set, sid_pal_ble_data_callback_t);
FAKE_VALUE_FUNC(sid_error_t, sid_ble_adapter_notification_changed_cb_set,
		sid_pal_ble_notify_callback_t);
FAKE_VALUE_FUNC(sid_error_t, sid_ble_adapter_conn_cb_set, sid_pal_ble_connection_callback_t);
FAKE_VALUE_FUNC(sid_error_t, sid_ble_adapter_mtu_cb_set, sid_pal_ble_mtu_callback_t);
FAKE_VALUE_FUNC(sid_error_t, sid_ble_adapter_adv_start_cb_set, sid_pal_ble_adv_start_callback_t);
FAKE_VOID_FUNC(sid_ble_adapter_conn_connected, const uint8_t *);
FAKE_VOID_FUNC(sid_ble_adapter_conn_disconnected, const uint8_t *);
FAKE_VOID_FUNC(sid_ble_adapter_mtu_changed, uint16_t);

#define FFF_FAKES_LIST_BLE_CALLBACKS(FAKE)                                                         \
	FAKE(sid_ble_adapter_notification_cb_set)                                                  \
	FAKE(sid_ble_adapter_data_cb_set)                                                          \
	FAKE(sid_ble_adapter_notification_changed_cb_set)                                          \
	FAKE(sid_ble_adapter_conn_cb_set)                                                          \
	FAKE(sid_ble_adapter_mtu_cb_set)                                                           \
	FAKE(sid_ble_adapter_adv_start_cb_set)                                                     \
	FAKE(sid_ble_adapter_conn_connected)                                                       \
	FAKE(sid_ble_adapter_conn_disconnected)                                                    \
	FAKE(sid_ble_adapter_mtu_changed)

#endif /* BLE_CALLBACKS_MOCK_H */
