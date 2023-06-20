/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stdint.h>
#include <unity.h>

#include <sid_pal_critical_region_ifc.h>

#include <zephyr/irq.h>
#include <zephyr/kernel.h>

#include "fix_zassert_macro.h"

#include <zephyr/interrupt_util.h>

#define TEST_IRQ (24)
#define TEST_IRQ2 (25)
#define TEST_IRQ3 (26)
#define TEST_IRQ4 (27)
#define TEST_IRQ_PRIO (2)

#define DELAY_BEFORE_IRQ_CHECK_US (50)

void irq_cb(const void *arg)
{
	*(uint32_t *)arg += 1;
}

void test_sid_pal_critical_region(void)
{
	static uint32_t call_count = 0;
	static volatile uint32_t expected_callcount = 0;
	IRQ_CONNECT(TEST_IRQ, TEST_IRQ_PRIO, irq_cb, (const void *)&call_count, 0);
	irq_enable(TEST_IRQ);

	trigger_irq(TEST_IRQ);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL(expected_callcount, call_count);

	sid_pal_enter_critical_region();

	trigger_irq(TEST_IRQ);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	TEST_ASSERT_EQUAL(expected_callcount, call_count);
	sid_pal_exit_critical_region();

	expected_callcount++;
	TEST_ASSERT_EQUAL(expected_callcount, call_count);

	trigger_irq(TEST_IRQ);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL(expected_callcount, call_count);
}

void test_sid_pal_critical_region_multi_level(void)
{
	static uint32_t call_count = 0;
	static volatile uint32_t expected_callcount = 0;
	IRQ_CONNECT(TEST_IRQ2, TEST_IRQ_PRIO, irq_cb, (const void *)&call_count, 0);
	irq_enable(TEST_IRQ2);

	trigger_irq(TEST_IRQ2);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);

	sid_pal_enter_critical_region();
	sid_pal_enter_critical_region();

	trigger_irq(TEST_IRQ2);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
	sid_pal_exit_critical_region();
	sid_pal_exit_critical_region();
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
	trigger_irq(TEST_IRQ2);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
}

void test_sid_pal_critical_region_max_level(void)
{
	static uint32_t call_count = 0;
	static volatile uint32_t expected_callcount = 0;
	IRQ_CONNECT(TEST_IRQ3, TEST_IRQ_PRIO, irq_cb, (const void *)&call_count, 0);
	irq_enable(TEST_IRQ3);

	trigger_irq(TEST_IRQ3);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);

	sid_pal_enter_critical_region();

	trigger_irq(TEST_IRQ3);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);

	for (int i = 0; i < 7; i++) {
		sid_pal_enter_critical_region();
	}

	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);

	for (int i = 0; i < 8; i++) {
		TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
		sid_pal_exit_critical_region();
	}
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
	trigger_irq(TEST_IRQ3);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
}

void test_sid_pal_critical_region_multiple_irq_in_CR(void)
{
	static uint32_t call_count = 0;
	static volatile uint32_t expected_callcount = 0;
	IRQ_CONNECT(TEST_IRQ4, TEST_IRQ_PRIO, irq_cb, (const void *)&call_count, 0);
	irq_enable(TEST_IRQ4);

	trigger_irq(TEST_IRQ4);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);

	sid_pal_enter_critical_region();

	trigger_irq(TEST_IRQ4);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);

	for (int i = 0; i < 7; i++) {
		sid_pal_enter_critical_region();
		trigger_irq(TEST_IRQ4);
		k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	}

	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);

	for (int i = 0; i < 8; i++) {
		TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
		sid_pal_exit_critical_region();
	}
	// only one IRQ is queued
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
	trigger_irq(TEST_IRQ4);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	TEST_ASSERT_EQUAL_UINT32(expected_callcount, call_count);
}

extern int unity_main(void);

int main(void)
{
	return unity_main();
}
