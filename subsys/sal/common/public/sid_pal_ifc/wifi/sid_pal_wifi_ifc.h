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

#ifndef SID_PAL_WIFI_IFC_H
#define SID_PAL_WIFI_IFC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sid_error.h>

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup sid_pal_wifi_ifc APIs used to interface with geolocation library
 * @{
 * @ingroup sid_pal_ifc
 * @details APIs used to interface with wifi APIs on device.
 */

#define SID_WIFI_MAC_ADDRESS_LENGTH 6
#define SID_WIFI_MAX_RESULTS 8
#define SID_WIFI_RSSI_SIZE 1

/**
 * Enum representing WiFi events emitted by the WiFi PAL
 *
 * @note Events start from 10 to avoid potential conflicts
 * with other event types (ex. GNSS).
 */
enum sid_pal_wifi_events {
    SID_PAL_WIFI_INTERNAL = 10,
    SID_PAL_WIFI_SCAN_COMPLETE,
    SID_PAL_WIFI_LAST,
};

/**
 * WiFi config struct with callback to notify consumer.
 */
struct sid_pal_wifi_config {
    void (*on_wifi_event)(void *ctx, enum sid_pal_wifi_events event, uint32_t delay_ms);
    void *ctx;
};

/**
 * Struct representing a WiFi access point's RSSI and MAC address.
 */
struct sid_pal_wifi_ap {
    uint8_t rssi;
    uint8_t mac[SID_WIFI_MAC_ADDRESS_LENGTH];
};

/**
 * Struct representing a single payload to be used by WiFi access
 * point location resolvers.
 */
struct sid_pal_wifi_payload {
    uint8_t nbr_results;
    struct sid_pal_wifi_ap results[SID_WIFI_MAX_RESULTS];
};

/**
 * Initialize WiFi for use with Sidewalk
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_wifi_init(struct sid_pal_wifi_config *config);

/**
 * Function to process WiFi PAL events.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_wifi_process_event(uint8_t event_id);

/**
 * Deinitialize WiFi for use with Sidewalk
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_wifi_deinit();

/**
 * Schedule a GNSS scan to occur after a delay.
 *
 * @param[in] scan_delay_s time delay in seconds
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_wifi_schedule_scan(uint32_t scan_delay_s);

/**
 * Cancel last scheduled WiFi scan.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_wifi_cancel_scan();

/**
 * Retrieve the last WiFi scan results. Requires a scan to have completed.
 *
 * @param[in] wifi_scan_result pointer to scan result struct to populate.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_wifi_get_scan_payload(struct sid_pal_wifi_payload *wifi_scan_result);

#ifdef __cplusplus
}
#endif
#endif   // SID_PAL_WIFI_IFC_H
