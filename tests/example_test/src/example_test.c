/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>


void setUp(void)
{
}

void test_pass(void)
{
	TEST_PASS();
}

void test_not_implemented(void)
{
	TEST_IGNORE_MESSAGE("Not implemented!");
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
