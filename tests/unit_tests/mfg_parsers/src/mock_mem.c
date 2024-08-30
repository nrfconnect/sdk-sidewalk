/**
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * 
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stdlib.h>
#include <zephyr/kernel.h>

K_HEAP_DEFINE(test_heap, KB(30));

void *sid_hal_malloc(size_t size)
{
	return k_heap_alloc(&test_heap, size, K_NO_WAIT);
}

void sid_hal_free(void *ptr)
{
	k_heap_free(&test_heap, ptr);
}
