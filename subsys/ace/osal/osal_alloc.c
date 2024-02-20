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

ace_status_t aceAlloc_init(void)
{
	return ACE_STATUS_OK;
}

ace_status_t aceAlloc_initWithAllocator(aceAlloc_allocator_t *allocators, size_t count)
{
	ARG_UNUSED(allocators);
	ARG_UNUSED(count);

	return ACE_STATUS_NOT_SUPPORTED;
}

ace_status_t aceAlloc_deInit(void)
{
	return ACE_STATUS_OK;
}

void *aceAlloc_alloc(aceModules_moduleId_t module_id, aceAlloc_bufferType_t buf_type, size_t size)
{
	ARG_UNUSED(module_id);
	ARG_UNUSED(buf_type);

	return sid_hal_malloc(size);
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
	ARG_UNUSED(module_id);
	ARG_UNUSED(buf_type);

	sid_hal_free(p);
}
