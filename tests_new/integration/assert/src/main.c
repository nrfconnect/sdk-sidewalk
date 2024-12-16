/**
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * 
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>
#include <sid_pal_assert_ifc.h>
#include <zephyr/ztest_error_hook.h>

void before(void *fixture)
{
	// Set assert behaviour to normal
	ztest_set_assert_valid(false);
}

ZTEST(pal_assert, test_sid_pal_assert_true)
{
	SID_PAL_ASSERT(true);
}
ZTEST(pal_assert, test_sid_pal_assert_false)
{
	// Expect asserts to fail
	ztest_set_assert_valid(true);
	SID_PAL_ASSERT(false);
}

ZTEST(pal_assert, test_sid_pal_assert_false_raw)
{
	// Expect asserts to fail
	ztest_set_assert_valid(true);
	sid_pal_assert(__LINE__, __FILE__);
}

ZTEST_SUITE(pal_assert, NULL, NULL, before, NULL, NULL);
