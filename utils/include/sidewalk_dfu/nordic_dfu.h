/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdbool.h>

#ifndef NORDIC_DFU_H
#define NORDIC_DFU_H

/** @brief Start nRF Connect SDK DFU service mode.
 *
 *  @return 0 in case of success, negative error code otherwise.
 */
int nordic_dfu_ble_start(void);

/** @brief Stops nRF Connect SDK DFU service mode.
 *
 *  @return 0 in case of success, negative error code otherwise.
 */
int nordic_dfu_ble_stop(void);

/**
 * @brief check if application is in dfu mode
 * 
 * @return true if in DFU mode
 * @return false 
 */
bool nordic_dfu_is_in_dfu();

#endif /* NORDIC_DFU_H */
