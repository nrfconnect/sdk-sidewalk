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

K_MUTEX_DEFINE(sid_mutex);

void sid_pal_enter_critical_region()
{
	k_mutex_lock(&sid_mutex, K_FOREVER);
}

void sid_pal_exit_critical_region()
{
	k_mutex_unlock(&sid_mutex);
}
