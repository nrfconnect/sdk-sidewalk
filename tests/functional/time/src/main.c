/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_timer_ifc.h>
#include <sid_pal_uptime_ifc.h>

/* number of nanoseconds per microseconds */
#define NSEC_PER_MSEC ((NSEC_PER_USEC) *(USEC_PER_MSEC))

#define SID_PAL_TIMER_PRIO_CLASS_TOO_BIG        (9)
#define SID_PAL_TIMER_PRIO_CLASS_TOO_SMALL      (-9)


static sid_pal_timer_t *p_null_timer = NULL;
static sid_pal_timer_t test_timer;
static int test_timer_arg;
static int callback_arg;
static int timer_callback_cnt;

/******************************************************************
* sid_pal_timer_ifc
* ****************************************************************/
static void timer_callback(void *arg, sid_pal_timer_t *originator)
{
	callback_arg = *((int *)arg);
	++timer_callback_cnt;
}

static void timer_init(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_callback, &test_timer_arg));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(&test_timer_arg, test_timer.callback_arg);
	TEST_ASSERT_EQUAL(timer_callback, test_timer.callback);
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
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_init(p_null_timer, timer_callback, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_deinit(p_null_timer));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_callback, NULL));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(timer_callback, test_timer.callback);
	TEST_ASSERT_EQUAL(SID_ERROR_ALREADY_INITIALIZED, sid_pal_timer_init(&test_timer, timer_callback, NULL));

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
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

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

void test_sid_pal_timer_one_shot_50usec(void)
{
	/**
	 * Scenario 1:
	 * 	In this case, we expect that callback will be executed just once after 50 usec.
	 */
	struct sid_timespec when = { .tv_nsec = 50 * NSEC_PER_USEC };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	// It should be enough time
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_periodically_execute_callback(void)
{
	/**
	 * Scenario 2:
	 * 	In this case, we expect that callback will be executed many times and still working.
	 */
	struct sid_timespec when = { .tv_nsec = 50 * NSEC_PER_USEC };
	struct sid_timespec period = { .tv_nsec = 500 };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	// It should be enough time
	k_sleep(K_MSEC(100));
	TEST_ASSERT_TRUE(10 < timer_callback_cnt);
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_cancel_before_it_expire(void)
{
	/**
	 * Scenario 3:
	 * 	Cancel timer before it expires.
	 */
	struct sid_timespec when = { .tv_nsec = (150 * NSEC_PER_MSEC) };
	struct sid_timespec period = { .tv_nsec = 500 };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	k_sleep(K_MSEC(100));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	// Arm the timer again, but now with short period
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	k_sleep(K_MSEC(100));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_cancel_after_it_expired(void)
{
	/**
	 * Scenario 4:
	 * 	Cancel timer after first expired.
	 */
	struct sid_timespec when = { .tv_nsec = (10 * NSEC_PER_MSEC) };
	struct sid_timespec period = { .tv_nsec = (40 * NSEC_PER_MSEC)  };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	k_sleep(K_MSEC(15));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_deinit_after_it_expired(void)
{
	/**
	 * Scenario 5:
	 * 	Deinit timer after first expired.
	 */
	struct sid_timespec when = { .tv_nsec = (10 * NSEC_PER_MSEC) };
	struct sid_timespec period = { .tv_nsec = (40 * NSEC_PER_MSEC)  };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	k_sleep(K_MSEC(15));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
}

void test_sid_pal_timer_one_shot_few_times(void)
{
	/**
	 * Scenario 6:
	 * 	One-shot timer executed few times with argument;
	 */
	struct sid_timespec when = { .tv_nsec = (10 * NSEC_PER_MSEC) };

	timer_callback_cnt = 0;
	test_timer_arg = 55;

	timer_init();

	for (int cnt = 0; cnt < 8; cnt++) {
		timer_callback_cnt = 0;
		test_timer_arg += cnt;
		TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
		TEST_ASSERT_EQUAL(0, timer_callback_cnt);
		TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
		k_sleep(K_MSEC(50));
		TEST_ASSERT_EQUAL(test_timer_arg, callback_arg);
		TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
		TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	}

	timer_deinit();
}

/******************************************************************
* sid_pal_uptime_ifc
* ****************************************************************/
void test_sid_pal_uptime_get(void)
{
	struct sid_timespec sid_time = { 0 };
	int64_t uptime_msec = k_uptime_get();
	uint32_t uptime_sec = (uint32_t)(uptime_msec / MSEC_PER_SEC);
	uint32_t uptime_nsec = (uint32_t)((uptime_msec * NSEC_PER_MSEC) % NSEC_PER_SEC);

	TEST_ASSERT_NOT_EQUAL_MESSAGE(0, uptime_msec, "Test data preparation failed.");

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_uptime_now(&sid_time));

	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(uptime_sec, sid_time.tv_sec);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(uptime_nsec, sid_time.tv_nsec);
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
