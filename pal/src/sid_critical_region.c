/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_log.c
 *  @brief Log interface implementation.
 */

#include <sid_pal_critical_region_ifc.h>

#include <kernel.h>

int key;

void sid_pal_enter_critical_region()
{
	key = irq_lock();
}

void sid_pal_exit_critical_region()
{
	irq_unlock(key);
}
