/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/ztest.h>

/* header under test */
#include <cli/location_shell.h>
#include <cli/location_shell_events.h>

/* mocks and dummy headers */
#include <mock/shell_mock.h>
#include <mock/sidewalk_mock.h>

/* Shell is not used by the commands apart from printing. */
#define TEST_SHELL ((const struct shell *)NULL)

static int run_cmd(int (*cmd)(const struct shell *, int32_t, const char **), int32_t argc,
		   const char **argv)
{
	return cmd(TEST_SHELL, argc, argv);
}

static void assert_event_sent(event_handler_t event)
{
	zassert_equal(1, sidewalk_event_send_fake.call_count);
	zassert_equal(event, sidewalk_event_send_fake.arg0_val);
	zassert_equal(0, sid_hal_free_fake.call_count);
	free(sidewalk_event_send_fake.arg1_val);
}

static void assert_nothing_sent(void)
{
	zassert_equal(0, sidewalk_event_send_fake.call_count);
}

ZTEST(sid_location_shell, test_init_deinit)
{
	const char *argv[] = { "cmd" };

	zassert_equal(0, run_cmd(cmd_location_init, 1, argv));
	zassert_equal(location_event_init, sidewalk_event_send_fake.arg0_val);
	zassert_is_null(sidewalk_event_send_fake.arg1_val);

	zassert_equal(0, run_cmd(cmd_location_deinit, 1, argv));
	zassert_equal(location_event_deinit, sidewalk_event_send_fake.arg0_val);
	zassert_equal(2, sidewalk_event_send_fake.call_count);
}

ZTEST(sid_location_shell, test_init_too_many_args)
{
	const char *argv[] = { "cmd", "1" };

	zassert_equal(-EINVAL, run_cmd(cmd_location_init, 2, argv));
	assert_nothing_sent();
}

struct level_params {
	int (*cmd)(const struct shell *, int32_t, const char **);
	event_handler_t event;
	int min_level;
	int max_level;
	bool allow_default;
};

static void test_level_cmd(const struct level_params *p)
{
	const char *argv[2] = { "cmd", NULL };
	char level[4];

	/* No argument: default effort, unless the command requires a level. */
	if (p->allow_default) {
		zassert_equal(0, run_cmd(p->cmd, 1, argv));
		zassert_equal(SID_LOCATION_EFFORT_DEFAULT,
			      *(uint32_t *)sidewalk_event_send_fake.arg1_val);
		assert_event_sent(p->event);
	} else {
		zassert_equal(-EINVAL, run_cmd(p->cmd, 1, argv));
		assert_nothing_sent();
	}

	for (int l = SID_LOCATION_EFFORT_L1; l <= SID_LOCATION_EFFORT_L6; l++) {
		RESET_FAKE(sidewalk_event_send);
		sidewalk_event_send_fake.return_val = 0;
		snprintf(level, sizeof(level), "%d", l);
		argv[1] = level;

		if (l < p->min_level || l > p->max_level) {
			zassert_equal(-EINVAL, run_cmd(p->cmd, 2, argv), "level %d", l);
			assert_nothing_sent();
			continue;
		}
		zassert_equal(0, run_cmd(p->cmd, 2, argv), "level %d", l);
		zassert_equal((uint32_t)l, *(uint32_t *)sidewalk_event_send_fake.arg1_val);
		assert_event_sent(p->event);
	}

	const char *bad[] = { "0", "7", "-1", "abc", "3x", "" };

	for (size_t i = 0; i < ARRAY_SIZE(bad); i++) {
		RESET_FAKE(sidewalk_event_send);
		argv[1] = bad[i];
		zassert_equal(-EINVAL, run_cmd(p->cmd, 2, argv), "arg [%s]", bad[i]);
		assert_nothing_sent();
	}

	const char *too_many[] = { "cmd", "3", "4" };

	zassert_equal(-EINVAL, run_cmd(p->cmd, 3, too_many));
}

ZTEST(sid_location_shell, test_send_levels_range)
{
	test_level_cmd(&(struct level_params){ cmd_location_send, location_event_send,
					       SID_LOCATION_EFFORT_L1, SID_LOCATION_EFFORT_L6,
					       true });
}

ZTEST(sid_location_shell, test_scan_levels_range)
{
	test_level_cmd(&(struct level_params){ cmd_location_scan, location_event_scan,
					       SID_LOCATION_EFFORT_L3, SID_LOCATION_EFFORT_L4,
					       true });
}

ZTEST(sid_location_shell, test_send_buf_levels_range)
{
	test_level_cmd(&(struct level_params){ cmd_location_send_buf, location_event_send_buffer,
					       SID_LOCATION_EFFORT_L3, SID_LOCATION_EFFORT_L4,
					       false });
}

ZTEST(sid_location_shell, test_level_cmd_alloc_fail)
{
	const char *argv[] = { "cmd", "3" };

	sid_hal_malloc_fake.custom_fake = NULL;
	sid_hal_malloc_fake.return_val = NULL;
	zassert_equal(-ENOMEM, run_cmd(cmd_location_send, 2, argv));
	assert_nothing_sent();
}

ZTEST(sid_location_shell, test_level_cmd_send_fail_frees_ctx)
{
	const char *argv[] = { "cmd", "3" };

	sidewalk_event_send_fake.return_val = -EAGAIN;
	zassert_equal(-ENOMSG, run_cmd(cmd_location_send, 2, argv));
	zassert_equal(1, sid_hal_free_fake.call_count);
}

ZTEST(sid_location_shell, test_send_levels_ordered)
{
	const char *argv[] = { "send_levels", "3", "4", "6" };

	zassert_equal(0, run_cmd(cmd_location_send_levels, 4, argv));
	zassert_equal(location_event_send_levels, sidewalk_event_send_fake.arg0_val);
	zassert_equal(sid_hal_free, sidewalk_event_send_fake.arg2_val);

	const struct location_send_levels_params *params = sidewalk_event_send_fake.arg1_val;

	zassert_equal(3, params->num_levels);
	zassert_equal(SID_LOCATION_EFFORT_L3, params->levels[0]);
	zassert_equal(SID_LOCATION_EFFORT_L4, params->levels[1]);
	zassert_equal(SID_LOCATION_EFFORT_L6, params->levels[2]);
	free(sidewalk_event_send_fake.arg1_val);
}

ZTEST(sid_location_shell, test_send_levels_max)
{
	const char *argv[] = { "send_levels", "1", "2", "3", "4", "5", "6" };

	zassert_equal(0, run_cmd(cmd_location_send_levels, 7, argv));

	const struct location_send_levels_params *params = sidewalk_event_send_fake.arg1_val;

	zassert_equal(SID_LOCATION_MAX_LEVELS, params->num_levels);
	for (int i = 0; i < SID_LOCATION_MAX_LEVELS; i++) {
		zassert_equal(i + 1, params->levels[i]);
	}
	free(sidewalk_event_send_fake.arg1_val);
}

ZTEST(sid_location_shell, test_send_levels_invalid)
{
	const char *no_args[] = { "send_levels" };
	const char *too_many[] = { "send_levels", "1", "2", "3", "4", "5", "6", "1" };
	const char *bad_level[] = { "send_levels", "3", "7" };
	const char *not_num[] = { "send_levels", "x" };

	zassert_equal(-EINVAL, run_cmd(cmd_location_send_levels, 1, no_args));
	zassert_equal(-EINVAL, run_cmd(cmd_location_send_levels, 8, too_many));
	zassert_equal(-EINVAL, run_cmd(cmd_location_send_levels, 3, bad_level));
	zassert_equal(-EINVAL, run_cmd(cmd_location_send_levels, 2, not_num));
	assert_nothing_sent();
	/* Allocated context of the rejected command must not leak. */
	zassert_equal(sid_hal_malloc_fake.call_count, sid_hal_free_fake.call_count);
}

ZTEST(sid_location_shell, test_send_pseudorange_valid)
{
	const char *argv[] = { "send_pseudorange", "00ff10Ab" };

	zassert_equal(0, run_cmd(cmd_location_send_pseudorange, 2, argv));
	zassert_equal(location_event_send_pseudorange, sidewalk_event_send_fake.arg0_val);

	const struct location_send_pseudorange_params *params = sidewalk_event_send_fake.arg1_val;
	const uint8_t expected[] = { 0x00, 0xff, 0x10, 0xab };

	zassert_equal(sizeof(expected), params->size);
	zassert_mem_equal(expected, params->data, sizeof(expected));
	free(sidewalk_event_send_fake.arg1_val);
}

ZTEST(sid_location_shell, test_send_pseudorange_max_size)
{
	char hex[2 * SID_LOCATION_MAX_PAYLOAD_SIZE + 1];

	memset(hex, 'a', sizeof(hex) - 1);
	hex[sizeof(hex) - 1] = '\0';
	const char *argv[] = { "send_pseudorange", hex };

	zassert_equal(0, run_cmd(cmd_location_send_pseudorange, 2, argv));

	const struct location_send_pseudorange_params *params = sidewalk_event_send_fake.arg1_val;

	zassert_equal(SID_LOCATION_MAX_PAYLOAD_SIZE, params->size);
	free(sidewalk_event_send_fake.arg1_val);
}

ZTEST(sid_location_shell, test_send_pseudorange_invalid)
{
	char too_long[2 * SID_LOCATION_MAX_PAYLOAD_SIZE + 3];

	memset(too_long, 'a', sizeof(too_long) - 1);
	too_long[sizeof(too_long) - 1] = '\0';

	const char *bad[] = { "", "abc", "zz", "0g", too_long };

	for (size_t i = 0; i < ARRAY_SIZE(bad); i++) {
		const char *argv[] = { "send_pseudorange", bad[i] };

		zassert_equal(-EINVAL, run_cmd(cmd_location_send_pseudorange, 2, argv), "case %zu",
			      i);
	}
	assert_nothing_sent();

	const char *no_args[] = { "send_pseudorange" };

	zassert_equal(-EINVAL, run_cmd(cmd_location_send_pseudorange, 1, no_args));
}

ZTEST(sid_location_shell, test_send_pseudorange_alloc_fail)
{
	const char *argv[] = { "send_pseudorange", "00" };

	sid_hal_malloc_fake.custom_fake = NULL;
	sid_hal_malloc_fake.return_val = NULL;
	zassert_equal(-ENOMEM, run_cmd(cmd_location_send_pseudorange, 2, argv));
	assert_nothing_sent();
}

ZTEST(sid_location_shell, test_send_pseudorange_send_fail_frees_ctx)
{
	const char *argv[] = { "send_pseudorange", "00" };

	sidewalk_event_send_fake.return_val = -EAGAIN;
	zassert_equal(-ENOMSG, run_cmd(cmd_location_send_pseudorange, 2, argv));
	zassert_equal(1, sid_hal_free_fake.call_count);
}

ZTEST(sid_location_shell, test_manage_effort_get)
{
	const char *argv[] = { "manage_effort" };

	zassert_equal(0, run_cmd(cmd_location_manage_effort, 1, argv));
	zassert_equal(location_event_manage_effort, sidewalk_event_send_fake.arg0_val);
	zassert_is_null(sidewalk_event_send_fake.arg1_val);
}

ZTEST(sid_location_shell, test_manage_effort_set)
{
	const char *off[] = { "manage_effort", "0" };
	const char *on[] = { "manage_effort", "1" };

	zassert_equal(0, run_cmd(cmd_location_manage_effort, 2, off));
	zassert_equal(0, *(uint32_t *)sidewalk_event_send_fake.arg1_val);
	free(sidewalk_event_send_fake.arg1_val);

	zassert_equal(0, run_cmd(cmd_location_manage_effort, 2, on));
	zassert_equal(1, *(uint32_t *)sidewalk_event_send_fake.arg1_val);
	free(sidewalk_event_send_fake.arg1_val);
}

ZTEST(sid_location_shell, test_manage_effort_invalid)
{
	const char *bad[] = { "2", "-1", "x", "" };

	for (size_t i = 0; i < ARRAY_SIZE(bad); i++) {
		const char *argv[] = { "manage_effort", bad[i] };

		zassert_equal(-EINVAL, run_cmd(cmd_location_manage_effort, 2, argv), "[%s]",
			      bad[i]);
	}
	assert_nothing_sent();
}

static void setup(void *fixture)
{
	ARG_UNUSED(fixture);
	SIDEWALK_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
	sid_hal_malloc_fake.custom_fake = malloc;
	sid_hal_free_fake.custom_fake = free;
}

ZTEST_SUITE(sid_location_shell, NULL, NULL, setup, NULL, NULL);
