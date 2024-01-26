/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <sid_pal_swi_ifc.h>

void mock_callback(void)
{
	/* Intentionally left blanc. */
}

static void swi_init_start(void *fixture)
{
	ARG_UNUSED(fixture);
	zassert_equal(sid_pal_swi_init(), SID_ERROR_NONE);
	zassert_equal(sid_pal_swi_start(mock_callback), SID_ERROR_NONE);
}

static void swi_stop_deinit(void *fixture)
{
	ARG_UNUSED(fixture);
	zassert_equal(sid_pal_swi_stop(), SID_ERROR_NONE);
	zassert_equal(sid_pal_swi_deinit(), SID_ERROR_NONE);
}

ZTEST(swi_tests, test_swi_reinit)
{
	zassert_equal(sid_pal_swi_deinit(), SID_ERROR_NONE);
	zassert_equal(sid_pal_swi_deinit(), SID_ERROR_NONE);
	zassert_equal(sid_pal_swi_init(), SID_ERROR_NONE);
	zassert_equal(sid_pal_swi_init(), SID_ERROR_NONE);
}

ZTEST(swi_tests, test_swi_start_with_null)
{
	zassert_equal(sid_pal_swi_start(NULL), SID_ERROR_NULL_POINTER);
}

ZTEST(swi_tests, test_swi_trigger_positive)
{
	zassert_equal(sid_pal_swi_trigger(), SID_ERROR_NONE);
}

ZTEST(swi_tests, test_swi_trigger_negative)
{
	/* stop - fail, restart - ok */
	zassert_equal(sid_pal_swi_stop(), SID_ERROR_NONE);
	zassert_equal(sid_pal_swi_trigger(), SID_ERROR_INVALID_STATE);

	zassert_equal(sid_pal_swi_start(mock_callback), SID_ERROR_NONE);
	zassert_equal(sid_pal_swi_trigger(), SID_ERROR_NONE);

	/* deinit - fail, reinit -ok */
	swi_stop_deinit(NULL);
	zassert_equal(sid_pal_swi_trigger(), SID_ERROR_INVALID_STATE);

	swi_init_start(NULL);
	zassert_equal(sid_pal_swi_trigger(), SID_ERROR_NONE);
}

ZTEST_SUITE(swi_tests, NULL, NULL, swi_init_start, swi_stop_deinit, NULL);
