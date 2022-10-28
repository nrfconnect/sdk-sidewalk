/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_timer_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_time_ops.h>
#include <sid_pal_log_ifc.h>
#include <unity.h>
#include <string.h>

#define SID_PAL_CASUAL_TIMER_TESTS 1

#define SID_CLOCK_SOURCE_UPTIME 0

// Arbitrary number of timers to test
#define NUMBER_OF_TIMERS_TO_TEST            3

/* A queue of timer expiry events is required, the checking/reporting can't
 * be conducted in interrupt context. For one-shot timer tests this just
 * needs to be at least NUMBER_OF_TIMERS_TO_TEST, but as the periodic & precision
 * tests accumulate timer expiry events then post-process them, it is larger.
 */
#define LENGTH_OF_EXPIRY_QUEUE              100

/* Software "spin loop" used to provide an *approximate* blocking time to
 * allow sanity checking of the official timer mechanism's timeouts.
 *
 * NOTE: This is an independent time reference to things like sid_clock
 * to ensure gross errors (such as getting HW clock scaling wrong) are
 * more likely to be detected. There are later tests cases that measure
 * precision but require the internal clock reference to be correct.
 *
 * This is NOT supposed or expected to be super-accurate, it will be disturbed
 * by interrupt activity for example.  It should be used only to provide a
 * check against gross time errors such e.g. very short or 2(+) x timeouts.
 *
 * NOTE: The magic coefficient will need calibrating on a PER PLATFORM basis
 * because it is dependant on CPU clock frequency, memory access time etc.
 * It must be set in the relevant application level 'CMakeLists.txt'
 */
#define PLATFORM_SW_SPIN_DELAY_MILLISEC(_ms) \
    do { for(volatile uint32_t i = ((_ms) * SID_PAL_PLATFORM_SW_SPIN_COUNT_PER_MS); i != 0; i--) {} } while(0)

// Overall timeout for one-shot testing. Must be longer than max test time in 'test_one_shot_timespecs'
#define TOTAL_ONE_SHOT_TEST_TIME_MS         2000

/* The spin loop timer is queued in chunks of time this long.  The cumulative
 * time summed from these delays will "slip" due to the checking code run
 * between the spin loops and other system interrupt activity.
 * Don't make this too small because the overhead becomes more significant.
 */
#define POLLING_TIMER_GRANULARITY_MS        50

#define TIMER_CANCEL_TEST_TIME_MS           100
#define TIMER_CANCEL_TIMEOUT_MS             (2 * TIMER_CANCEL_TEST_TIME_MS)

/* How often the short periodic timer will repeat in the precision test.
 * This should be smaller than POLLING_TIMER_GRANULARITY_MS.
 */
#define TIMER_PERIODIC_REPEAT_TIME_MS       20ul

/* How much processing overhead we allow for in periodic timers. Initial testing
 * showed this could typically be between 30us and 60 (1 to 2 slow clock ticks)
 * with occasional higher values, so for unit testing we allow 100us in total.
 * The test will fail if there is any long-term drift between timer expiries,
 * this number is the max jitter allowed for single events.
 */
#define TIMER_PERIODIC_ALLOWED_OVERHEAD_US  100

/* Define the two timer periods for the casual timer test. These periods have
 * to be more than 1s apart to be able to test casual timer delays.
 */
#define TIMER_PRECISE_INDEX                 0
#define TIMER_PRECISE_PERIOD_MS             1200ul
#define TIMER_CASUAL_INDEX                  1
#define TIMER_CASUAL_PERIOD_MS              55ul

/* The number of "precise" events of the *casual* timer, before it snaps
 * to the next precise timer event. Note the use of 1s (1000ms) magic number
 * here, as the tolerance of casual timers is defined in a static const
 * in the implementation source, and therefore isn't accessible here.
 *
 * This number is one less than the total number of casual timer events
 */
#define NUM_CASUAL_TIMEOUTS_BEFORE_SNAP     ((TIMER_PRECISE_PERIOD_MS - 1000) / TIMER_CASUAL_PERIOD_MS)

/* The total number of timer cycles when testing casual timers. One cycle is
 * one precise timer and (NUM_CASUAL_TIMEOUTS_BEFORE_SNAP+1) casual timer events.
 */
#define TEST_CASUAL_TIMER_CYCLES            5

// The timer instance number is used to generate (simple!) arbitrary test data
#define CALC_CHECK_ARG_FROM_INSTANCE(inst)  (NUMBER_OF_TIMERS_TO_TEST - (inst))
#define CALC_INSTANCE_FROM_TMR_PTR(tmr_ptr) \
    (((uint32_t)(tmr_ptr) - (uint32_t)test_timers) / sizeof(sid_pal_timer_t))

// Reset the event queue
#define RESET_TIMER_EVENT_QUEUE \
    do { expiry_event_q_write = expiry_event_q_read = 0; } while(0)

struct arg_data {
    uint32_t arg_u32;   // additional timer ID verified in one-shot timer tests
    struct sid_timespec offset; // used with one-shot timer tests
    struct sid_timespec next_timeout;   //used with periodic & casual timer tests
    bool casual_timer;
};

struct expiry_event {
    struct sid_timespec delay;
    void *p_arg_data;
    sid_pal_timer_t *originator;
    bool timer_jumped;
};

// This is modified by the timer expiry callback function
static sid_pal_timer_t test_timers[NUMBER_OF_TIMERS_TO_TEST];

// Argument data, used to cross check timer instances and queued times
static struct arg_data timer_arg_data[NUMBER_OF_TIMERS_TO_TEST];

static uint32_t polling_time_ms = 0;

volatile uint8_t expiry_event_q_write = 0;
volatile uint8_t expiry_event_q_read = 0;
struct expiry_event expiry_events[LENGTH_OF_EXPIRY_QUEUE];

/* Timer test time definitions.  Note: For now, just use a single test
 * time for each timer instance we test.
 *
 * NOTE:  We *roughly* check these using a simple software spin loop.
 * See the definition of POLLING_TIMER_GRANULARITY_MS and don't use
 * times here that are less than two times that period in milliseconds
 */
const struct sid_timespec test_one_shot_timespecs[NUMBER_OF_TIMERS_TO_TEST] =
{
   { 1UL, 500000000UL },  // 1.5 secs
   { 1UL, 0UL },          // 1.0 secs
   { 0UL, 500000000UL }   // 0.5 secs
};

// NOTE: This timeout handler is common to all timers.  It queues an expiry event
static void test_timer_cb_common(void *arg, sid_pal_timer_t *originator)
{
    struct sid_timespec now;
    struct arg_data *timer_arg = (struct arg_data*)arg;

    // Get the current time as early as possible in the handler, to minimise delays
    sid_pal_uptime_now(&now);

    // The event queue is single-use for each test case - reset it by zeroing the write index
    if (expiry_event_q_write < LENGTH_OF_EXPIRY_QUEUE) {
        expiry_events[expiry_event_q_write].timer_jumped = false;

        if (sid_time_is_zero(&timer_arg->next_timeout)) {
            expiry_events[expiry_event_q_write].delay = SID_TIME_ZERO;
        } else {
            // Calculate the delay from the expected firing time
            sid_time_delta(&expiry_events[expiry_event_q_write].delay, &now, &timer_arg->next_timeout);

            // Update the expected timeout for a periodic timer assuming it's precise for now
            sid_time_add(&timer_arg->next_timeout, &originator->period);

            /* If the delay for this event was substantially larger than the
             * expected period then it is possibly due to a casual timer getting
             * delayed. Check for casual timer and if it has snapped then
             * recompute the next event time from the current time instead.
             */
            expiry_events[expiry_event_q_write].timer_jumped =
                sid_time_gt(&expiry_events[expiry_event_q_write].delay, &originator->period);

            if(timer_arg->casual_timer && expiry_events[expiry_event_q_write].timer_jumped) {
                timer_arg->next_timeout = originator->alarm;
            }
        }

        // Write data into the queue, for subsequent NON-interrupt context handling
        expiry_events[expiry_event_q_write].p_arg_data = arg;
        expiry_events[expiry_event_q_write].originator = originator;

        expiry_event_q_write++;
    }
}

void test_timer_init(void)
{
    uint32_t tmr_inst;

    // Initialize all the timers with a common callback but with the arg pointing to different structures
    for (tmr_inst = 0; tmr_inst < NUMBER_OF_TIMERS_TO_TEST; tmr_inst++) {
        const sid_error_t ret = sid_pal_timer_init(&test_timers[tmr_inst], test_timer_cb_common, &timer_arg_data[tmr_inst]);
        TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    }
}

void test_canceling_timer_fails_given_no_timer_arg(void)
{
    const sid_error_t sid_error = sid_pal_timer_cancel(NULL);
    TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_error);
}

void test_timer_is_armed_fails_given_no_timer_arg(void)
{
    const bool armed = sid_pal_timer_is_armed(NULL);
    TEST_ASSERT_FALSE(armed);
}

void test_unarmed_timer_can_be_canceled(void)
{
    uint32_t tmr_inst;

    for (tmr_inst = 0; tmr_inst < NUMBER_OF_TIMERS_TO_TEST; tmr_inst++) {
        const bool armed = sid_pal_timer_is_armed(&test_timers[tmr_inst]);
        TEST_ASSERT_FALSE(armed);

        const sid_error_t ret = sid_pal_timer_cancel(&test_timers[tmr_inst]);
        TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    }
}

void test_arming_timer_fails_given_no_timer_arg(void)
{
    const sid_error_t ret = sid_pal_timer_arm(NULL, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &test_one_shot_timespecs[0], NULL);
    TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, ret);
}

void test_arming_timer_fails_given_no_deadline(void)
{
    const sid_error_t ret = sid_pal_timer_arm(&test_timers[0], SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL, NULL);
    TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, ret);
}

// Helper: This function acts as a convenient local "wrapper" around the pal timer API
static sid_error_t timer_arm_relative(uint32_t tmr_inst,
                                      sid_pal_timer_prio_class_t type,
                                      const struct sid_timespec *relative,
                                      const struct sid_timespec *period)
{
    struct sid_timespec first_shot = { .tv_sec = 0, .tv_nsec = 0 };

    // Set arg data structure with a derived instance number and the required time delay value
    timer_arg_data[tmr_inst].arg_u32 = CALC_CHECK_ARG_FROM_INSTANCE(tmr_inst);
    timer_arg_data[tmr_inst].offset = *relative;

    if(type == SID_PAL_TIMER_PRIO_CLASS_LOWPOWER) {
        timer_arg_data[tmr_inst].casual_timer = true;
    } else {
        timer_arg_data[tmr_inst].casual_timer = false;
    }

    const sid_error_t ret = sid_pal_uptime_now(&first_shot);
    if (SID_ERROR_NONE != ret) {
        return ret;
    }
    sid_time_add(&first_shot, relative);

    return sid_pal_timer_arm(&test_timers[tmr_inst], type, &first_shot, period);
}

// Helper: Invoked by polling when there is something in the queue to process
static void process_one_shot_timer_expiry(void)
{
    struct arg_data *p_arg_data = expiry_events[expiry_event_q_read].p_arg_data;
    const sid_pal_timer_t *originator = expiry_events[expiry_event_q_read].originator;
    const uint32_t instance = CALC_INSTANCE_FROM_TMR_PTR(originator);

    /* Note:  This math won't work for all possible times that can be queued
     * but will work for all the times we can sensibly test.
     * Please also note comments about minimum POLLING_TIMER_GRANULARITY_MS
     */
    const uint32_t expected_expiry_ms = (p_arg_data->offset.tv_sec  * 1000) +
                                        (p_arg_data->offset.tv_nsec / 1000000);

    // check calculated instance number is within bounds
    TEST_ASSERT_LESS_THAN_UINT32(NUMBER_OF_TIMERS_TO_TEST, instance);

    // Check the arg data corresponds with the timer it was saved in
    TEST_ASSERT_EQUAL_UINT32(p_arg_data->arg_u32, CALC_CHECK_ARG_FROM_INSTANCE(instance));

    /* Compare the configured delay time with what has been approximated with
     * the spin loop mechanism.  We deliberately "widen" the acceptance gates
     * because this is not an exact science, this is just a sanity check!
     * Debugging has shown that the cumulative spin time is consistently less
     * than the queued times hence the asymmetric + 1/3 and - 1/2 thresholds.
     */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(polling_time_ms, (expected_expiry_ms + (expected_expiry_ms / 3)));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(polling_time_ms, (expected_expiry_ms - (expected_expiry_ms / 2)));
}

void test_one_shot_timers_fire_after_arming(void)
{
    uint32_t tmr_inst;

    // Reset the event queue pointers for this test run
    RESET_TIMER_EVENT_QUEUE;

    // Arm (effectively "queue") all the timers using time specifications from the test data
    for (tmr_inst = 0; tmr_inst < NUMBER_OF_TIMERS_TO_TEST; tmr_inst++) {
        timer_arg_data[tmr_inst].next_timeout = SID_TIME_ZERO;

        const sid_error_t ret = timer_arm_relative(tmr_inst, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &test_one_shot_timespecs[tmr_inst], NULL);
        TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

        const bool armed = sid_pal_timer_is_armed(&test_timers[tmr_inst]);
        TEST_ASSERT_TRUE(armed);
    }

    // Wait for timers to trigger
    for (polling_time_ms = 0; polling_time_ms < TOTAL_ONE_SHOT_TEST_TIME_MS; ) {
        PLATFORM_SW_SPIN_DELAY_MILLISEC(POLLING_TIMER_GRANULARITY_MS);
        polling_time_ms += POLLING_TIMER_GRANULARITY_MS;

        if (expiry_event_q_read != expiry_event_q_write) {
            process_one_shot_timer_expiry();

            // Each timer will fire once
            if (++expiry_event_q_read >= NUMBER_OF_TIMERS_TO_TEST) {
                break;
            }
        }
    }

    // Check all timer callbacks were received before timeout
    TEST_ASSERT_LESS_THAN_UINT32(TOTAL_ONE_SHOT_TEST_TIME_MS, polling_time_ms);
}

void test_one_shot_timers_dont_repeat(void)
{
    uint32_t tmr_inst;

    for (tmr_inst = 0; tmr_inst < NUMBER_OF_TIMERS_TO_TEST; tmr_inst++) {
        const bool armed = sid_pal_timer_is_armed(&test_timers[tmr_inst]);
        TEST_ASSERT_FALSE(armed);
    }
}

void test_timer_can_be_canceled_after_arming(void)
{
    const uint32_t tmr_inst = 0;
    const struct sid_timespec delay_time = { .tv_sec = 0, .tv_nsec = (TIMER_CANCEL_TEST_TIME_MS * 1000000) };

    // Reset the event queue pointers for this test run
    RESET_TIMER_EVENT_QUEUE;

    // Sanity check timer is NOT armed
    const bool armed = sid_pal_timer_is_armed(&test_timers[tmr_inst]);
    TEST_ASSERT_FALSE(armed);

    // arm another timer
    const sid_error_t ret = timer_arm_relative(tmr_inst, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &delay_time, NULL);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    // Immediately cancel, then wait until after it would have expired
    const sid_error_t cancel_ret = sid_pal_timer_cancel(&test_timers[tmr_inst]);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, cancel_ret);
    PLATFORM_SW_SPIN_DELAY_MILLISEC(TIMER_CANCEL_TIMEOUT_MS);

    // check for timer expiry at end of timeout period
    TEST_ASSERT_EQUAL_UINT8(expiry_event_q_read, expiry_event_q_write);
}

void test_periodic_timer_is_precise(void)
{
    const uint32_t tmr_inst = 0;
    struct sid_timespec period;

    // Reset the event queue pointers for this test run
    RESET_TIMER_EVENT_QUEUE;

    /* Arm a periodic timer, then wait for a number of expiry events to be queued.
     * Measure the delta between each expiry and look for consistency (e.g. to
     * make sure processing time around an expiry event doesn't accumulate into
     * subsequent expiry times). The period of the timer is short, with a large
     * number of expiry events to provide more data.
     */
    period.tv_nsec = (TIMER_PERIODIC_REPEAT_TIME_MS * SID_TIME_NSEC_PER_MSEC);
    period.tv_sec = 0;

    const sid_error_t ret = timer_arm_relative(tmr_inst, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &period, &period);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    // After the timer has been armed we can copy the initial timeout time
    timer_arg_data[tmr_inst].next_timeout = test_timers[tmr_inst].alarm;

    // Spin until the event queue is almost full
    PLATFORM_SW_SPIN_DELAY_MILLISEC(
        (LENGTH_OF_EXPIRY_QUEUE * TIMER_PERIODIC_REPEAT_TIME_MS) - (2 * POLLING_TIMER_GRANULARITY_MS));

    // Cancel the periodic timer to stop further events getting queued
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timers[tmr_inst]));

    // Check we got at least the expected number of events (it may be one more)
    const uint32_t expected_events =
        (LENGTH_OF_EXPIRY_QUEUE - (2 * POLLING_TIMER_GRANULARITY_MS) / TIMER_PERIODIC_REPEAT_TIME_MS);
    TEST_ASSERT_GREATER_OR_EQUAL(expected_events, expiry_event_q_write);

    // Iterate through the queued events comparing delta against previous times
    while(expiry_event_q_read < expiry_event_q_write) {
        /* The delta should be almost exactly the repeating period (allowing
         * for callback processing time), and never less.
         */
        const uint32_t max_delta =
            (TIMER_PERIODIC_ALLOWED_OVERHEAD_US * SID_TIME_NSEC_PER_USEC);

        if (max_delta < expiry_events[expiry_event_q_read].delay.tv_nsec) {
            // Test is about to fail so log the values
            SID_PAL_LOG_INFO("Evt %d: delay = %ld", expiry_event_q_read, expiry_events[expiry_event_q_read].delay.tv_nsec);
        }

        TEST_ASSERT_EQUAL_UINT32(0, expiry_events[expiry_event_q_read].delay.tv_sec);
        TEST_ASSERT_LESS_OR_EQUAL_UINT32(max_delta, expiry_events[expiry_event_q_read].delay.tv_nsec);

        expiry_event_q_read++;
    }
}

void test_casual_timer_snaps(void)
{
#ifdef SID_PAL_CASUAL_TIMER_TESTS
    struct sid_timespec offset, period;
    sid_error_t ret;
    uint8_t unsnapped_casual_timers;

    // Reset the event queue pointers for this test run
    RESET_TIMER_EVENT_QUEUE;

    /* Start two timers - a precise timer with a >1s period, and a casual
     * timer with a <<1s period - the two periods should also be more than 1s
     * apart (where 1s is the maximum delay that can be applied to a casual timer).
     *
     * The expected behaviour is for the casual timer to alternate between
     * snapping to the precise timer and expiring (precisely) on it's own period.
     * The number of casual timed events between precise timed events depends on
     * the respective periods used (defined at the top of this file). For the
     * test pattern of 1200ms precise and 55ms casual, the pattern will be:
     *
     *  Casual @ t0
     *  Precise @ t0
     *  Casual @ t0+(1*55)
     *  Casual @ t0+(2*55)
     *  Casual @ t0+(3*55)
     *
     *  Casual @ t0+1200
     *  Precise @ t0+1200
     *  etc.
     */
    offset.tv_nsec = (2 * SID_TIME_NSEC_PER_MSEC);  // start 2ms in the future
    offset.tv_sec = 0;
    period.tv_nsec = (TIMER_PRECISE_PERIOD_MS * SID_TIME_NSEC_PER_MSEC);
    period.tv_sec = 0;
    sid_time_normalize(&period);

    // Start the precise timer first, so that the casual timer can snap to it straight away
    ret = timer_arm_relative(TIMER_PRECISE_INDEX, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &offset, &period);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    offset.tv_nsec = (1 * SID_TIME_NSEC_PER_MSEC);  // attempt to start 1ms in the future
    period.tv_nsec = (TIMER_CASUAL_PERIOD_MS * SID_TIME_NSEC_PER_MSEC);
    period.tv_sec = 0;
    sid_time_normalize(&period);

    ret = timer_arm_relative(TIMER_CASUAL_INDEX, SID_PAL_TIMER_PRIO_CLASS_LOWPOWER, &offset, &period);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    // Confirm that the earlier casual timer has snapped to the slightly later precise timer
    TEST_ASSERT_EQUAL_UINT32(test_timers[TIMER_PRECISE_INDEX].alarm.tv_sec,
        test_timers[TIMER_CASUAL_INDEX].alarm.tv_sec);
    TEST_ASSERT_EQUAL_UINT32(test_timers[TIMER_PRECISE_INDEX].alarm.tv_nsec,
        test_timers[TIMER_CASUAL_INDEX].alarm.tv_nsec);

    // After the timers have been armed we can copy the initial timeout time
    timer_arg_data[TIMER_PRECISE_INDEX].next_timeout = test_timers[TIMER_PRECISE_INDEX].alarm;
    timer_arg_data[TIMER_CASUAL_INDEX].next_timeout = test_timers[TIMER_CASUAL_INDEX].alarm;

    // Spin until several precise timer events have occurred
    const uint32_t expected_events = (NUM_CASUAL_TIMEOUTS_BEFORE_SNAP + 2) * TEST_CASUAL_TIMER_CYCLES;
    while(expiry_event_q_write < expected_events) {
        PLATFORM_SW_SPIN_DELAY_MILLISEC(TIMER_CASUAL_PERIOD_MS);
    }

    // Cancel the timers to stop further events getting queued
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timers[TIMER_PRECISE_INDEX]));
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timers[TIMER_CASUAL_INDEX]));

    /* Iterate through the queued events comparing delta against previous times.
     * There is a repeating pattern of unsnapped and snapped casual timers so
     * also check the timer events follow that pattern.
     */
    unsnapped_casual_timers = NUM_CASUAL_TIMEOUTS_BEFORE_SNAP;
    while(expiry_event_q_read < expiry_event_q_write) {
        const struct expiry_event *evt = &expiry_events[expiry_event_q_read];
        const bool is_casual = (evt->originator == &test_timers[TIMER_CASUAL_INDEX]);

        /* The delta should be almost exactly the repeating period (allowing
         * for callback processing time), and never less.
         */
        const uint32_t max_delta =
            (TIMER_PERIODIC_ALLOWED_OVERHEAD_US * SID_TIME_NSEC_PER_USEC);

        if ((!is_casual || unsnapped_casual_timers) && (max_delta < evt->delay.tv_nsec)) {
            // Test is about to fail so log the values
            if (is_casual) {
                SID_PAL_LOG_INFO("Evt %d: [C] delay = %ld", expiry_event_q_read, evt->delay.tv_nsec);
            } else {
                SID_PAL_LOG_INFO("Evt %d: [P] delay = %ld", expiry_event_q_read, evt->delay.tv_nsec);
            }

            TEST_ASSERT_EQUAL_UINT32(0, expiry_events[expiry_event_q_read].delay.tv_sec);
            TEST_ASSERT_LESS_OR_EQUAL_UINT32(max_delta, expiry_events[expiry_event_q_read].delay.tv_nsec);

        } else if (is_casual && !unsnapped_casual_timers && !evt->timer_jumped) {
            // The last casual timer should have jumped, but didn't, so fail
            SID_PAL_LOG_INFO("Evt %d: [C] delay = %ld - DIDN'T SNAP", expiry_event_q_read, evt->delay.tv_nsec);
            TEST_ASSERT_EQUAL(true, evt->timer_jumped);
        }

        if (is_casual && unsnapped_casual_timers > 0) {
            unsnapped_casual_timers--;
        } else {
            // Reset unsnapped count on reaching 0 or precise timer
            unsnapped_casual_timers = NUM_CASUAL_TIMEOUTS_BEFORE_SNAP;
        }

        expiry_event_q_read++;
    }
#else
    TEST_IGNORE_MESSAGE("Skipping, unsupported on this platform.");
#endif
}

void test_casual_timer_doesnt_snap(void)
{
#ifdef SID_PAL_CASUAL_TIMER_TESTS
    struct sid_timespec offset, period;
    sid_error_t ret;

    // Reset the event queue pointers for this test run
    RESET_TIMER_EVENT_QUEUE;

    /* Start two timers - a precise timer and a casual timer with the same period
     * but slightly different expiry times. Start the casual timer first so that
     * it never re-arms with the precise timer later than it. The casual timer
     * should therefore always expire at the expected time (i.e. it's also precise).
     *
     * This test doesn't need either of the timers to have a long period, so use
     * the short (casual) period for both timers to complete the test faster.
     */
    offset.tv_nsec = (1 * SID_TIME_NSEC_PER_MSEC);
    offset.tv_sec = 0;
    period.tv_nsec = (TIMER_CASUAL_PERIOD_MS * SID_TIME_NSEC_PER_MSEC);
    period.tv_sec = 0;
    sid_time_normalize(&period);

    ret = timer_arm_relative(TIMER_CASUAL_INDEX, SID_PAL_TIMER_PRIO_CLASS_LOWPOWER, &offset, &period);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    offset.tv_nsec = (2 * SID_TIME_NSEC_PER_MSEC);
    ret = timer_arm_relative(TIMER_PRECISE_INDEX, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &offset, &period);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    // Confirm that the casual timer has not snapped to the precise timer
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(test_timers[TIMER_PRECISE_INDEX].alarm.tv_sec,
        test_timers[TIMER_CASUAL_INDEX].alarm.tv_sec);
    TEST_ASSERT_NOT_EQUAL_UINT32(test_timers[TIMER_PRECISE_INDEX].alarm.tv_nsec,
        test_timers[TIMER_CASUAL_INDEX].alarm.tv_nsec);

    // After the timers have been armed we can copy the initial timeout time
    timer_arg_data[TIMER_PRECISE_INDEX].next_timeout = test_timers[TIMER_PRECISE_INDEX].alarm;
    timer_arg_data[TIMER_CASUAL_INDEX].next_timeout = test_timers[TIMER_CASUAL_INDEX].alarm;

    // Wait for enough test events to accumulate
    struct sid_timespec now;
    sid_pal_uptime_now(&now);
    SID_PAL_LOG_INFO("Starting test at: %ld.%ld", now.tv_sec, now.tv_nsec);
    SID_PAL_LOG_INFO("First expiry due: %ld.%ld",
        timer_arg_data[TIMER_CASUAL_INDEX].next_timeout.tv_sec,
        timer_arg_data[TIMER_CASUAL_INDEX].next_timeout.tv_nsec);
    SID_PAL_LOG_INFO("Precise period:   %ld.%ld",
        test_timers[TIMER_PRECISE_INDEX].period.tv_sec,
        test_timers[TIMER_PRECISE_INDEX].period.tv_nsec);
    SID_PAL_LOG_INFO("Casual period:    %ld.%ld",
        test_timers[TIMER_CASUAL_INDEX].period.tv_sec,
        test_timers[TIMER_CASUAL_INDEX].period.tv_nsec);

    // Spin until several timer pair events have occurred
    const uint32_t expected_events = 2 * TEST_CASUAL_TIMER_CYCLES;
    while(expiry_event_q_write < expected_events) {
        PLATFORM_SW_SPIN_DELAY_MILLISEC(TIMER_CASUAL_PERIOD_MS);
    }

    sid_pal_uptime_now(&now);
    SID_PAL_LOG_INFO("  Ending test at: %ld.%ld", now.tv_sec, now.tv_nsec);
    SID_PAL_LOG_INFO("Gathered %d timer events", expiry_event_q_write);

    // Cancel the timers to stop further events getting queued
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timers[TIMER_PRECISE_INDEX]));
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timers[TIMER_CASUAL_INDEX]));

    /* Iterate through the queued events comparing delta against previous times.
     * There is a repeating pattern of unsnapped and snapped casual timers so
     * also check the timer events follow that pattern.
     */
    while(expiry_event_q_read < expiry_event_q_write) {
        const struct expiry_event *evt = &expiry_events[expiry_event_q_read];
        const bool is_casual = (evt->originator == &test_timers[TIMER_CASUAL_INDEX]);

        /* The delta should be almost exactly the repeating period (allowing
         * for callback processing time), and never less.
         */
        const uint32_t max_delta =
            (TIMER_PERIODIC_ALLOWED_OVERHEAD_US * SID_TIME_NSEC_PER_USEC);

            if (is_casual) {
                SID_PAL_LOG_INFO("Evt %d: [C] delay = %ld", expiry_event_q_read, evt->delay.tv_nsec);
            } else {
                SID_PAL_LOG_INFO("Evt %d: [P] delay = %ld", expiry_event_q_read, evt->delay.tv_nsec);
            }

        if (max_delta < evt->delay.tv_nsec) {
            // Test is about to fail so log the values
            if (is_casual) {
                SID_PAL_LOG_INFO("Evt %d: [C] delay = %ld", expiry_event_q_read, evt->delay.tv_nsec);
            } else {
                SID_PAL_LOG_INFO("Evt %d: [P] delay = %ld", expiry_event_q_read, evt->delay.tv_nsec);
            }

            TEST_ASSERT_EQUAL_UINT32(0, expiry_events[expiry_event_q_read].delay.tv_sec);
            TEST_ASSERT_LESS_OR_EQUAL_UINT32(max_delta, expiry_events[expiry_event_q_read].delay.tv_nsec);
        }

        expiry_event_q_read++;
    }
#else
    TEST_IGNORE_MESSAGE("Skipping, unsupported on this platform.");
#endif
}

void test_timer_deinit(void)
{
    uint32_t tmr_inst;

    for (tmr_inst = 0; tmr_inst < NUMBER_OF_TIMERS_TO_TEST; tmr_inst++) {
        const sid_error_t ret = sid_pal_timer_deinit(&test_timers[tmr_inst]);
        TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    }
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
