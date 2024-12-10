/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <zephyr/kernel.h>
#include <stdlib.h>
#include <sid_pal_delay_ifc.h>

#define MIN_DELAY_US (0ULL)
#define DELAY_US (1200ULL)

#define MAX_DELAY_US_THRESHOLD (120ULL)

#define TIMESTAMP_US_GET() (k_ticks_to_us_floor64((uint64_t)k_uptime_ticks()))

ZTEST(pal_delay, test_sid_pal_delay)
{
	uint64_t timestamp = TIMESTAMP_US_GET();

	sid_pal_delay_us(MIN_DELAY_US);
	uint64_t delta = TIMESTAMP_US_GET() - timestamp;

	zassert_true(abs((int)(MIN_DELAY_US - delta)) <= MAX_DELAY_US_THRESHOLD,
		     "expected delay %d took %d", MIN_DELAY_US, delta);
	timestamp = TIMESTAMP_US_GET();
	sid_pal_delay_us(DELAY_US);
	delta = TIMESTAMP_US_GET() - timestamp;
	zassert_true(abs((int)(DELAY_US - delta)) <= MAX_DELAY_US_THRESHOLD,
		     "expected delay %d took %d", DELAY_US, delta);
}

ZTEST(pal_delay, test_sanity)
{
	zassert_equal(true, true);
}

ZTEST_SUITE(pal_delay, NULL, NULL, NULL, NULL, NULL);
