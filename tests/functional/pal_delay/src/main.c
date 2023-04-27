/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <zephyr/kernel.h>
#include <stdlib.h>
#include <sid_pal_delay_ifc.h>

#define MIN_DELAY_US       (0ULL)
#define DELAY_US           (1200ULL)

#define MAX_DELAY_US_THRESHOLD (120ULL)

#define TIMESTAMP_US_GET() (k_ticks_to_us_floor64((uint64_t)k_uptime_ticks()))

void setUp(void)
{
}

void tearDown(void)
{
}

void test_sid_pal_delay(void)
{
	uint64_t timestamp = TIMESTAMP_US_GET();

	sid_pal_delay_us(MIN_DELAY_US);
	uint64_t delta = TIMESTAMP_US_GET() -  timestamp;

	TEST_ASSERT_LESS_OR_EQUAL(MAX_DELAY_US_THRESHOLD, abs((int)(MIN_DELAY_US - delta)));

	timestamp = TIMESTAMP_US_GET();
	sid_pal_delay_us(DELAY_US);
	delta = TIMESTAMP_US_GET() -  timestamp;
	TEST_ASSERT_LESS_OR_EQUAL(MAX_DELAY_US_THRESHOLD, abs((int)(DELAY_US - delta)));
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

int main(void)
{
	(void)unity_main();

	return 0;
}
