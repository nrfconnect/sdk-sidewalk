/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stdint.h>
#include <zephyr/ztest.h>

#include <sid_pal_critical_region_ifc.h>

#include <zephyr/irq.h>
#include <zephyr/kernel.h>

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

ZTEST(pal_critical_region, test_sid_pal_critical_region)
{
	static uint32_t call_count = 0;
	static volatile uint32_t expected_callcount = 0;
	IRQ_CONNECT(TEST_IRQ, TEST_IRQ_PRIO, irq_cb, (const void *)&call_count, 0);
	irq_enable(TEST_IRQ);

	trigger_irq(TEST_IRQ);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	zassert_equal(expected_callcount, call_count);

	sid_pal_enter_critical_region();
	{
		trigger_irq(TEST_IRQ);
		k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
		zassert_equal(expected_callcount, call_count);
	}
	sid_pal_exit_critical_region();

	expected_callcount++;
	zassert_equal(expected_callcount, call_count);

	trigger_irq(TEST_IRQ);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	zassert_equal(expected_callcount, call_count);
}

ZTEST(pal_critical_region, test_sid_pal_critical_multiple_entry)
{
	static uint32_t call_count = 0;
	static volatile uint32_t expected_callcount = 0;
	IRQ_CONNECT(TEST_IRQ3, TEST_IRQ_PRIO, irq_cb, (const void *)&call_count, 0);
	irq_enable(TEST_IRQ3);

	trigger_irq(TEST_IRQ3);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	zassert_equal(expected_callcount, call_count);

	sid_pal_enter_critical_region();
	{
		trigger_irq(TEST_IRQ3);
		k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
		zassert_equal(expected_callcount, call_count);

		sid_pal_enter_critical_region();
		{
			zassert_equal(expected_callcount, call_count);
		}
		sid_pal_exit_critical_region();
		zassert_equal(expected_callcount, call_count);
	}
	sid_pal_exit_critical_region();

	expected_callcount++;
	zassert_equal(expected_callcount, call_count);
	trigger_irq(TEST_IRQ3);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	zassert_equal(expected_callcount, call_count);
}

ZTEST(pal_critical_region, test_sid_pal_critical_region_multiple_entry_multiple_irq)
{
	static uint32_t call_count = 0;
	static volatile uint32_t expected_callcount = 0;
	IRQ_CONNECT(TEST_IRQ4, TEST_IRQ_PRIO, irq_cb, (const void *)&call_count, 0);
	irq_enable(TEST_IRQ4);

	trigger_irq(TEST_IRQ4);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	zassert_equal(expected_callcount, call_count);

	sid_pal_enter_critical_region();
	{
		trigger_irq(TEST_IRQ4);
		k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
		zassert_equal(expected_callcount, call_count);
		sid_pal_enter_critical_region();
		{
			trigger_irq(TEST_IRQ4);
			k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
			zassert_equal(expected_callcount, call_count);
		}
		sid_pal_exit_critical_region();
		k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
		zassert_equal(expected_callcount, call_count);
	}
	sid_pal_exit_critical_region();

	// only one IRQ is queued
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	zassert_equal(expected_callcount, call_count);

	trigger_irq(TEST_IRQ4);
	k_busy_wait(DELAY_BEFORE_IRQ_CHECK_US);
	expected_callcount++;
	zassert_equal(expected_callcount, call_count);
}

ZTEST(pal_critical_region, test_sanity)
{
	zassert_true(true);
}

ZTEST_SUITE(pal_critical_region, NULL, NULL, NULL, NULL, NULL);
