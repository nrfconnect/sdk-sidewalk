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

#ifndef SID_LOCATION_IFC_H
#define SID_LOCATION_IFC_H
/// @cond sid_ifc_ep_en
/** @file
 *
 * @defgroup SIDEWALK_API Sidewalk API
 * @brief API to request and send location information.
 * @{
 * @ingroup  SIDEWALK_API
 */

#include <sid_error.h>

#include <stdint.h>
#include <stdbool.h>
#include <sid_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Forward declare
 */
struct sid_handle;

/**
 * Enum to represent the location methods available to use on device
 */
enum sid_location_method_type {
    SID_LOCATION_METHOD_BLE_GATEWAY = (1u << 0),
    SID_LOCATION_METHOD_GNSS = (1u << 1),
    SID_LOCATION_METHOD_WIFI = (1u << 2),
    SID_LOCATION_METHOD_ALL = SID_LOCATION_METHOD_BLE_GATEWAY | SID_LOCATION_METHOD_WIFI | SID_LOCATION_METHOD_GNSS
};

/**
 * Enum to represent the location effort modes on device
 */
enum sid_location_effort_mode {
    SID_LOCATION_EFFORT_L1 = 1,
    SID_LOCATION_EFFORT_L2,
    SID_LOCATION_EFFORT_L3,
    SID_LOCATION_EFFORT_L4,
    SID_LOCATION_EFFORT_INVALID,
    SID_LOCATION_EFFORT_DEFAULT
};

/**
 * Event types of the sid location library
 */
enum sid_location_status {
    SID_LOCATION_LVL1_READY,         // To be used with timesync connections
    SID_LOCATION_LVL1_UNAVAILABLE,   // To be used with timesync connections
    SID_LOCATION_SEND_DONE,
    SID_LOCATION_SCAN_DONE
};

enum sid_location_run_type {
    SID_LOCATION_SEND_ONLY = (1u << 0),
    SID_LOCATION_SCAN_ONLY = (1u << 1),
    SID_LOCATION_SCAN_AND_SEND = SID_LOCATION_SEND_ONLY | SID_LOCATION_SCAN_ONLY,
};

struct sid_location_run_config {
    enum sid_location_effort_mode mode;
    enum sid_location_run_type type;
    uint8_t *buffer;
    size_t size;
};

#define SID_LOCATION_MAX_PAYLOAD_SIZE 106

/**
 * Struct with return information on location uplink attempt
 */
struct sid_location_result {
    enum sid_location_status status;
    sid_error_t err;
    enum sid_location_effort_mode mode;
    enum sid_link_type link;
    uint8_t payload[SID_LOCATION_MAX_PAYLOAD_SIZE];
    size_t size;
};

/**
 * The set of callback(s) a user can register through sid_location_init().
 */
struct sid_location_event_callback {
    /**
     * Callback invoked when location start/send is complete.
     *
     * @param[in] handle A pointer to the handle returned by sid_init().
     * @param[in] result struct containing info on the attempted uplink.
     */
    void (*on_update)(const struct sid_location_result *const result, void *context);
    /**
     * Context that user sid_location API can use
     */
    void *context;
};

/**
 * Struct to configure the timeout (ms) for stepping down location levels.
 * In the case of L4->L3 or L3->L2, the timeout will start after the scan completes.
 */
struct sid_location_stepdown_timeouts {
    uint32_t l4_to_l3;
    uint32_t l3_to_l2;
    uint32_t l2_to_l1;
};

/**
 * Struct to configure the timeout (ms) for total time to retry and total attempts
 * when fragmentation is used to uplink scan data (GNSS or WiFi) over LoRa.
 */
struct sid_location_fragmentation_retries {
    uint32_t timeout_ms;
    uint8_t max_retries;
};

/**
 * Struct representing location initialization configurations.
 */
struct sid_location_config {
    enum sid_location_method_type sid_location_type_mask;
    struct sid_location_event_callback callbacks;
    enum sid_location_effort_mode max_effort;
    bool manage_effort;
    struct sid_location_stepdown_timeouts stepdowns;
    struct sid_location_fragmentation_retries fragmentation;
};

/**
 * Initialize sid location library
 *
 * This API will configure and initialize one or both of the supported
 * location libraries available on device.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] config Location configuration structure
 *
 * @returns #SID_ERROR_NONE in case of success
 **/
sid_error_t sid_location_init(struct sid_handle *handle, struct sid_location_config *config);

/**
 * Deinitialize sid location library
 *
 * This API will deinitialize the enabled location libraries.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 **/
sid_error_t sid_location_deinit(struct sid_handle *handle);

/**
 * API to scan and send available location information to the cloud to resolve
 * coordinates. If effort mode is NOT managed, SID_LOCATION_EFFORT_DEFAULT cannot
 * be used.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] config struct containing the configuration for the location request
 * @param[in] delay_s delay to start scan (if applicable) and uplink data
 *
 * @returns #SID_ERROR_NONE in case of success
 **/
sid_error_t sid_location_run(struct sid_handle *handle, struct sid_location_run_config *config, uint32_t delay_s);

/**
 * API to set the maximum effort mode allowed (ex. if the device is in a low power mode we only want to
 * send messages via mode 1)
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] mode maximum effort mode allowed.
 *
 **/

sid_error_t sid_location_set_max_mode(struct sid_handle *handle, enum sid_location_effort_mode mode);

/**
 * API to retrieve the location mode maintained by the location library
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns the current sid location effort mode.
 **/
enum sid_location_effort_mode sid_location_get_effort_mode(struct sid_handle *handle);
/**
 * API to update the GNSS almanac
 *
 * @param[in] handle A pointer to the handle returned by sid_init().
 * @param[in] almanac_buffer a pointer to a buffer containing the almanac.
 * @param[in] almanac_size size of the almanac buffer.
 *
 * @returns #SID_ERROR_NONE in case of success
 **/
sid_error_t sid_location_update_almanac(struct sid_handle *handle, uint8_t *almanac_buffer, size_t almanac_size);

#ifdef __cplusplus
}
#endif

#endif   // SID_LOCATION_IFC_H
