/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <sid_pal_log_ifc.h>

static void before_test(void *fixture)
{
	ARG_UNUSED(fixture);
}

ZTEST_SUITE(pal_log, NULL, NULL, before_test, NULL, NULL);

ZTEST(pal_log, test_log_severity)
{
	sid_pal_log(SID_PAL_LOG_SEVERITY_ERROR, 1, "Sidewalk log Error");
	sid_pal_log(SID_PAL_LOG_SEVERITY_WARNING, 1, "Sidewalk log Warning");
	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 1, "Sidewalk log Info");
	sid_pal_log(SID_PAL_LOG_SEVERITY_DEBUG, 1, "Sidewalk log Debug");
	sid_pal_log(SID_PAL_LOG_SEVERITY_DEBUG + 1, 1, "Sidewalk log Unknow");
}

ZTEST(pal_log, test_log_arguments)
{
	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 1, "%s", "Text");
	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 1, "%d", 1);
	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 5, "%d, %d, %d, %d, %d", 1, 2, 3, 4, 5);

	sid_pal_log(
		SID_PAL_LOG_SEVERITY_INFO, 1,
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
}

ZTEST(pal_log, test_log_flush)
{
	sid_pal_log_flush();
}

ZTEST(pal_log, test_log_push_string)
{
	char test_string[] = "test message 123";

	const char *ret_string = sid_pal_log_push_str(test_string);

	zassert_str_equal(test_string, ret_string);
}

ZTEST(pal_log, test_log_get_buffer)
{
	bool ret;
	struct sid_pal_log_buffer *const test_log_buffer = NULL;

	ret = sid_pal_log_get_log_buffer(test_log_buffer);
	zassert_false(ret,
		      "sid_pal_log_get_log_buffer not implemented, should always return false.");
}
