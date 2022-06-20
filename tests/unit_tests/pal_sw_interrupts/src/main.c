/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>

#include <sid_pal_swi_ifc.h>

void test_cb(void)
{
}

void test_sid_pal_swi_init(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_swi_init(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_init(test_cb));
}

void test_sid_pal_swi_trigger(void)
{
	TEST_ASSERT_EQUAL_MESSAGE(SID_ERROR_NOSUPPORT, sid_pal_swi_trigger(), "SWI not supported on native posix");
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
