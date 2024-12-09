/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/kernel.h>
#include <zephyr/ztest_assert.h>
#include <zephyr/ztest.h>
#include <sid_error.h>
#include <sid_pal_swi_ifc.h>
#include <string.h>

#define BUFFER_NO 10
#define BUFFER_SIZE 10

static char buffer_in[BUFFER_NO][BUFFER_SIZE] = {
	"test_0", "test_1", "test_2", "test_3", "test_4",
	"test_5", "test_6", "test_7", "test_8", "test_9",
};
static char buffer_out[BUFFER_NO][BUFFER_SIZE];

static void swi_callback(void)
{
	static size_t cnt = 0;

	(void)memcpy(buffer_out[cnt], buffer_in[cnt], BUFFER_SIZE);

	cnt++;
	if (cnt >= BUFFER_NO) {
		cnt = 0;
	}
}

ZTEST(interrupts, test_sid_pal_swi)
{
	zassert_equal(SID_ERROR_NONE, sid_pal_swi_init());
	zassert_equal(SID_ERROR_NONE, sid_pal_swi_start(swi_callback));

	for (int i = 0; i < BUFFER_NO; i++) {
		zassert_equal(SID_ERROR_NONE, sid_pal_swi_trigger());
		zassert_mem_equal(buffer_in[i], buffer_out[i], BUFFER_SIZE, "in %s, out %s",
				  buffer_in[i], buffer_out[i]);
	}

	zassert_equal(SID_ERROR_NONE, sid_pal_swi_stop());
	zassert_equal(SID_ERROR_NONE, sid_pal_swi_deinit());
}

ZTEST_SUITE(interrupts, NULL, NULL, NULL, NULL, NULL);
