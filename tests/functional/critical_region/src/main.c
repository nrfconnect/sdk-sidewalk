/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_critical_region_ifc.h>
#include <sid_pal_timer_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_error.h>
#include <sid_time_ops.h>

#define UNCHANGED 0
#define CHANGED 1
static volatile int resource;

static sid_pal_timer_t test_timer;
static int test_timer_arg;

static void timer_callback(void *arg, sid_pal_timer_t *originator)
{
	resource = CHANGED;
}

static void relative_time_calculate(struct sid_timespec *when)
{
	struct sid_timespec now;

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_uptime_now(&now));
	sid_time_add(&now, when);
	*when = now;
}

void start_timer_and_wait(void)
{
	struct sid_timespec timeout = { .tv_nsec = 100 };
	struct sid_timespec now = { 0 };
	sid_error_t erc = 0;
	static bool timer_initialized = false;

	if (!timer_initialized) {
		erc = sid_pal_timer_init(&test_timer, timer_callback, &test_timer_arg);
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, erc);
		timer_initialized = true;
	}

	relative_time_calculate(&timeout);
	erc = sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &timeout, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, erc);

	do {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_uptime_now(&now));
	} while (sid_time_gt(&timeout, &now));
}

void test_sid_pal_critical_region_with_timer(void)
{
	resource = UNCHANGED;
	start_timer_and_wait();
	TEST_ASSERT_EQUAL_UINT32(CHANGED, resource);

	sid_pal_enter_critical_region();
	resource = UNCHANGED;
	start_timer_and_wait();
	TEST_ASSERT_EQUAL_UINT32(UNCHANGED, resource);
	sid_pal_exit_critical_region();

	start_timer_and_wait();
	TEST_ASSERT_EQUAL_UINT32(CHANGED, resource);
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
