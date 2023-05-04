/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>

#include <sid_pal_swi_ifc.h>

#define CHANGED         (1U)
#define UNCHANGED       (0U)
static volatile uint32_t resources[2];

static void test_cb(void)
{
	resources[0] = CHANGED;
}

static void test_cb_2(void)
{
	resources[1] = CHANGED;
}

void test_sid_pal_swi_init_negative(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_swi_init(NULL));
}

void test_sid_pal_swi_trigger(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_init(test_cb));
	resources[0] = UNCHANGED;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_trigger());
	TEST_ASSERT_EQUAL(CHANGED, resources[0]);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_init(test_cb_2));
	resources[0] = UNCHANGED;
	resources[1] = UNCHANGED;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_swi_trigger());
	TEST_ASSERT_EQUAL(CHANGED, resources[1]);
	TEST_ASSERT_EQUAL(UNCHANGED, resources[0]);
}

extern int unity_main(void);

int main(void)
{
	return unity_main();
}
