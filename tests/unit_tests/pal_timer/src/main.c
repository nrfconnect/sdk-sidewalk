/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_timer_ifc.h>

#define SID_PAL_TIMER_PRIO_CLASS_TOO_BIG        (9)
#define SID_PAL_TIMER_PRIO_CLASS_TOO_SMALL      (-9)

static sid_pal_timer_t *p_null_timer = NULL;
static sid_pal_timer_t test_timer;
static int test_timer_arg;

void setUp(void)
{
}

/******************************************************************
* sid_pal_timer_ifc
* ****************************************************************/
void timer_cb(void *arg, sid_pal_timer_t *originator)
{

}

static void timer_init(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_cb, &test_timer_arg));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(&test_timer_arg, test_timer.callback_arg);
	TEST_ASSERT_EQUAL(timer_cb, test_timer.callback);
}

static void timer_deinit(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));
	TEST_ASSERT_FALSE(test_timer.is_initialized);
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_NULL(test_timer.callback);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
}

void test_sid_pal_timer_init_deinit(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_init(NULL, NULL, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_init(&test_timer, NULL, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_init(p_null_timer, timer_cb, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_deinit(p_null_timer));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_cb, NULL));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(timer_cb, test_timer.callback);
	TEST_ASSERT_EQUAL(SID_ERROR_ALREADY_INITIALIZED, sid_pal_timer_init(&test_timer, timer_cb, NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));
	TEST_ASSERT_FALSE(test_timer.is_initialized);
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_NULL(test_timer.callback);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);

	timer_init();
	timer_deinit();
}

void test_sid_pal_timer_arm(void)
{
	struct sid_timespec when = { .tv_sec = 5 };
	struct sid_timespec period = { .tv_sec = 5 };

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_arm(p_null_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_arm(p_null_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL, NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_TOO_BIG, &when, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_TOO_SMALL, &when, NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_NOT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_TRUE(test_timer.is_periodic);
	TEST_ASSERT_NOT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));

	timer_deinit();
}

void test_sid_pal_timer_is_armed(void)
{
	struct sid_timespec when = { .tv_sec = 5 };

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(p_null_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_cancel(void)
{
	struct sid_timespec when = { .tv_sec = 5 };
	struct sid_timespec period = { .tv_sec = 5 };

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_cancel(p_null_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_timer_cancel(&test_timer));

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_TRUE(test_timer.is_periodic);
	TEST_ASSERT_NOT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);

	timer_deinit();
}

void test_sid_pal_timer_is_armed_deinit(void)
{
	struct sid_timespec when = { .tv_sec = 5 };

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
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
