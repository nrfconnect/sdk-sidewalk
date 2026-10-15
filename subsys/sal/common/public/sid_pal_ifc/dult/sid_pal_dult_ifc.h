/*
 * Copyright 2024-2026 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_DULT_IFC_H
#define SID_PAL_DULT_IFC_H

/** @file
 *
 * @defgroup SID_PAL_DULT_IFC Sidewalk PAL DULT Interface
 * @brief Platform abstraction layer interface for Detect Unwanted Location Trackers (DULT).
 * @{
 * @ingroup SID_PAL_DULT_IFC
 */

#include <sid_detect_unwanted_location_tracker.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start DULT advertisement.
 *
 * Begins broadcasting DULT BLE advertisements. Must be called after
 * #sid_pal_dult_init.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_start(void);

/**
 * Stop DULT advertisement.
 *
 * Stops broadcasting DULT BLE advertisements.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_stop(void);

/**
 * Enter the identifier payload read state.
 *
 * Enables a non-owner to read the identifier payload over BLE GATT.
 * This should only be called when
 * #SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_IDENTIFIER_LOOKUP_BLE is set.
 *
 * @see sid_detect_unwanted_location_tracker_read_identifier_payload_enter
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_identifier_payload_enter(void);

/**
 * Update the motion detection status.
 *
 * Called in response to a #SID_MOTION_DETECTION_EVENT_TYPE_GET_MOTION_STATUS request
 * to report whether motion has been detected.
 *
 * @note #SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_MOTION_DETECTION must be set
 * in the accessory capabilities for this API to work.
 *
 * @param[in] status true if motion has been detected, false otherwise.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_update_motion_detection(bool status);

/**
 * Enable or disable motion detection as a source for non-owner find events.
 *
 * @note #SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_MOTION_DETECTION must be set
 * in the accessory capabilities for this API to work.
 *
 * @param[in] set true to enable motion detection, false to disable.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_set_motion_detection(bool set);

/**
 * Update the status of a non-owner find event.
 *
 * Reports whether the requested find event action was successfully performed.
 *
 * @param[in] status true if the find event action was honored, false otherwise.
 * @param[in] src    Source of the find event.
 * @param[in] type   Type of the non-owner find event.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_update_find_event_status(bool status,
                                                  enum sid_non_owner_find_event_src src,
                                                  enum sid_non_owner_find_event_type type);

/**
 * Set the current owner proximity state.
 *
 * Updates the DULT advertisement payload to reflect whether the device is
 * near the owner or separated.
 *
 * @param[in] state The owner proximity state to set.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_set_current_owner_proximity_state(enum sid_owner_proximity_state state);

/**
 * Set the current battery level.
 *
 * Updates the battery level reported in the DULT accessory information.
 *
 * @param[in] level The battery level to set.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_set_battery_level(enum sid_detect_unwanted_location_tracker_accessory_battery_level level);

/**
 * Initialize the DULT PAL layer.
 *
 * Sets up the platform layer with the accessory information and event callbacks
 * required for DULT operation. Must be called before #sid_pal_dult_start.
 *
 * @param[in] info      Pointer to the accessory information describing the device.
 * @param[in] callbacks Pointer to the event callbacks for DULT events.
 *                      Must remain valid for the lifetime of the DULT session.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_init(const struct sid_detect_unwanted_location_tracker_accessory_info *info,
                              struct sid_detect_unwanted_location_tracker_event_callbacks *callbacks);

/**
 * De-initialize the DULT PAL layer.
 *
 * Releases resources allocated during #sid_pal_dult_init. Must be called
 * after #sid_pal_dult_stop.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_dult_deinit(void);

/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_PAL_DULT_IFC_H */
