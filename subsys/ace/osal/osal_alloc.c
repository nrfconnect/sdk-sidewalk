/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <osal_alloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/math_extras.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ace, CONFIG_SIDEWALK_LOG_LEVEL);

K_HEAP_DEFINE(sid_heap, CONFIG_SIDEWALK_HEAP_SIZE);

#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
#define HEAP_USAGE_WARNING_THRESHOLD (7 * CONFIG_SIDEWALK_HEAP_SIZE / 10)

static void heap_alloc_stats(struct sys_heap *p_heap, size_t mem_to_alloc)
{
	struct sys_heap_runtime_stats stat;

	sys_heap_runtime_stats_get(p_heap, &stat);
	if (mem_to_alloc > stat.free_bytes) {
		LOG_ERR("Not heap left. Alloc size: %u, free: %u", mem_to_alloc, stat.free_bytes);
	}

	if (stat.max_allocated_bytes > HEAP_USAGE_WARNING_THRESHOLD) {
		LOG_WRN("Max heap usage %u", stat.max_allocated_bytes);
	}

	if (CONFIG_SIDEWALK_HEAP_SIZE > 0) {
		size_t usage = stat.allocated_bytes + mem_to_alloc;
		LOG_DBG("heap usage %4u [%2d%%]", usage, (100 * usage / CONFIG_SIDEWALK_HEAP_SIZE));
	}
}

#endif  /* CONFIG_SYS_HEAP_RUNTIME_STATS */

ace_status_t aceAlloc_init(void)
{
	return ACE_STATUS_OK;
}

ace_status_t aceAlloc_initWithAllocator(aceAlloc_allocator_t *allocators,
					size_t count)
{
	ARG_UNUSED(allocators);
	ARG_UNUSED(count);

	return ACE_STATUS_NOT_SUPPORTED;
}

ace_status_t aceAlloc_deInit(void)
{
	return ACE_STATUS_OK;
}

void *aceAlloc_alloc(aceModules_moduleId_t module_id,
		     aceAlloc_bufferType_t buf_type, size_t size)
{
	ARG_UNUSED(module_id);
	ARG_UNUSED(buf_type);

#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
	heap_alloc_stats(&sid_heap.heap, size);
#endif

	return k_heap_alloc(&sid_heap, size, K_NO_WAIT);
}

void *aceAlloc_calloc(aceModules_moduleId_t module_id,
		      aceAlloc_bufferType_t buf_type, size_t nmemb,
		      size_t size)
{
	size_t bounds;

	if (size_mul_overflow(nmemb, size, &bounds)) {
		return NULL;
	}

	void *p = aceAlloc_alloc(module_id, buf_type, bounds);

	if (p) {
		(void)memset(p, 0, bounds);
	}

	return p;
}

void aceAlloc_free(aceModules_moduleId_t module_id,
		   aceAlloc_bufferType_t buf_type, void *p)
{
	ARG_UNUSED(module_id);
	ARG_UNUSED(buf_type);

	k_heap_free(&sid_heap, p);
}
