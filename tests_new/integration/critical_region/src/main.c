/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sid_pal_critical_region_ifc.h>
#include <sid_error.h>
#include <zephyr/ztest.h>
#include <zephyr/irq.h>

#if defined(CONFIG_SOC_FAMILY_NORDIC_NRF)
#include <cmsis_core.h>
#elif defined(CONFIG_SOC_POSIX)
#include <zephyr/arch/posix/posix_soc_if.h>
#else
#error "Architecture not supported"
#endif /* CONFIG_SOC */

#define TEST_IRQ (30)
#define TEST_IRQ_PRIO (2)
#define TEST_IRQ_FLAGS (0)

#define UNCHANGED 0
#define CHANGED 1
static volatile uint32_t resource;

static void irq_cb(const void *arg)
{
	resource = CHANGED;
}

static void soc_irq_enable(int irq)
{
#if defined(CONFIG_SOC_FAMILY_NORDIC_NRF)
    irq_enable(irq);
#elif defined(CONFIG_SOC_POSIX)
    posix_irq_enable(irq);
#endif /* CONFIG_SOC */
}

static void soc_irq_trigger(int irq)
{
#if defined(CONFIG_SOC_FAMILY_NORDIC_NRF)
    NVIC_SetPendingIRQ(irq);
#elif defined(CONFIG_SOC_POSIX)
    posix_sw_set_pending_IRQ(irq);
#endif /* CONFIG_SOC */
}

ZTEST(sid_pal_suite, test_critical_region_with_timer)
{
	IRQ_CONNECT(TEST_IRQ, TEST_IRQ_PRIO, irq_cb, NULL, TEST_IRQ_FLAGS);
	soc_irq_enable(TEST_IRQ);

	resource = UNCHANGED;
	soc_irq_trigger(TEST_IRQ);
	zassert_equal(resource, CHANGED, "IRQ should change resource");

	sid_pal_enter_critical_region();
	resource = UNCHANGED;
	soc_irq_trigger(TEST_IRQ);
	zassert_equal(resource, UNCHANGED, "Resource should not change in critical section");
	sid_pal_exit_critical_region();

	soc_irq_trigger(TEST_IRQ);
	zassert_equal(resource, CHANGED, "IRQ should change resource after critical section");
}

ZTEST_SUITE(sid_pal_suite, NULL, NULL, NULL, NULL, NULL);
