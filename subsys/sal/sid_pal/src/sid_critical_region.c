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

/* The kernel saves and restores BASEPRI per thread across a context switch, so
 * the nesting depth and the saved key have to be per-thread as well.
 * Otherwise, if thread A blocked inside the critical region, and let thread B
 * increase the nesting from 1 to 2, thread A could exit the critical region
 * without restoring the mask, which would leave interrupts masked permanently.

 * Architectures without thread local storage, including ARCH_POSIX used for
 * unit tests, keep the shared state and stay exposed to that.
 */
static SID_PAL_TLS uint32_t nesting;
static SID_PAL_TLS unsigned int key;

void sid_pal_enter_critical_region()
{
	const unsigned int local_key = irq_lock();

	if (nesting++ == 0U) {
		key = local_key;
	}

	assert(nesting <= CONFIG_SIDEWALK_CRITICAL_REGION_RE_ENTRY_MAX);
}

void sid_pal_exit_critical_region()
{
	assert(nesting > 0U);

	if (nesting == 0U) {
		return;
	}

	if (--nesting == 0U) {
		irq_unlock(key);
	}
}
