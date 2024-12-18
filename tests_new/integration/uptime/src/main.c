/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>
#include <sid_pal_uptime_ifc.h>

ZTEST(uptime, test_sid_pal_uptime_get)
{
	struct sid_timespec sid_time = { 0 };
	int64_t uptime_msec = k_uptime_get();
	uint32_t uptime_sec = (uint32_t)(uptime_msec / MSEC_PER_SEC);
	uint32_t uptime_nsec = (uint32_t)((uptime_msec * NSEC_PER_MSEC) % NSEC_PER_SEC);

	zassert_not_equal(0, uptime_msec, "Test data preparation failed.");

	zassert_equal(SID_ERROR_NONE, sid_pal_uptime_now(&sid_time));

	zassert_true(sid_time.tv_sec >= uptime_sec);
	zassert_true(sid_time.tv_nsec >= uptime_nsec);
}

ZTEST_SUITE(uptime, NULL, NULL, NULL, NULL, NULL);
