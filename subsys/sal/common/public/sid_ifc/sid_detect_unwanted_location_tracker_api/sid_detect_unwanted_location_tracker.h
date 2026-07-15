/*
 * Copyright 2024-2025 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SID_DETECT_UNWANTED_LOCATION_TRACKER_H
#define SID_DETECT_UNWANTED_LOCATION_TRACKER_H

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup SIDEWALK_DETECT_UNWANTED_LOCATION_TRACKER_API Sidewalk Detect Unwanted Location Tracker API
 * @brief API for using Detecting Unwanted Location Trackers
 * @{
 * @ingroup SIDEWALK_DETECT_UNWANTED_LOCATION_TRACKER_API
 */

#include <sid_error.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sid_handle;

enum {
    /** Max size of Product Data Accessory Information  */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_PRODUCT_DATA_MAX_SIZE = 8,
    /** Max size of Manufacturer Data Accessory Information  */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_MANUFACTURER_NAME_MAX_SIZE = 64,
    /** Max size of Model Name Accessory Information  */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_MODEL_NAME_MAX_SIZE = 64,
    /** Max size of the Identifier Payload */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_IDENTIFIER_PAYLOAD_MAX_SIZE = 32,
    /** Time Interval to check if device is near owner (30 min)*/
    SID_DETECT_UNWANTED_LOCATION_TRACKER_NON_OWNER_CHECK_TIME_SEC = (30 * 60),
};

/**
 * Describes the mode in which DULT Advertisement is enabled
 */
enum sid_detect_unwanted_location_tracker_advertisement_mode {
    /** DULT Advertisement will be shown only in separated mode */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ADVERTISEMENT_SEPARATED_MODE = 0,
    /** DULT Advertisement will be shown in both near and separated mode */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ADVERTISEMENT_NEAR_SEPARATED_MODE = 1,
};

/**
 * Describes the Owner Proximity State
 */
enum sid_owner_proximity_state {
    /** The device is separated from the owner */
    SID_OWNER_PROXIMITY_STATE_SEPARATED = 0,
    /** The device is in near proximity to the owner */
    SID_OWNER_PROXIMITY_STATE_NEAR = 1,
    /** Invalid */
    SID_OWNER_PROXIMITY_STATE_INVALID,
};

/**
 * Describes the Non-Owner hardware to trigger for
 * locating the device
 */
enum sid_non_owner_find_event_type {
    /** Trigger sound hardware to locate device */
    SID_NON_OWNER_FIND_EVENT_TYPE_SOUND = 1,
    /** Trigger lights to locate device UNSUPPORTED*/
    SID_NON_OWNER_FIND_EVENT_TYPE_LIGHTS = 2,
    /** Trigger haptic feedback to locate device UNSUPPORTED*/
    SID_NON_OWNER_FIND_EVENT_TYPE_HAPTIC = 3,
};

/**
 * Describes the source of the Non-Owner find event
 * @see #sid_non_owner_find_event_type
 */
enum sid_non_owner_find_event_src {
    /** Event was generated from BLE GATT opcode */
    SID_NON_OWNER_FIND_EVENT_SRC_BT_GATT = 1,
    /** Event was generated from the motion detection */
    SID_NON_OWNER_FIND_EVENT_SRC_MOTION_DETECTION = 2,
    /** Event was generated from an external API */
    SID_NON_OWNER_FIND_EVENT_SRC_EXTERNAL = 3,
    /** Number of events */
    SID_NON_OWNER_FIND_EVENT_SRC_COUNT,
};

/**
 * Describes the control action to take for the
 * Non-Onwer find event
 */
enum sid_non_owner_find_event_action {
    /** Start the non-owner find hardware */
    SID_NON_OWNER_FIND_EVENT_ACTION_START = 1,
    /** Stop the non-owner find hardware */
    SID_NON_OWNER_FIND_EVENT_ACTION_STOP = 2,
};

/**
 * Describes the Non-Onwer Find Event
 */
struct sid_non_owner_find_event {
    /** Type of the event */
    enum sid_non_owner_find_event_type type;
    /** Source of the event */
    enum sid_non_owner_find_event_src src;
    /** Acton that needs to be taken for the event */
    enum sid_non_owner_find_event_action action;
};

/**
 * Describes the action of motion detection event
 */
enum sid_motion_detection_event_action {
    /** Start motion detection */
    SID_MOTION_DETECTION_EVENT_TYPE_START = 1,
    /** Stop motion detection */
    SID_MOTION_DETECTION_EVENT_TYPE_STOP = 2,
    /** Retrieve teh state of motion detection
     * @see #sid_detect_unwanted_location_tracker_update_motion_detect
     */
    SID_MOTION_DETECTION_EVENT_TYPE_GET_MOTION_STATUS = 3,
};

/**
 * Describes motion detection event
 */
struct sid_motion_detection_event {
    /** Action that needs to be taken for the motion detection event */
    enum sid_motion_detection_event_action action;
};

/**
 * The set of callbacks user that user can register through
 * #sid_detect_unwanted_location_tracker_init
 */
struct sid_detect_unwanted_location_tracker_event_callbacks {
    /** User context data */
    void *context;

    /**
     * Callback that is invoked whenever the owner proximity changes
     *
     * The Sidewalk Stack calls this callback to let the user know that it near to
     * owner or it has been separated from the owner.
 .   *
     * @param[in] state #sid_owner_proximity_state value indicating the owner proximity state
     * @param[in] context The context pointer given in sid_detect_unwanted_location_tracker_event_callbacks.context
     */
    void (*on_owner_proximity_change)(enum sid_owner_proximity_state state, void *context);

    /**
     * Callback that is invoked when non-owner find event is triggered
     *
     * The sidewalk stack calls this callback to let the user know that a non-owner find
     * event has been triggered either from ble_gatt or motion detector
     *
     * @param[in] event The #sid_non_owner_find_event
     * @param[in] context The context pointer given in sid_detect_unwanted_location_tracker_event_callbacks.context
     */
    void (*on_non_owner_find_event)(const struct sid_non_owner_find_event *event, void *context);

    /**
     * Callback that is invoked when motion-detection event is triggered
     *
     * The sidewalk stack calls this callback when the user wants to use motion detection to trigger
     * the non-owner find event. This option is only required if
     * #SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_MOTION_DETECTION is set in
     * #sid_detect_unwanted_location_tracker_accessory_info.capabilities
     *
     * @see #sid_detect_unwanted_location_tracker_set_motion_detection
     *
     * @param[in] event The #sid_non_owner_find_event
     * @param[in] context The context pointer given in sid_detect_unwanted_location_tracker_event_callbacks.context
     */
    void (*on_motion_detection_event)(const struct sid_motion_detection_event *event, void *context);

    /**
     * Callback that is invoked when a non-owner wants to read the Identifier Payload
     *
     * The Sidewalk stack will trigger this callback, when the Get_Identifier opcode is called by over
     * BLE GATT. The payload size is set by #SID_DETECT_UNWANTED_LOCATION_TRACKER_IDENTIFIER_PAYLOAD_MAX_SIZE
     * This callback can only be triggered once #sid_detect_unwanted_location_read_identifier_payload_enter api
     * is called.
     *
     * @param[out] buf Buffer that user will fill with the Identifier Payload
     * @param[in] len The len of the buffer
     * @param[in] context The context pointer given in sid_detect_unwanted_location_tracker_event_callbacks.context
     *
     * @returns 0 in case of an error else the filled up buffer len
     */
    size_t (*on_identifier_read)(uint8_t *buf, size_t len, void *context);
};

/**
 * Describes the type of battery
 */
enum sid_detect_unwanted_location_tracker_accessory_battery_type {
    /** Indicates that the battery is powered */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_POWERED = 0,
    /** Indicates that the battery is non rechargeable */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_NON_RECHARGEABLE = 1,
    /** Indicates that the battery is rechargeable */
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_RECHARGEABLE = 2,
};

/**
 * Level of the battery
 */
enum sid_detect_unwanted_location_tracker_accessory_battery_level {
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_LEVEL_FULL = 0,
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_LEVEL_MEDIUM = 1,
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_LEVEL_LOW = 2,
    SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_LEVEL_CRITICALLY_LOW = 3
};

/**
 * Describes the capabilties of the device
 */
enum sid_detect_unwanted_location_tracker_capability_op {
    /**
     * The device supports sound as way for a non-owner to find the device
     * MANDATORY Unless #sid_detect_unwanted_location_tracker_config.enable_only_proximity_status
     * is set.
     * If this capability is set then sid_detect_unwanted_location_tracker_event_callbacks.on_non_owner_find_event
     * has to be provided.
     */
    SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_SOUND = (1 << 0),
    /**
     * The device supports motion_detection as a way to trigger find events
     * If this capability is set then sid_detect_unwanted_location_tracker_event_callbacks.on_motion_detection_event
     * has to be provided.
     */
    SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_MOTION_DETECTION = (1 << 1),
    /**
     * The device supports using NFC as a way to get the Identifier Payload
     */
    SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_IDENTIFIER_LOOKUP_NFC = (1 << 2),
    /**
     * The device supports using BLE as a way to get the Identifier Payload. If this
     * capability is set then sid_detect_unwanted_location_tracker_event_callbacks.on_identifier_read
     * has to be provided.
     */
    SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_IDENTIFIER_LOOKUP_BLE = (1 << 3),
};

/**
 * Describes the device accessory information.
 */
struct sid_detect_unwanted_location_tracker_accessory_info {
    /** Product data of the device,
     * @see #SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_PRODUCT_DATA_MAX_SIZE
     */
    const uint8_t *product_data;
    /** Product data of the device,
     * @see #SID_DETECT_UNWANTED_LOCATION_TRACKER_MANUFACTURER_NAME_MAX_SIZE
     */
    const uint8_t *manufacturer_name;
    /** Product data of the device,
     * @see #SID_DETECT_UNWANTED_LOCATION_TRACKER_MODEL_NAME_MAX_SIZE
     */
    const uint8_t *model_name;
    /** Holds the capabilities of the device
     * @see #sid_detect_unwanted_location_tracker_capability_op
     */
    uint32_t capabilities;
    /** Firmware version of the device
     * @see #sid_detect_unwanted_location_tracker_firmware_version
     */
    uint32_t firmware_version;
    /** Category of the device see DULT 4. Accessory Category Value for the
     * list of possible values
     */
    uint8_t category;
    /** The network_id that the device is part of. See DULT 10.1 and 10.1.1 for
     * possible values
     */
    uint8_t network_id;
    /* The type of battery in the device */
    enum sid_detect_unwanted_location_tracker_accessory_battery_type battery;
};

/**
 * Describes the config needed to initialize sidewalk DULT
 */
struct sid_detect_unwanted_location_tracker_config {
    /**
     * Below configuration controls the if Advertisement is done in both NEAR/SEPARATED mode or only
     * in SEPARATED mode;
     */
    enum sid_detect_unwanted_location_tracker_advertisement_mode mode;

    /**
     * Below is the config to define which mode dult is initialized in
     */
    enum sid_owner_proximity_state initial_promixity;

    /**
     * Accessory information that is exposed when device is in separated mode
     */
    const struct sid_detect_unwanted_location_tracker_accessory_info *info;
    /**
     * Callbacks associated with Detect Unwanted Location Tracker.
     * Must status static const storage as members are accessed without being copied
     */
    struct sid_detect_unwanted_location_tracker_event_callbacks *callbacks;
};

/**
 * Create the firmware version from the major, minor and revision
 *
 * @see #sid_detect_unwanted_location_tracker_accessory_info.firmware_version
 *
 * @param[in] major_version Major version number
 * @param[in] minor_version Minor version number
 * @param[in] revision Revision version number
 *
 * @returns Firmware version in 4 bytes format
 */
#define SID_DETECT_UNWANTED_LOCATION_TRACKER_FIRMWARE_VERSION(_major_version, _minor_version, _revision) \
    ((_major_version << 16) | (_minor_version << 8) | (_revision & 0xff))

static inline uint16_t
sid_detect_unwanted_location_tracker_firmware_major_version_from_firmware_version(uint32_t firmware_version)
{
    return (firmware_version >> 16) & 0xFFFF;
}

static inline uint8_t
sid_detect_unwanted_location_tracker_firmware_minor_version_from_firmware_version(uint32_t firmware_version)
{
    return (firmware_version >> 8) & 0xFF;
}

static inline uint8_t
sid_detect_unwanted_location_tracker_firmware_revision_from_firmware_version(uint32_t firmware_version)
{
    return firmware_version & 0xFF;
}

/**
 * Initialize Sidewalk Detect Unwanted Location Tracker support
 *
 * @param[in] config Required configuration needed to initialize Sidewalk Unwanted Location Tracker
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_detect_unwanted_location_tracker_init(const struct sid_detect_unwanted_location_tracker_config *config,
                                                      struct sid_handle *handle);

/**
 * De-initialize Sidewalk Detect Unwanted Location Tracker support
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_detect_unwanted_location_tracker_deinit(struct sid_handle *handle);

/**
 * Enters Sidewalk Detect Unwanted Location Tracker Read Identifier State
 *
 * According to DULT spec, a product can only allow the reading of Identifier Payload,
 * when the users deems it so. For e.g the user can long press a button which the
 * user application catches and then calls this API.
 *
 * @note #SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_IDENTIFIER_LOOKUP_BLE has to be set in
 * sid_detect_unwanted_location_tracker_accessory_info.capabilities for this api to work
 *
 * @see #sid_detect_unwanted_location_tracker_event_callbacks.on_identifier_read
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_detect_unwanted_location_tracker_read_identifier_payload_enter(struct sid_handle *handle);

/**
 * Update the status of motion detection to Sidewalk
 *
 * This API is called when #SID_MOTION_DETECTION_EVENT_TYPE_GET_MOTION_STATUS is given to the user
 * via sid_detect_unwanted_location_tracker_event_callbacks.on_motion_detection_event The user is expected
 * to call this api within the context of the callback.
 *
 * @note #SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_MOTION_DETECTION has to be set in
 * sid_detect_unwanted_location_tracker_accessory_info.capabilities for this API to work
 *
 * @see #sid_detect_unwanted_location_tracker_event_callbacks.on_motion_detection_event
 * @see #SID_MOTION_DETECTION_EVENT_TYPE_GET_MOTION_STATUS
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] status True if motion has been detected else false
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_detect_unwanted_location_tracker_update_motion_detection(struct sid_handle *handle, bool status);

/**
 * Trigger non-owner find event based on motion detection
 *
 * This Api is called to enable motion detection as on of the sources to trigger non-owner find events
 *
 * @note #SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_MOTION_DETECTION has to be set in
 * sid_detect_unwanted_location_tracker_accessory_info.capabilities for this API to work
 *
 * @see #sid_detect_unwanted_location_tracker_event_callbacks.on_motion_detection_event
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] set True to enable motion detection, false to disable motion detection
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_detect_unwanted_location_tracker_set_motion_detection(struct sid_handle *handle, bool set);

/**
 * Update find event status
 *
 * This Api should be called with the status of the action that was requested in
 * #sid_detect_unwanted_location_tracker_event_callbacks.on_non_owner_find_event
 *
 * @note sid_detect_unwanted_location_tracker_config.enable_only_proximity_status should not be set for
 * for this api to work
 *
 * @see #sid_detect_unwanted_location_tracker_event_callbacks.on_non_owner_find_event
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] status True if Find event was honered , false otherwise
 * @param[in] src Source of the event
 * @param[in] type Type of the non_owner find event
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t
sid_detect_unwanted_location_tracker_update_non_owner_find_event_status(struct sid_handle *handle,
                                                                        bool status,
                                                                        enum sid_non_owner_find_event_src src,
                                                                        enum sid_non_owner_find_event_type type);

/**
 * Retrieve the current proximity state
 *
 * This Api should be called when the application firmware wants to know if the device is near owner or in
 * separated mode
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[out] status A pointer that indicates if the device is near owner or separated
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t
sid_detect_unwanted_location_tracker_get_current_owner_proximity_state(struct sid_handle *handle,
                                                                       enum sid_owner_proximity_state *state);

/**
 * Set the current battery level
 *
 * This Api should be called to update the current battery level
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[out] level Battery level enum value see @sid_detect_unwanted_location_tracker_accessory_battery_level
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_detect_unwanted_location_tracker_set_battery_level(
    struct sid_handle *handle,
    enum sid_detect_unwanted_location_tracker_accessory_battery_level level);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_DETECT_UNWANTED_LOCATION_TRACKER_H */
