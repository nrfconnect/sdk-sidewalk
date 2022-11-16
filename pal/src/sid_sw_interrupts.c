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

#ifndef CONFIG_SIDEWALK_SWI_PRIORITY
	#error "CONFIG_SIDEWALK_SWI_PRIORITY must be defined"
#endif

#ifndef CONFIG_SIDEWALK_SWI_STACK_SIZE
	#error "CONFIG_SIDEWALK_SWI_STACK_SIZE must be defined"
#endif

static K_SEM_DEFINE(swi_trigger_sem, 0, 1);

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
	k_sem_give(&swi_trigger_sem);

	return SID_ERROR_NONE;
}

static void swi_task(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (1) {
		k_sem_take(&swi_trigger_sem, K_FOREVER);
		if (swi_cb) {
			swi_cb();
		}
	}
}

K_THREAD_DEFINE(swi_thread, CONFIG_SIDEWALK_SWI_STACK_SIZE, swi_task, NULL, NULL, NULL,
		K_PRIO_COOP(CONFIG_SIDEWALK_SWI_PRIORITY), 0, 0);
