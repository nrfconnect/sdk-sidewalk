/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <sid_pal_critical_region_ifc.h>
#include <sid_error.h>

#if defined(CONFIG_CPU_CORTEX_M)
#include <cmsis_core.h>
#else
#error "Architecture not supported"
#endif

#define TEST_IRQ (30)
#define TEST_IRQ_PRIO (2)
#define TEST_IRQ_FLAGS (0)

#define UNCHANGED 0
#define CHANGED 1
static volatile int resource;

static void irq_cb(void)
{
	resource = CHANGED;
}

static void trigger_irq(int irq)
{
	NVIC_SetPendingIRQ(irq);
}

void test_sid_pal_critical_region_with_timer(void)
{
	IRQ_CONNECT(TEST_IRQ, TEST_IRQ_PRIO, irq_cb, NULL, TEST_IRQ_FLAGS);
	irq_enable(TEST_IRQ);

	resource = UNCHANGED;
	trigger_irq(TEST_IRQ);
	TEST_ASSERT_EQUAL_UINT32(CHANGED, resource);

	sid_pal_enter_critical_region();
	resource = UNCHANGED;
	trigger_irq(TEST_IRQ);
	TEST_ASSERT_EQUAL_UINT32(UNCHANGED, resource);
	sid_pal_exit_critical_region();

	trigger_irq(TEST_IRQ);
	TEST_ASSERT_EQUAL_UINT32(CHANGED, resource);
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
