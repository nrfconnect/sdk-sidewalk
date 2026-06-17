/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BLE_ADAPTER_WRAP_H
#define BLE_ADAPTER_WRAP_H

#include <stdint.h>
#include <sid_ble_connection.h>
#include <sid_ble_advert.h>
#include <sid_ble_service.h>

extern int wrap_settings_load_call_count;
extern int wrap_sid_ble_conn_init_call_count;
extern int wrap_sid_ble_advert_init_call_count;
extern int wrap_sid_ble_send_data_call_count;

int __wrap_settings_load(void);
void __wrap_sid_ble_conn_init(void);
void __wrap_sid_ble_conn_deinit(void);
const sid_ble_conn_data_t *__wrap_sid_ble_conn_data_get(void);
int __wrap_sid_ble_conn_disconnect(void);
int __wrap_sid_ble_advert_init(void);
int __wrap_sid_ble_advert_deinit(void);
int __wrap_sid_ble_advert_update(uint8_t *data, uint8_t len);
int __wrap_sid_ble_advert_start(void);
int __wrap_sid_ble_advert_stop(void);
int __wrap_sid_ble_send_data(sid_ble_srv_params_t *params, uint8_t *data, uint16_t len);

void ble_adapter_wrap_reset(void);

#endif /* BLE_ADAPTER_WRAP_H */
