/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>

#include <sid_pal_timer_ifc.h>
#include <sid_pal_critical_region_ifc.h>

extern int test_enter_critical_call_count;
extern int test_exit_critical_call_count;

static sid_pal_timer_t *p_null_timer = NULL;
static sid_pal_timer_t test_timer;
static sid_pal_timer_t test_timer_2;
static int test_timer_arg;

static int timer_callback_cnt = 0;

static void before_test(void *fixture)
{
	ARG_UNUSED(fixture);

	timer_callback_cnt = 0;
	test_enter_critical_call_count = 0;
	test_exit_critical_call_count = 0;
}

ZTEST_SUITE(pal_timer, NULL, NULL, before_test, NULL, NULL);

static void timer_cb(void *arg, sid_pal_timer_t *originator)
{
	ARG_UNUSED(arg);
	ARG_UNUSED(originator);

	timer_callback_cnt++;
}

static void timer_init(void)
{
	zassert_equal(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_cb, &test_timer_arg));
	zassert_equal_ptr(&test_timer_arg, test_timer.callback_arg);
	zassert_equal_ptr(timer_cb, test_timer.callback);
}

static void timer_deinit(void)
{
	zassert_equal(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));
	zassert_is_null(test_timer.callback_arg);
	zassert_is_null(test_timer.callback);
	zassert_false(sid_pal_timer_is_armed(&test_timer));
}

ZTEST(pal_timer, test_sid_pal_timer_init_deinit)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_timer_init(NULL, NULL, NULL));
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_timer_init(&test_timer, NULL, NULL));
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_timer_init(p_null_timer, timer_cb, NULL));
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_timer_deinit(p_null_timer));

	zassert_equal(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_cb, NULL));

	zassert_is_null(test_timer.callback_arg);
	zassert_equal_ptr(timer_cb, test_timer.callback);

	zassert_equal(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));

	zassert_is_null(test_timer.callback_arg);
	zassert_is_null(test_timer.callback);

	timer_init();
	timer_deinit();
}

ZTEST(pal_timer, test_sid_pal_timer_arm)
{
	struct sid_timespec when = { .tv_sec = 5 };
	struct sid_timespec period = { .tv_sec = 5 };

	zassert_equal(SID_ERROR_INVALID_ARGS,
		      sid_pal_timer_arm(p_null_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL,
					NULL));
	zassert_equal(SID_ERROR_INVALID_ARGS,
		      sid_pal_timer_arm(p_null_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					NULL));
	zassert_equal(SID_ERROR_INVALID_ARGS,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL, NULL));
	timer_init();

	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					NULL));

	zassert_equal(SID_ERROR_INVALID_ARGS,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					NULL));

	zassert_equal(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					&period));

	zassert_equal(SID_ERROR_INVALID_ARGS,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					NULL));

	timer_deinit();
}

ZTEST(pal_timer, test_sid_pal_timer_is_armed)
{
	struct sid_timespec when = { .tv_sec = 5 };

	zassert_false(sid_pal_timer_is_armed(p_null_timer));
	zassert_false(sid_pal_timer_is_armed(&test_timer));

	timer_init();

	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					NULL));
	zassert_true(sid_pal_timer_is_armed(&test_timer));
	zassert_equal(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	zassert_false(sid_pal_timer_is_armed(&test_timer));

	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_LOWPOWER, &when,
					NULL));
	zassert_true(sid_pal_timer_is_armed(&test_timer));
	zassert_equal(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	zassert_false(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

ZTEST(pal_timer, test_sid_pal_timer_cancel)
{
	struct sid_timespec when = { .tv_sec = 5 };
	struct sid_timespec period = { .tv_sec = 5 };

	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_timer_cancel(p_null_timer));

	timer_init();

	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					&period));
	zassert_equal(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));

	timer_deinit();
}

ZTEST(pal_timer, test_sid_pal_timer_is_armed_deinit)
{
	struct sid_timespec when = { .tv_sec = 5 };

	timer_init();

	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					NULL));
	zassert_true(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();

	zassert_false(sid_pal_timer_is_armed(&test_timer));
}

ZTEST(pal_timer, test_sid_pal_timer_one_shot)
{
	struct sid_timespec when = { .tv_nsec = 50 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec fake_time_low = { .tv_nsec = 40 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec fake_time_expected = when;

	timer_init();

	zassert_false(sid_pal_timer_is_armed(&test_timer));
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when,
					NULL));
	zassert_equal(0, timer_callback_cnt);

	sid_pal_timer_event_callback(NULL, &fake_time_low);

	zassert_equal(0, timer_callback_cnt);
	zassert_true(sid_pal_timer_is_armed(&test_timer));

	sid_pal_timer_event_callback(NULL, &fake_time_expected);

	zassert_equal(1, timer_callback_cnt);
	zassert_false(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

ZTEST(pal_timer, test_sid_pal_timer_two_timers)
{
	struct sid_timespec when_1 = { .tv_nsec = 50 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec when_2 = { .tv_nsec = 80 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec fake_time_low = { .tv_nsec = 40 * SID_TIME_NSEC_PER_USEC, .tv_sec = 0 };
	struct sid_timespec fake_time_expected = when_1;

	timer_init();
	zassert_equal(SID_ERROR_NONE, sid_pal_timer_init(&test_timer_2, timer_cb, &test_timer_arg));
	zassert_false(sid_pal_timer_is_armed(&test_timer));
	zassert_false(sid_pal_timer_is_armed(&test_timer_2));
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when_1,
					NULL));
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_timer_arm(&test_timer_2, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when_2,
					NULL));
	zassert_equal(0, timer_callback_cnt);

	sid_pal_timer_event_callback(NULL, &fake_time_low);

	zassert_equal(0, timer_callback_cnt);
	zassert_true(sid_pal_timer_is_armed(&test_timer));
	zassert_true(sid_pal_timer_is_armed(&test_timer_2));

	sid_pal_timer_event_callback(NULL, &fake_time_expected);

	zassert_equal(1, timer_callback_cnt);
	zassert_false(sid_pal_timer_is_armed(&test_timer));
	zassert_true(sid_pal_timer_is_armed(&test_timer_2));

	fake_time_expected = when_2;
	sid_pal_timer_event_callback(NULL, &fake_time_expected);

	zassert_equal(2, timer_callback_cnt);
	zassert_false(sid_pal_timer_is_armed(&test_timer_2));
	zassert_equal(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer_2));
	timer_deinit();
}
