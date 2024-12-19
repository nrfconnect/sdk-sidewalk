/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>
#include <sid_pal_uptime_ifc.h>

/**
 * @brief Tests the Sidewalk uptime
 *
 * 1. Get kernel uptime, convert it to seconeds and nanoseconds.
 * 2. Get Sidewalk uptime.
 * 3. Get kernel uptime again, convert it to seconeds and nanoseconds.
 * 4. Check if Sidewalk uptime is between the two kernel uptimes.
 */
ZTEST(uptime, test_sid_pal_uptime_get)
{
	struct sid_timespec time_befre, time_after, sid_time = { 0 };
	uint64_t uptime = k_ticks_to_ns_floor64(k_uptime_ticks());
	time_befre.tv_sec = (uint32_t)(uptime / NSEC_PER_SEC);
	time_befre.tv_nsec = (uint32_t)(uptime % NSEC_PER_SEC);

	zassert_equal(SID_ERROR_NONE, sid_pal_uptime_now(&sid_time));

	uptime = k_ticks_to_ns_ceil64(k_uptime_ticks());
	time_after.tv_sec = (uint32_t)(uptime / NSEC_PER_SEC);
	time_after.tv_nsec = (uint32_t)(uptime % NSEC_PER_SEC);

	zassert_true(time_befre.tv_sec <= time_after.tv_sec, "Test data preparation failed.");
	zassert_true((time_befre.tv_nsec <= time_after.tv_nsec) ||
			     (time_befre.tv_sec < time_after.tv_sec),
		     "Test data preparation failed.");

	zassert_true(sid_time.tv_sec >= time_befre.tv_sec, "before: %d, sidewalk: %d",
		     time_befre.tv_sec, sid_time.tv_sec);
	zassert_true((sid_time.tv_nsec >= time_befre.tv_nsec) ||
			     (sid_time.tv_sec > time_befre.tv_sec),
		     "before: %d.%d, sidewalk: %d.%d", time_befre.tv_sec, time_befre.tv_nsec,
		     sid_time.tv_sec, sid_time.tv_nsec);
	zassert_true(sid_time.tv_sec <= time_after.tv_sec, "sidewlk: %d.%d, after: %d.%d",
		     sid_time.tv_sec, time_after.tv_sec);
	zassert_true((sid_time.tv_nsec <= time_after.tv_nsec) ||
			     (sid_time.tv_sec < time_after.tv_sec),
		     "sidewlk: %d.%d, after: %d.%d", sid_time.tv_sec, sid_time.tv_nsec,
		     time_after.tv_sec, time_after.tv_nsec);
}

ZTEST_SUITE(uptime, NULL, NULL, NULL, NULL, NULL);
