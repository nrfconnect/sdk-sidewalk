/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_sw_interrupt.c
 *  @brief Sidewalk software interrupt.
 */

#include <sid_pal_swi_ifc.h>
#include <kernel.h>

static sid_pal_swi_cb_t swi_cb;

sid_error_t sid_pal_swi_init(sid_pal_swi_cb_t event_callback)
{
	if (!event_callback) {
		return SID_ERROR_NULL_POINTER;
	}

	swi_cb = event_callback;

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_trigger(void)
{
	k_sched_lock();

	if (swi_cb) {
		swi_cb();
	}

	k_sched_unlock();

	return SID_ERROR_NONE;
}
