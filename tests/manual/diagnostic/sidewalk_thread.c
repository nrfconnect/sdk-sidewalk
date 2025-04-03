/*
 * Copyright 2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sidewalk_thread.h>

#include <sid_error.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_common_ifc.h>
#include <sid_pal_log_ifc.h>
#include <zephyr/storage/flash_map.h>

#include <sid_asd_cli.h>
#include <sid_diagnostics_cli.h>
// #include <sid_on_dev_cert_cli.h> TBD: on dev cert cli

#include <app_subGHz_config.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

#define LOG_FLUSH_SLEEP_PERIOD K_MSEC(5)
LOG_MODULE_REGISTER(sid_thread, CONFIG_SIDEWALK_LOG_LEVEL);

K_THREAD_STACK_DEFINE(sid_stack_area, CONFIG_SIDEWALK_THREAD_STACK_SIZE);
static struct k_thread sid_thread;
static k_tid_t sid_tid;

static void log_flush(void)
{
#ifndef CONFIG_LOG_MODE_MINIMAL
    /* Note: log_buffered_cnt is not supported in minimal log mode. */
    while (log_buffered_cnt()) {
        k_sleep(LOG_FLUSH_SLEEP_PERIOD);
    }
#endif
}

static void sidewalk_thread(void *context, void *u2, void *u3)
{
    ARG_UNUSED(u2);
    ARG_UNUSED(u3);
    ARG_UNUSED(context);

    LOG_INF("sid diagnostics application started...");

    /* Diagnostics CLI init needs to be inside thread as it may auto-start
     * radio test mode, subject to config stored in flash.
     */
    sid_cli_init();
    sid_diagnostics_cli_init();
    // sid_on_dev_cert_cli_init(); TBD: on dev cert cli

    while (1) {
        sid_cli_process();
        sid_pal_log_flush();
    }
}

void sidewalk_thread_enable(void)
{
    sid_error_t ret_code = sid_pal_storage_kv_init();
    if (ret_code != SID_ERROR_NONE) {
        LOG_ERR("Sidewalk KV store init failed err: %d", ret_code);
        return;
    }

    ret_code = sid_pal_crypto_init();
    if (ret_code != SID_ERROR_NONE) {
        LOG_ERR("Sidewalk Init Crypto HAL err: %d", ret_code);
        return;
    }

    const sid_pal_mfg_store_region_t nrf_mfg = {
        .addr_start = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage)),
        .addr_end = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage) + FLASH_AREA_SIZE(mfg_storage)),
    };
    sid_pal_mfg_store_init(nrf_mfg);

    platform_parameters_t platform_parameters = {
        .platform_init_parameters.radio_cfg = get_radio_cfg(),
    };
    ret_code = sid_pal_common_init(&platform_parameters.platform_init_parameters);
    if (ret_code != SID_ERROR_NONE) {
        LOG_ERR("Sidewalk Platform Init err: %d", ret_code);
        return;
    }

    sid_tid = k_thread_create(&sid_thread, sid_stack_area, K_THREAD_STACK_SIZEOF(sid_stack_area), sidewalk_thread, NULL,
                              NULL, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
    ARG_UNUSED(sid_tid);
    k_thread_name_set(&sid_thread, "sidewalk");
}
