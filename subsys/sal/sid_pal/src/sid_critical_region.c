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

static atomic_t count = ATOMIC_INIT(0);
static unsigned int key = 0;

void sid_pal_enter_critical_region()
{
	const unsigned int prev_val = atomic_add(&count, 1);

	if (prev_val == 0) {
		key = irq_lock();
	}

	assert(prev_val <= CONFIG_SIDEWALK_CRITICAL_REGION_RE_ENTRY_MAX);
}

void sid_pal_exit_critical_region()
{
	const unsigned int prev_val = atomic_sub(&count, 1);

	assert(prev_val > 0);

	if (prev_val == 1) {
		irq_unlock(key);
	}
}
