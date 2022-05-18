/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_critical_region.c
 *  @brief Critical region interface implementation.
 */

#include <sid_pal_critical_region_ifc.h>

#include <kernel.h>

static unsigned int key;

void sid_pal_enter_critical_region()
{
	key = irq_lock();
}

void sid_pal_exit_critical_region()
{
	irq_unlock(key);
}
