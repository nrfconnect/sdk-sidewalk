/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "zephyr_time.h"
#include <zephyr/kernel.h>

uint64_t zephyr_uptime_ns(void)
{
	uint64_t uptime_ticks;

	uptime_ticks = (uint64_t)k_uptime_ticks();
	return k_ticks_to_ns_floor64(uptime_ticks);
}
