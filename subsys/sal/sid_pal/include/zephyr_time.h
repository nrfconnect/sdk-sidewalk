/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef ZEPHYR_TIME_H
#define ZEPHYR_TIME_H

#include <stdint.h>

/**
 * @brief Function to get time from device start form Zephyr RTOS.
 * @return Number of nanoseconds since last reboot. 64 bit variable.
 */
uint64_t zephyr_uptime_ns(void);

#endif /* ZEPHYR_TIME_H */
