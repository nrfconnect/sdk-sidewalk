/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_error.h>
#include <sid_pal_swi_ifc.h>

#define UNCHANGED 0
#define CHANGED 1
static volatile int resource;

static void swi_callback(void)
{
	resource = CHANGED;
}

void test_sid_pal_swi(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_init(swi_callback));

	resource = UNCHANGED;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_trigger());
	TEST_ASSERT_EQUAL(CHANGED, resource);
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
