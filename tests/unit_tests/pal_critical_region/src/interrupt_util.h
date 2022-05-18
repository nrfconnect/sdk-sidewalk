/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#ifndef INTERRUPT_UTIL_H_
#define INTERRUPT_UTIL_H_

#if defined(CONFIG_ARCH_POSIX)
#include <irq_ctrl.h>

static inline void trigger_irq(int irq)
{
	hw_irq_ctrl_raise_im_from_sw(irq);
}

#elif defined(CONFIG_CPU_CORTEX_M)
#include <arch/arm/aarch32/cortex_m/cmsis.h>

static inline void trigger_irq(int irq)
{
#if defined(CONFIG_CPU_CORTEX_M0) || defined(CONFIG_CPU_CORTEX_M0PLUS) \
	|| defined(CONFIG_CPU_CORTEX_M1) || defined(CONFIG_ARMV6_M_ARMV8_M_BASELINE)
	/* QEMU does not simulate the STIR register: this is a workaround */
	NVIC_SetPendingIRQ(irq);
#else
	NVIC->STIR = irq;
#endif
}

#else
#error "Architecture not supported"
#endif

#endif /* INTERRUPT_UTIL_H_ */
