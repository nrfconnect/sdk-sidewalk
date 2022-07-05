/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file memory.c
 *  @brief HAL memory interface implementation.
 */

#include <sid_hal_memory_ifc.h>
#include <zephyr/kernel.h>

void *sid_hal_malloc(size_t size)
{
	return k_malloc(size);
}

void sid_hal_free(void *ptr)
{
	k_free(ptr);
}
