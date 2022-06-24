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

#define SIDEWALK_SWI_STACK_SIZE 512
#define SIDEWALK_SWI_PRIORITY K_PRIO_COOP(CONFIG_NUM_COOP_PRIORITIES - 1)

K_THREAD_STACK_DEFINE(swi_stack_area, SIDEWALK_SWI_STACK_SIZE);

static struct k_work_q swi_work_q;
static struct k_work swi_work;
static sid_pal_swi_cb_t swi_cb;

static void sid_work_handler(struct k_work *work)
{
	if (swi_cb) {
		swi_cb();
	}
};

sid_error_t sid_pal_swi_init(sid_pal_swi_cb_t event_callback)
{
	if (!event_callback) {
		return SID_ERROR_NULL_POINTER;
	}
	swi_cb = event_callback;

	k_work_init(&swi_work, sid_work_handler);
	k_work_queue_init(&swi_work_q);

	k_work_queue_start(&swi_work_q, swi_stack_area,
			   K_THREAD_STACK_SIZEOF(swi_stack_area), SIDEWALK_SWI_PRIORITY,
			   NULL);

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_trigger(void)
{
	int ret;

	ret = k_work_submit_to_queue(&swi_work_q, &swi_work);
	if (ret < 0) {
		return SID_ERROR_GENERIC;
	}

	ret = k_work_queue_drain(&swi_work_q, false);
	if (ret < 0) {
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}
