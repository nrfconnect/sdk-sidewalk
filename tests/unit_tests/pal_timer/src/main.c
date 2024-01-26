/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <sid_pal_timer_ifc.h>
#include <cmock_sid_pal_critical_region_ifc.h>

static sid_pal_timer_t *p_null_timer = NULL;
static sid_pal_timer_t test_timer;
static sid_pal_timer_t test_timer_2;
static int test_timer_arg;

static int timer_callback_cnt = 0;

void setUp(void)
{
	timer_callback_cnt = 0;
	__cmock_sid_pal_enter_critical_region_Ignore();
	__cmock_sid_pal_exit_critical_region_Ignore();
}

/******************************************************************
* sid_pal_timer_ifc
* ****************************************************************/
void timer_cb(void *arg, sid_pal_timer_t *originator)
{
	timer_callback_cnt++;
}

static void timer_init(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_init(&test_timer, timer_cb, &test_timer_arg));
	TEST_ASSERT_EQUAL_PTR(&test_timer_arg, test_timer.callback_arg);
	TEST_ASSERT_EQUAL_PTR(timer_cb, test_timer.callback);
}

static void timer_deinit(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_NULL(test_timer.callback);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
}

void test_sid_pal_timer_init_deinit(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_init(NULL, NULL, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_init(&test_timer, NULL, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_init(p_null_timer, timer_cb, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_deinit(p_null_timer));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_cb, NULL));

	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_EQUAL_PTR(timer_cb, test_timer.callback);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));

	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_NULL(test_timer.callback);

	timer_init();
	timer_deinit();
}

void test_sid_pal_timer_arm(void)
{
	struct sid_timespec when = { .tv_sec = 5 };
	struct sid_timespec period = { .tv_sec = 5 };

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS,
			  sid_pal_timer_arm(p_null_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL,
					    NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS,
			  sid_pal_timer_arm(p_null_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL,
					    NULL));
	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    &period));

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    NULL));

	timer_deinit();
}

void test_sid_pal_timer_is_armed(void)
{
	struct sid_timespec when = { .tv_sec = 5 };

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(p_null_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    NULL));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_LOWPOWER, &when,
					    NULL));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_cancel(void)
{
	struct sid_timespec when = { .tv_sec = 5 };
	struct sid_timespec period = { .tv_sec = 5 };

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_cancel(p_null_timer));

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    &period));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_is_armed_deinit(void)
{
	struct sid_timespec when = { .tv_sec = 5 };

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    NULL));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
}

void test_sid_pal_timer_one_shot(void)
{
	struct sid_timespec when = { .tv_nsec = 50 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec fake_time_low = { .tv_nsec = 40 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec fake_time_expected = when;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					    NULL));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);

	sid_pal_timer_event_callback(NULL, &fake_time_low);

	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));

	sid_pal_timer_event_callback(NULL, &fake_time_expected);

	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_two_timers(void)
{
	struct sid_timespec when_1 = { .tv_nsec = 50 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec when_2 = { .tv_nsec = 80 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec fake_time_low = { .tv_nsec = 40 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec fake_time_expected = when_1;

	timer_init();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_init(&test_timer_2, timer_cb, &test_timer_arg));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer_2));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when_1,
					    NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_timer_arm(&test_timer_2, SID_PAL_TIMER_PRIO_CLASS_PRECISE,
					    &when_2, NULL));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);

	sid_pal_timer_event_callback(NULL, &fake_time_low);

	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer_2));

	sid_pal_timer_event_callback(NULL, &fake_time_expected);

	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer_2));

	fake_time_expected = when_2;
	sid_pal_timer_event_callback(NULL, &fake_time_expected);

	TEST_ASSERT_EQUAL(2, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer_2));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer_2));
	timer_deinit();
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

int main(void)
{
	return unity_main();
}
