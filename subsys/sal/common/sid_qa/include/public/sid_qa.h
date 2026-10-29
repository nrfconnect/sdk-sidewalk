/*
 * Copyright 2021-2026 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_QA_CLI_H
#define SID_QA_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_error.h>
#include <sid_api.h>
#include <sid_device_information.h>

enum qa_proc_wait_status {
    QA_PROC_NO_WAIT = 0,
    QA_PROC_WAIT,
};

typedef void (*pwr_meas_func_t)(void);
typedef void (*notify_blocked_func_t)(void);
typedef void (*notify_unblocked_func_t)(void);

typedef void (*reboot_func_t)(void);
typedef void (*set_sub_ghz_cfg_t)(struct sid_sub_ghz_links_config *sub_ghz_cfg);

struct sid_qa_callbacks {
    reboot_func_t reboot_cmd;
    set_sub_ghz_cfg_t set_sub_ghz_cfg;
    struct sid_device_info_config *device_info_cfg;
    /* Opaque GPS-assist PAL backend config (e.g. the Airoha serial-bus factory),
     * forwarded into sid_location_config::gps_assist_pal_config. NULL if unused. */
    void *gps_assist_pal_config;
};

struct sid_qa_pwr_meas_if {
    pwr_meas_func_t enter_func;
    pwr_meas_func_t exit_func;
    notify_blocked_func_t blocked_func;
    notify_unblocked_func_t unblocked_func;
};

/**
 * Initializes the QA CLI commands.
 */
sid_error_t sid_qa_init(struct sid_qa_callbacks *qa_callbacks);

/**
 * set qa config.
 */
void sid_qa_set_config(struct sid_config *config_p, struct sid_qa_pwr_meas_if *platform_ifc);

/**
 * Set extra qa_callbacks
 */
void sid_qa_set_extra_callbacks(struct sid_event_callbacks *callbacks);

/**
 *  QA process routine.
 */
void sid_qa_process(enum qa_proc_wait_status wait_status);

/**
 * Get Sidewalk Handle
 */
struct sid_handle *sid_qa_get_sid_handle(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
