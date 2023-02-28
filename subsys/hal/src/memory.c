/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_hal_memory_ifc.h>
#include <sid_pal_critical_region_ifc.h>
#include <sid_memory_pool.h>
#include <sid_pal_log_ifc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hal_memory, CONFIG_SIDEWALK_LOG_LEVEL);

#ifndef SID_HAL_PROTOCOL_MEMORY_SZ
#if GW_SUPPORT
#define SID_HAL_PROTOCOL_MEMORY_SZ 5120
#else
#define SID_HAL_PROTOCOL_MEMORY_SZ 1024
#endif // GW_SUPPORT
#endif // SID_HAL_PROTOCOL_MEMORY_SZ

static uint8_t mem_pool[SID_HAL_PROTOCOL_MEMORY_SZ] __attribute__((aligned));
struct sid_memory_pool *mem_pool_handle = NULL;

void *sid_hal_malloc(size_t size)
{
    if (mem_pool_handle == NULL) {
        struct sid_memory_pool_config mem_config = {.size = sizeof(mem_pool), .buffer = mem_pool};
        int rv = 0;
        if ((rv = sid_memory_pool_init(&mem_pool_handle, &mem_config)) != SID_ERROR_NONE) {
            LOG_INF("%s: pool init failed (%d)", __func__, rv);
            return NULL;
        }
    }
    void *ptr = NULL;
    sid_pal_enter_critical_region();
    ptr = sid_memory_pool_allocate(mem_pool_handle, size);
    sid_pal_exit_critical_region();
    return ptr;
}

void sid_hal_free(void *ptr)
{
    if (!ptr) {
        return;
    }
    sid_pal_enter_critical_region();
    sid_memory_pool_free(mem_pool_handle, ptr);
    sid_pal_exit_critical_region();
}
