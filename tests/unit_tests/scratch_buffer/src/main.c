/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sbdt/scratch_buffer.h>

#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include <stdlib.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(void *, sid_hal_malloc, size_t);
FAKE_VOID_FUNC(sid_hal_free, void *);

static void *setup(void)
{
	RESET_FAKE(sid_hal_malloc);
	RESET_FAKE(sid_hal_free);

	FFF_RESET_HISTORY();

	sid_hal_malloc_fake.custom_fake = malloc;
	sid_hal_free_fake.custom_fake = free;

	return NULL;
}

ZTEST_SUITE(my_suite, NULL, setup, NULL, NULL, NULL);

ZTEST(my_suite, test_scratch_buffer_basic)
{
	uint32_t id = 1;
	size_t size = 64;
	void *p = NULL;
	uint8_t test[size];
	memset(test, 0xAA, size);

	scratch_buffer_init();

	p = scratch_buffer_create(id, size);
	zassert_equal(sid_hal_malloc_fake.call_count, 1);
	zassert_equal(sid_hal_malloc_fake.arg0_history[0], size);
	zassert_not_null(p);

	scratch_buffer_remove(id);
	zassert_equal(sid_hal_free_fake.call_count, 1);
}
