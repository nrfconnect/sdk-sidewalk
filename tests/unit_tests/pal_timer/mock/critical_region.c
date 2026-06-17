/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>

int test_enter_critical_call_count;
int test_exit_critical_call_count;

void __wrap_sid_pal_enter_critical_region(void)
{
	test_enter_critical_call_count++;
}

void __wrap_sid_pal_exit_critical_region(void)
{
	test_exit_critical_call_count++;
}
