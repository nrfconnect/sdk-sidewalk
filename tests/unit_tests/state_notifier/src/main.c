/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ztest_assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <state_notifier/state_notifier.h>

#include <zephyr/ztest.h>

void state_watch_init_example1(struct notifier_ctx *ctx);

struct handler_argument {
	struct notifier_state state;
};

struct handler_history {
	struct handler_argument arguments[10];
	uint32_t callcount;
};

static struct handler_history handler1_saved;

static void handler1(const struct notifier_state *state)
{
	handler1_saved.arguments[handler1_saved.callcount++] =
		(struct handler_argument){ .state = *state };
}

static struct handler_history handler2_saved;

static void handler2(const struct notifier_state *state)
{
	handler2_saved.arguments[handler2_saved.callcount++] =
		(struct handler_argument){ .state = *state };
}

static struct handler_history handler3_saved;

static void handler3(const struct notifier_state *state)
{
	handler3_saved.arguments[handler3_saved.callcount++] =
		(struct handler_argument){ .state = *state };
}

ZTEST(happy_case, test_one_receiver)
{
	struct notifier_ctx notifier_ctx = {};

	subscribe_for_state_change(&notifier_ctx, handler1);

	zassert_equal(0, handler1_saved.callcount, "handler called before state broadcast");

	application_state_s1(&notifier_ctx, 1);

	struct notifier_state expected_state = (struct notifier_state){ .s1 = 1 };

	zassert_equal(1, handler1_saved.callcount, "handler called before state broadcast");
	zassert_mem_equal(&handler1_saved.arguments[0].state, &expected_state,
			  sizeof(struct notifier_state), "Application state invalid");
}

ZTEST(happy_case, test_two_receivers)
{
	struct notifier_ctx notifier_ctx = {};

	zassert_equal(true, subscribe_for_state_change(&notifier_ctx, handler1));
	zassert_equal(true, subscribe_for_state_change(&notifier_ctx, handler2));

	zassert_equal(0, handler1_saved.callcount, "handler called before state broadcast");
	zassert_equal(0, handler2_saved.callcount, "handler called before state broadcast");
	application_state_s2(&notifier_ctx, 1);

	struct notifier_state expected_state = (struct notifier_state){ .s2 = 1 };

	zassert_equal(1, handler1_saved.callcount, "handler called before state broadcast");
	zassert_mem_equal(&handler1_saved.arguments[0].state, &expected_state,
			  sizeof(struct notifier_state), "Application state invalid");

	zassert_equal(1, handler2_saved.callcount, "handler called before state broadcast");
	zassert_mem_equal(&handler2_saved.arguments[0].state, &expected_state,
			  sizeof(struct notifier_state), "Application state invalid");
}

ZTEST(invalid_case, test_too_many_receivers)
{
	struct notifier_ctx notifier_ctx = {};

	zassert_equal(true, subscribe_for_state_change(&notifier_ctx, handler1));
	zassert_equal(true, subscribe_for_state_change(&notifier_ctx, handler2));
	zassert_equal(false, subscribe_for_state_change(&notifier_ctx, handler3));

	zassert_equal(0, handler1_saved.callcount, "handler called before state broadcast");
	zassert_equal(0, handler2_saved.callcount, "handler called before state broadcast");
	zassert_equal(0, handler3_saved.callcount, "handler called before state broadcast");
	application_state_s2(&notifier_ctx, 1);

	struct notifier_state expected_state = (struct notifier_state){ .s2 = 1 };

	zassert_equal(1, handler1_saved.callcount, "handler called before state broadcast");
	zassert_mem_equal(&handler1_saved.arguments[0].state, &expected_state,
			  sizeof(struct notifier_state), "Application state invalid");

	zassert_equal(1, handler2_saved.callcount, "handler called before state broadcast");
	zassert_mem_equal(&handler2_saved.arguments[0].state, &expected_state,
			  sizeof(struct notifier_state), "Application state invalid");

	zassert_equal(0, handler3_saved.callcount, "handler called before state broadcast");
}

struct enumerate_arguments {
	enum application_state state_id;
	uint32_t value;
};

struct enumerate_history {
	struct enumerate_arguments args[10];
	uint32_t call_count;
};

struct enumerate_history enumerate_mock_history;

void enumerate_mock(enum application_state state_id, uint32_t value)
{
	enumerate_mock_history.args[enumerate_mock_history.call_count++] =
		(struct enumerate_arguments){ .state_id = state_id, .value = value };
}

ZTEST(utils, test_enumerate_differences_1_difference)
{
	struct notifier_state state1 = (struct notifier_state){};
	struct notifier_state state2 = (struct notifier_state){ .s2 = 1 };

	enumerate_differences(&state1, &state2, enumerate_mock);

	zassert_equal(1, enumerate_mock_history.call_count, "Invalid call cound of the enumerate");
	zassert_equal(APPLICATION_STATE_s2, enumerate_mock_history.args[0].state_id,
		      "Invalid state change called");
	zassert_equal(1, enumerate_mock_history.args[0].value,
		      "Invalid value on change enumerate element");
}

ZTEST(utils, test_enumerate_differences_no_difference)
{
	struct notifier_state state1 = (struct notifier_state){ .s1 = 1 };
	struct notifier_state state2 = (struct notifier_state){ .s1 = 1 };

	enumerate_differences(&state1, &state2, enumerate_mock);

	zassert_equal(0, enumerate_mock_history.call_count, "Invalid call cound of the enumerate");
}

ZTEST(utils, test_enumerate_states)
{
	struct notifier_state state1 = (struct notifier_state){ .s1 = 1, .s4 = 1 };

	enumerate_states(&state1, enumerate_mock);

	zassert_equal(4, enumerate_mock_history.call_count, "Invalid call cound of the enumerate");

	zassert_equal(APPLICATION_STATE_s1, enumerate_mock_history.args[0].state_id,
		      "Invalid state change called");
	zassert_equal(1, enumerate_mock_history.args[0].value,
		      "Invalid value on change enumerate element");

	zassert_equal(APPLICATION_STATE_s2, enumerate_mock_history.args[1].state_id,
		      "Invalid state change called");
	zassert_equal(0, enumerate_mock_history.args[1].value,
		      "Invalid value on change enumerate element");

	zassert_equal(APPLICATION_STATE_s3, enumerate_mock_history.args[2].state_id,
		      "Invalid state change called");
	zassert_equal(0, enumerate_mock_history.args[2].value,
		      "Invalid value on change enumerate element");

	zassert_equal(APPLICATION_STATE_s4, enumerate_mock_history.args[3].state_id,
		      "Invalid state change called");
	zassert_equal(1, enumerate_mock_history.args[3].value,
		      "Invalid value on change enumerate element");
}

void clean_handlers(void *fixture)
{
	memset(&handler1_saved, 0, sizeof(handler1_saved));
	memset(&handler2_saved, 0, sizeof(handler2_saved));
	memset(&handler3_saved, 0, sizeof(handler3_saved));
}

void clean_enumerate(void *fixture)
{
	memset(&enumerate_mock_history, 0, sizeof(struct enumerate_history));
}

ZTEST_SUITE(happy_case, NULL, NULL, clean_handlers, NULL, NULL);
ZTEST_SUITE(invalid_case, NULL, NULL, clean_handlers, NULL, NULL);
ZTEST_SUITE(utils, NULL, NULL, clean_enumerate, NULL, NULL);
