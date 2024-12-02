/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_pal_assert_ifc.h>
#include <stdbool.h>
#include <zephyr/ztest.h>
#include <zephyr/fff.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC_VARARG(z_log_minimal_printk, const char *, ...);

static void suite_setup(void *f)
{
	RESET_FAKE(z_log_minimal_printk);
}

/******************************************************************
* sid_pal_assert_ifc
* ****************************************************************/
static bool should_assert;
void assert_post_action(const char *file, unsigned int line)
{
	ARG_UNUSED(file);
	ARG_UNUSED(line);

	if (should_assert) {
		should_assert = false;
		ztest_test_pass();
	}
	ztest_test_fail();
}

ZTEST(assert_tests, test_sid_pal_assert_true)
{
    should_assert = false;
    SID_PAL_ASSERT(true);
    zassert_false(should_assert, "No assert when it should be.");
}

ZTEST(assert_tests, test_sid_pal_assert_false)
{
    should_assert = true;
    SID_PAL_ASSERT(false);
    zassert_false(should_assert, "Asserted, but should not.");
}

ZTEST_SUITE(assert_tests, NULL, NULL, suite_setup, NULL, NULL);
