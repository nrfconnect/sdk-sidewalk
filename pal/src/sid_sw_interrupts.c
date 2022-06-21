/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_sw_interrupt.c
 *  @brief Sidewalk software interrupt.
 */

#include <sid_pal_swi_ifc.h>
#include <irq.h>
#if defined(CONFIG_CPU_CORTEX_M)
#include <arch/arm/aarch32/cortex_m/cmsis.h>
#endif

#define TEST_IRQ_LINE   (24)
#define TEST_IRQ_PRIO   (2)

static sid_pal_swi_cb_t swi_cb;

void sid_pal_swi_isr(void)
{
	if (swi_cb) {
		swi_cb();
	}
}

sid_error_t sid_pal_swi_init(sid_pal_swi_cb_t event_callback)
{
	if (!event_callback) {
		return SID_ERROR_NULL_POINTER;
	}

	swi_cb = event_callback;

	IRQ_CONNECT(TEST_IRQ_LINE, TEST_IRQ_PRIO, sid_pal_swi_isr, NULL, 0);
	irq_enable(TEST_IRQ_LINE);

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_trigger(void)
{
#if defined(CONFIG_CPU_CORTEX_M)
	NVIC_SetPendingIRQ(TEST_IRQ_LINE);
	return SID_ERROR_NONE;
#endif

	return SID_ERROR_NOSUPPORT;
}
