/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_uptime_ifc.h>
#include <zephyr/sys_clock.h>
#include <cmock_zephyr_time.h>

void setUp(void)
{
}

/******************************************************************
* sid_pal_uptime_ifc
* ****************************************************************/
static void uptime_test_time(uint64_t uptime_nanoseconds)
{
	uint32_t seconds, nanoseconds;
	struct sid_timespec sid_time;

	seconds = (uint32_t)(uptime_nanoseconds / NSEC_PER_SEC);
	nanoseconds = (uint32_t)(uptime_nanoseconds % NSEC_PER_SEC);

	__cmock_zephyr_uptime_ns_ExpectAndReturn(uptime_nanoseconds);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_uptime_now(&sid_time));

	TEST_ASSERT_EQUAL(seconds, sid_time.tv_sec);
	TEST_ASSERT_EQUAL(nanoseconds, sid_time.tv_nsec);
}

void test_sid_pal_uptime_get_now(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_uptime_now(NULL));

	uptime_test_time(0ull);
	uptime_test_time(1ull * NSEC_PER_SEC + 10ull);
	uptime_test_time(10ull * NSEC_PER_SEC + 100ull);
	uptime_test_time(LONG_MAX);
	uptime_test_time(LLONG_MAX);
	uptime_test_time(ULLONG_MAX);
}

void test_sid_pal_uptime_accuracy(void)
{
	/* Note: This is a oversimplified test for dummy implementation.*/
	int16_t ppm;

	ppm = sid_pal_uptime_get_xtal_ppm();
	sid_pal_uptime_set_xtal_ppm(ppm);
	TEST_ASSERT_EQUAL(ppm, sid_pal_uptime_get_xtal_ppm());
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
