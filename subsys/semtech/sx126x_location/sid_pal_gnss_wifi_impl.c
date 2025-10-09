/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#include <sid_pal_gnss_ifc.h>
#include <sid_pal_wifi_ifc.h>
#include <sid_error.h>
#include <zephyr/toolchain.h>

/* GNSS PAL Implementation - All functions return NOT_SUPPORTED */

sid_error_t sid_pal_gnss_init(struct sid_pal_gnss_config *config)
{
    ARG_UNUSED(config);
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_process_event(uint8_t event_id)
{
    ARG_UNUSED(event_id);
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_schedule_scan(uint32_t scan_delay_s)
{
    ARG_UNUSED(scan_delay_s);
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_cancel_scan(void)
{
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_get_scan_payload(struct sid_pal_gnss_payload *gnss_scan_group)
{
    ARG_UNUSED(gnss_scan_group);
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_alm_demod_start(void)
{
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_deinit(void)
{
    return SID_ERROR_NOSUPPORT;
}

/* WiFi PAL Implementation - All functions return NOT_SUPPORTED */

sid_error_t sid_pal_wifi_init(struct sid_pal_wifi_config *config)
{
    ARG_UNUSED(config);
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_process_event(uint8_t event_id)
{
    ARG_UNUSED(event_id);
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_deinit(void)
{
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_schedule_scan(uint32_t scan_delay_s)
{
    ARG_UNUSED(scan_delay_s);
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_cancel_scan(void)
{
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_get_scan_payload(struct sid_pal_wifi_payload *wifi_scan_result)
{
    ARG_UNUSED(wifi_scan_result);
    return SID_ERROR_NOSUPPORT;
}
