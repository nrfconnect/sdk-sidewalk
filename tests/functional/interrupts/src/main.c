/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_error.h>
#include <sid_pal_swi_ifc.h>
#include <string.h>

#define BUFFER_OUT_NO 100
#define BUFFER_SIZE 8
static char buffer_in[BUFFER_SIZE] = "test_me";
static char buffer_out[BUFFER_OUT_NO][BUFFER_SIZE];

static void swi_callback(void)
{
	static size_t buf_out_cnt = 0;

	(void)memcpy(buffer_out[buf_out_cnt++], buffer_in, BUFFER_SIZE);
}

void test_sid_pal_swi(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_init(swi_callback));

	for (int i = 0; i < BUFFER_OUT_NO; i++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_trigger());
	}

	for (int i = 0; i < BUFFER_OUT_NO; i++) {
		TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer_in, buffer_out[i], BUFFER_SIZE);
	}
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
