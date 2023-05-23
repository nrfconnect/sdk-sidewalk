/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>

#include <flashing_led.h>
#define CALL_HISTORY_DEPTH 50
struct test_set_LED_call_history {
	int64_t call_timestamp[CALL_HISTORY_DEPTH];
	int new_state_history[CALL_HISTORY_DEPTH];
	int call_id;
};

static void set_LED(int new_state, void *ctx)
{
	struct test_set_LED_call_history *history = (struct test_set_LED_call_history *)ctx;

	printk("LED call %d time %d\n", history->call_id, k_uptime_get());
	history->call_timestamp[history->call_id] = k_uptime_get();
	history->new_state_history[history->call_id] = new_state;
	history->call_id = history->call_id + 1;
}

ZTEST(pattern, test_0_element_patern){
	DEFINE_PATTERN(zero_element_pattern);
	static struct test_set_LED_call_history history = { 0 };
	static struct test_set_LED_call_history history_expected = { 0 };

	pattern_id id = play_toggle_pattern(
		false, set_LED, &history, zero_element_pattern,
		LED_PATTERN_LENGTH(zero_element_pattern), false);

	zassert_equal(-EINVAL, id, "invalid");
	zassert_mem_equal(&history, &history_expected, sizeof(history_expected));
}

ZTEST(pattern, test_1_element_patern){
	DEFINE_PATTERN(zero_element_pattern, 10);
	static struct test_set_LED_call_history history = { 0 };

	int64_t current_time = k_uptime_get();
	pattern_id id = play_toggle_pattern(
		false, set_LED, &history, zero_element_pattern,
		LED_PATTERN_LENGTH(zero_element_pattern), false);

	k_sleep(K_MSEC(20));
	zassert_equal(0, id, "invalid");
	zassert_equal(2, history.call_id, "invalid call count");
	zassert_within(current_time, history.call_timestamp[0], 1, "call time not within 1ms");
	zassert_within(current_time + 10, history.call_timestamp[1], 1, "call time not within 1ms");
	zassert_equal(false, history.new_state_history[0], "wrong initial state");
	zassert_equal(true, history.new_state_history[1], "wrong initial state");
}

ZTEST(pattern, test_1_element_patern_loopback){
	DEFINE_PATTERN(zero_element_pattern, 50);
	static struct test_set_LED_call_history history = { 0 };

	int64_t current_time = k_uptime_get();
	pattern_id id = play_toggle_pattern(
		false, set_LED, &history, zero_element_pattern,
		LED_PATTERN_LENGTH(zero_element_pattern), true);

	k_sleep(K_MSEC(125));
	stop_toggle_pattern(id);
	zassert_equal(0, id, "invalid");
	zassert_equal(3, history.call_id, "invalid call count");
	zassert_within(current_time, history.call_timestamp[0], 5, "call time not within 5ms");
	zassert_within(current_time + 50, history.call_timestamp[1], 5, "call time not within 5ms");

	zassert_within(current_time + 50 + 50, history.call_timestamp[2], 5, "call time not within 5ms");
	zassert_equal(false, history.new_state_history[0], "wrong initial state");
	zassert_equal(true, history.new_state_history[1], "wrong state");
	zassert_equal(false, history.new_state_history[2], "wrong state");
}

ZTEST(pattern, test_2_element_patern){
	DEFINE_PATTERN(zero_element_pattern, 50, 25);
	static struct test_set_LED_call_history history = { 0 };

	int64_t current_time = k_uptime_get();
	pattern_id id = play_toggle_pattern(
		false, set_LED, &history, zero_element_pattern,
		LED_PATTERN_LENGTH(zero_element_pattern), false);

	k_sleep(K_MSEC(100));

	zassert_equal(0, id, "invalid");
	zassert_equal(3, history.call_id, "invalid call count");
	zassert_within(current_time, history.call_timestamp[0], 1, "call time not within 5ms");
	zassert_within(current_time + 50, history.call_timestamp[1], 5, "call time not within 5ms");
	zassert_within(current_time + 50 + 25, history.call_timestamp[2], 5, "call time not within 5ms");
	zassert_equal(false, history.new_state_history[0], "wrong initial state");
	zassert_equal(true, history.new_state_history[1], "wrong initial state");
	zassert_equal(false, history.new_state_history[2], "wrong initial state");
}

ZTEST_SUITE(pattern, NULL, NULL, NULL, NULL, NULL);
