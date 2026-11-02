/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_critical_region.c
 *  @brief Critical region interface implementation.
 */

#include <sid_pal_critical_region_ifc.h>
#include <assert.h>

#include <zephyr/kernel.h>

#ifdef CONFIG_THREAD_LOCAL_STORAGE
#define SID_PAL_TLS Z_THREAD_LOCAL
#else
#define SID_PAL_TLS
#endif

struct critical_region_state {
	uint32_t nesting;
	unsigned int key;
};

/* The kernel saves and restores BASEPRI per thread across a context switch, so
 * the nesting depth and the saved key have to be per-thread as well.
 * Otherwise, if thread A blocked inside the critical region, and let thread B
 * increase the nesting from 1 to 2, thread A could exit the critical region
 * without restoring the mask, which would leave interrupts masked permanently.
 *
 * Architectures without thread local storage, including ARCH_POSIX used for
 * unit tests, keep the shared state and stay exposed to that.
 */
static SID_PAL_TLS struct critical_region_state thread_state;

/* An ISR sees the thread-local storage of the thread it preempted. On Cortex-M
 * the context switch is not atomic, so an ISR can run on top of a thread that
 * has already unmasked interrupts on its way to block inside the critical
 * region. Sharing that thread's nesting would make the ISR skip irq_unlock()
 * and return with interrupts masked. While an ISR holds the critical region,
 * no other ISR that may enter it can preempt, so a single state is enough.
 */
static struct critical_region_state isr_state;

static struct critical_region_state *current_state(void)
{
	return k_is_in_isr() ? &isr_state : &thread_state;
}

void sid_pal_enter_critical_region()
{
	const unsigned int local_key = irq_lock();
	struct critical_region_state *state = current_state();

	if (state->nesting++ == 0U) {
		state->key = local_key;
	}

	assert(state->nesting <= CONFIG_SIDEWALK_CRITICAL_REGION_RE_ENTRY_MAX);
}

void sid_pal_exit_critical_region()
{
	struct critical_region_state *state = current_state();

	assert(state->nesting > 0U);

	if (state->nesting == 0U) {
		return;
	}

	if (--state->nesting == 0U) {
		irq_unlock(state->key);
	}
}
