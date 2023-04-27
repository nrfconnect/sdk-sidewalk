/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_pal_temperature_ifc.h>
#include <syscalls/device.h>
#include <unity.h>

char DT_N_NODELABEL_temp_FULL_NAME[50] = "dummy_device_name";

void test_sid_pal_temperature_init()
{
	TEST_IGNORE_MESSAGE("TBD");
}

void test_sid_pal_temperature_get()
{
	TEST_IGNORE_MESSAGE("TBD");
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

int main(void)
{
	(void)unity_main();

	return 0;
}
