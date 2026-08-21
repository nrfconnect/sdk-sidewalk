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

#include <stdbool.h>

#define TEST_IRQ (30)
#define TEST_IRQ_PRIO (2)
#define TEST_IRQ_FLAGS (0)

static volatile bool irq_fired;

static void irq_cb(const void *arg)
{
	irq_fired = true;
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
	bool irq_fired_in_critical_region;

	soc_irq_trigger(TEST_IRQ);
	zassert_true(irq_fired, "IRQ should fire");

	sid_pal_enter_critical_region();
	irq_fired = false;
	soc_irq_trigger(TEST_IRQ);
	irq_fired_in_critical_region = irq_fired;
	sid_pal_exit_critical_region();
	zassert_false(irq_fired_in_critical_region, "IRQ should not fire in critical region");

	soc_irq_trigger(TEST_IRQ);
	zassert_true(irq_fired, "IRQ should fire after critical region");
}

#ifdef CONFIG_THREAD_LOCAL_STORAGE

/*
 * For platforms that support thread local storage, verify that the critical region state is per-thread.
 * For ARCH_POSIX, the state is shared between threads and cannot uphold the isolation checked below.
 */

#define HOLDER_STACK_SIZE (1024)
#define HOLDER_TIMEOUT K_MSEC(100)

static K_THREAD_STACK_DEFINE(holder_stack, HOLDER_STACK_SIZE);
static struct k_thread holder_thread;
static K_SEM_DEFINE(holder_in_region, 0, 1);
static K_SEM_DEFINE(holder_release, 0, 1);

static volatile bool irq_fired_in_holder;
static bool holder_started;

static void holder_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	sid_pal_enter_critical_region();

	/* Notify the main thread that we have entered the critical region. */
	k_sem_give(&holder_in_region);

	/* Let the main thread enter and exit the critical region while this thread is still in the critical region. */
	k_sem_take(&holder_release, K_FOREVER);

	irq_fired = false;
	soc_irq_trigger(TEST_IRQ);
	irq_fired_in_holder = irq_fired;

	sid_pal_exit_critical_region();
}

ZTEST(sid_pal_suite, test_critical_region_is_thread_local)
{
	bool irq_fired_in_critical_region;
	bool irq_fired_after_critical_region;
	bool irq_fired_after_holder_left_critical_region;

	bool holder_entered_critical_region;
	bool holder_left_critical_region;

	/* Seeded so that a holder which never reaches its check fails below. */
	irq_fired_in_holder = true;

	k_thread_create(&holder_thread, holder_stack, HOLDER_STACK_SIZE, holder_fn, NULL, NULL,
			NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
	holder_started = true;

	holder_entered_critical_region = (0 == k_sem_take(&holder_in_region, HOLDER_TIMEOUT));

	/* The holder is blocked inside its own region. Entering here must still
	 * mask interrupts for this thread, rather than being swallowed by the
	 * holder's nesting count.
	 */
	sid_pal_enter_critical_region();
	irq_fired = false;
	soc_irq_trigger(TEST_IRQ);
	irq_fired_in_critical_region = irq_fired;
	sid_pal_exit_critical_region();

	irq_fired = false;
	soc_irq_trigger(TEST_IRQ);
	irq_fired_after_critical_region = irq_fired;

	k_sem_give(&holder_release);
	holder_left_critical_region = (0 == k_thread_join(&holder_thread, HOLDER_TIMEOUT));
	holder_started = false;

	irq_fired = false;
	soc_irq_trigger(TEST_IRQ);
	irq_fired_after_holder_left_critical_region = irq_fired;

	zassert_true(holder_entered_critical_region, "Holder thread did not enter the critical region");
	zassert_false(irq_fired_in_critical_region, "IRQ should not fire in critical region");
	zassert_true(irq_fired_after_critical_region, "IRQ should fire after critical region");
	zassert_true(holder_left_critical_region, "Holder thread did not finish");
	zassert_false(irq_fired_in_holder, "IRQ should not fire in holder");
	zassert_true(irq_fired_after_holder_left_critical_region, "IRQ should fire after holder left critical region");
}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */

static void *sid_pal_suite_setup(void)
{
	IRQ_CONNECT(TEST_IRQ, TEST_IRQ_PRIO, irq_cb, NULL, TEST_IRQ_FLAGS);
	irq_enable(TEST_IRQ);
	irq_fired = false;
	return NULL;
}

ZTEST_SUITE(sid_pal_suite, NULL, sid_pal_suite_setup, NULL, NULL, NULL);
