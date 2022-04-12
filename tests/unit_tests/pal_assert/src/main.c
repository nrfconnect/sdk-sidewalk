/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <unity.h>
#include <sid_pal_assert_ifc.h>
#include <stdbool.h>
#include <toolchain.h>

void setUp(void)
{
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
		TEST_PASS_MESSAGE("Asserted.");
	}
	TEST_FAIL_MESSAGE("Asserted, but should not.");
}

void test_sid_pal_assert(void)
{
	should_assert = false;
	SID_PAL_ASSERT(true);

	should_assert = true;
	SID_PAL_ASSERT(false);
	TEST_ASSERT_FALSE_MESSAGE(should_assert, "No assert when it should be.");
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
