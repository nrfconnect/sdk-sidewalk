/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <sid_error.h>
#include <sid_pal_swi_ifc.h>
#include <string.h>

#define BUFFER_NO 10
#define BUFFER_SIZE 6

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

void test_sid_pal_swi(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_init(swi_callback));

	for (int i = 0; i < BUFFER_NO; i++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_trigger());
		TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer_in[i], buffer_out[i], BUFFER_SIZE);
	}
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

int main(void)
{
	return unity_main();
}
