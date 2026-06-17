/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <sid_pal_uptime_ifc.h>
#include <zephyr/sys_clock.h>

extern uint64_t test_zephyr_uptime_ns_value;

static void before_test(void *fixture)
{
	ARG_UNUSED(fixture);
	test_zephyr_uptime_ns_value = 0;
}

ZTEST_SUITE(pal_uptime, NULL, NULL, before_test, NULL, NULL);

static void uptime_test_time(uint64_t uptime_nanoseconds)
{
	uint32_t seconds, nanoseconds;
	struct sid_timespec sid_time;

	seconds = (uint32_t)(uptime_nanoseconds / NSEC_PER_SEC);
	nanoseconds = (uint32_t)(uptime_nanoseconds % NSEC_PER_SEC);

	test_zephyr_uptime_ns_value = uptime_nanoseconds;
	zassert_equal(SID_ERROR_NONE, sid_pal_uptime_now(&sid_time));

	zassert_equal(seconds, sid_time.tv_sec);
	zassert_equal(nanoseconds, sid_time.tv_nsec);
}

ZTEST(pal_uptime, test_sid_pal_uptime_get_now)
{
	zassert_equal(SID_ERROR_NULL_POINTER, sid_pal_uptime_now(NULL));

	uptime_test_time(0ull);
	uptime_test_time(1ull * NSEC_PER_SEC + 10ull);
	uptime_test_time(10ull * NSEC_PER_SEC + 100ull);
	uptime_test_time(LONG_MAX);
	uptime_test_time(LLONG_MAX);
	uptime_test_time(ULLONG_MAX);
}

ZTEST(pal_uptime, test_sid_pal_uptime_accuracy)
{
	int16_t ppm;

	ppm = sid_pal_uptime_get_xtal_ppm();
	sid_pal_uptime_set_xtal_ppm(ppm);
	zassert_equal(ppm, sid_pal_uptime_get_xtal_ppm());
}
