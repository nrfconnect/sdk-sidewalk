/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>

uint64_t test_zephyr_uptime_ns_value;

uint64_t zephyr_uptime_ns(void)
{
	return test_zephyr_uptime_ns_value;
}
