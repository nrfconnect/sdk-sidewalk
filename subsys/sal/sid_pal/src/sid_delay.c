/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_delay.c
 *  @brief Sidewalk delay implementation.
 */

#include <sid_pal_delay_ifc.h>
#include <zephyr/kernel.h>

void sid_pal_delay_us(uint32_t delay)
{
	(void)k_busy_wait(delay);
}

void sid_pal_scheduler_delay_ms(uint32_t delay)
{
	(void)k_msleep(delay);
}
