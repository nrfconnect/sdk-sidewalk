/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef NRF_BLE_ADVERT_H
#define NRF_BLE_ADVERT_H

#include <stdint.h>

int sid_ble_advert_start(void);
int sid_ble_advert_stop(void);
int sid_ble_advert_update(uint8_t *data, uint8_t data_len);

#endif /* NRF_BLE_ADVERT_H */
