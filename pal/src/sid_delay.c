/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_delay.c
 *  @brief Sidewalk delay implementation.
 */

#include <sid_pal_delay_ifc.h>
#include <zephyr.h>

void sid_pal_delay_us(uint32_t delay)
{
#ifdef CONFIG_MULTITHREADING
	(void)k_usleep(delay);
#else
	(void)k_busy_wait(delay);
#endif
}
