/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <osal_alloc.h>
#include <sid_hal_memory_ifc.h>
#include <zephyr/sys/math_extras.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ace, CONFIG_SIDEWALK_LOG_LEVEL);

static void *malloc_wrap(size_t size, void *ctx)
{
	ARG_UNUSED(ctx);

	return sid_hal_malloc(size);
}

static void free_wrap(void *p, void *ctx)
{
	ARG_UNUSED(ctx);

	sid_hal_free(p);
}

static const aceAlloc_allocator_t default_allocator = {
	.alloc = malloc_wrap,
	.free = free_wrap,
};

static aceAlloc_allocator_t allocator = default_allocator;

ace_status_t aceAlloc_init(void)
{
	allocator = default_allocator;
	return ACE_STATUS_OK;
}

ace_status_t aceAlloc_initWithAllocator(aceAlloc_allocator_t *allocators, size_t count)
{
	if (count != 1) {
		return ACE_STATUS_NOT_SUPPORTED;
	}
	allocator = *allocators;
	return ACE_STATUS_OK;
}

ace_status_t aceAlloc_deInit(void)
{
	return ACE_STATUS_OK;
}

void *aceAlloc_alloc(aceModules_moduleId_t module_id, aceAlloc_bufferType_t buf_type, size_t size)
{
	return allocator.alloc(size, allocator.ctx);
}

void *aceAlloc_calloc(aceModules_moduleId_t module_id, aceAlloc_bufferType_t buf_type, size_t nmemb,
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

void aceAlloc_free(aceModules_moduleId_t module_id, aceAlloc_bufferType_t buf_type, void *p)
{
	return allocator.free(p, allocator.ctx);
}
