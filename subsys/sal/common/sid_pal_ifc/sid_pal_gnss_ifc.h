/*
 * Copyright 2024-2025 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PAL_GNSS_IFC_H
#define SID_PAL_GNSS_IFC_H

#include <stdint.h>
#include <stddef.h>
#include <sid_error.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup sid_pal_gnss_ifc APIs used to interface with geolocation library
 * @{
 * @ingroup sid_pal_ifc
 * @details APIs used to interface with GNSS APIs on device.
 */

#define GNSS_MAX_PAYLOAD_SIZE 106

/**
 * Enum representing GNSS events emitted by the GNSS PAL
 */
enum sid_pal_gnss_events {
    SID_PAL_GNSS_INTERNAL,
    SID_PAL_GNSS_SCAN_COMPLETE,
    SID_PAL_GNSS_ALMANAC_DEMOD_UPDATE,
    SID_PAL_GNSS_LAST,
};

/**
 * GNSS config struct with callback to notify consumer.
 */
struct sid_pal_gnss_config {
    void (*on_gnss_event)(void *ctx, enum sid_pal_gnss_events event, uint32_t delay_ms);
    void *ctx;
};

/**
 * Struct representing a single payload to be used by GNSS location resolvers.
 */
struct sid_pal_gnss_payload {
    size_t size;
    uint8_t payload_data[GNSS_MAX_PAYLOAD_SIZE];
};

/**
 * Initialize GNSS for use with Sidewalk
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_gnss_init(struct sid_pal_gnss_config *config);

/**
 * Function to process GNSS PAL events.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_gnss_process_event(uint8_t event_id);

/**
 * Schedule a GNSS scan to occur after a delay.
 *
 * @param[in] scan_delay_s time delay in seconds
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_gnss_schedule_scan(uint32_t scan_delay_s);

/**
 * Cancel last scheduled GNSS scan.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_gnss_cancel_scan();

/**
 * Retrieve the current gnss payload to be sent to the cloud for position resolution. Requires
 * a scan to have completed.
 *
 * @param[in] gnss_scan_group pointer to the struct containing required GNSS payload of multiple
 * scans.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_gnss_get_scan_payload(struct sid_pal_gnss_payload *gnss_scan_group);

/**
 * Start the almanac demodulation service to update over satellite.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_gnss_alm_demod_start();

/**
 * deinitialize GNSS for use with Sidewalk
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_pal_gnss_deinit();

#ifdef __cplusplus
}
#endif
#endif   // SID_PAL_GNSS_IFC_H
