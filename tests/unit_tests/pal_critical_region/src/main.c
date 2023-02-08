/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>

#include <sid_pal_critical_region_ifc.h>

#include <zephyr/irq.h>
#include <zephyr/kernel.h>

#include "fix_zassert_macro.h"

#include <zephyr/interrupt_util.h>

#define TEST_IRQ        (24)
#define TEST_IRQ_PRIO   (2)
#define CHANGED         (1U)
#define UNCHANGED       (0U)

static volatile uint32_t resource;

void irq_cb(const void *arg)
{
	resource = CHANGED;
}

void test_sid_pal_critical_region(void)
{
	IRQ_CONNECT(TEST_IRQ, TEST_IRQ_PRIO, irq_cb, NULL, 0);
	irq_enable(TEST_IRQ);

	trigger_irq(TEST_IRQ);
	TEST_ASSERT_EQUAL(CHANGED, resource);

	sid_pal_enter_critical_region();
	resource = UNCHANGED;
	trigger_irq(TEST_IRQ);
	TEST_ASSERT_EQUAL(UNCHANGED, resource);
	sid_pal_exit_critical_region();

	trigger_irq(TEST_IRQ);
	TEST_ASSERT_EQUAL(CHANGED, resource);
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
