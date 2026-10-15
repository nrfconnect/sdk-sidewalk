/*
 * Copyright 2025-2026 Amazon.com, Inc. or its affiliates.  All rights reserved.
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
#ifndef SID_DEVICE_INFORMATION_H
#define SID_DEVICE_INFORMATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_error.h>
#include <sid_api.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief Device information type enumeration
 */
enum sid_device_info_type {
    SID_DEVICE_INFO_TYPE_ENV = 1,            /**< Environment information type */
    SID_DEVICE_INFO_TYPE_BATTERY_LEVEL = 2,  /**< Battery Level type */
    SID_DEVICE_INFO_TYPE_OTA_BLOCK_SIZE = 3, /**< OTA block size type */
    SID_DEVICE_INFO_TYPE_REGION = 4,         /**< Regulatory region type */
};

/**
 * @brief Device environment enumeration
 */
enum sid_device_env {
    SID_DEVICE_ENV_PROD = 1,  /**< Production environment */
    SID_DEVICE_ENV_GAMMA = 2, /**< Gamma environment */
    SID_DEVICE_ENV_DEV = 3,   /**< Development environment */
};

/**
 * @brief Device regulatory region enumeration
 *
 * One of four regulatory zones the device firmware distinguishes. Provisioned during device
 * setup and persisted by the application so the device selects the correct frequency plan and
 * regulatory parameters on radio bring-up.
 */
enum sid_device_region {
    SID_DEVICE_REGION_UNSET = 0,         /**< Reserved / unset (SID_PAL_RADIO_RC_NONE) */
    SID_DEVICE_REGION_NORTH_AMERICA = 1, /**< North America (US/CA/MX) */
    SID_DEVICE_REGION_EUROPE = 2,        /**< Europe, incl. UK */
    SID_DEVICE_REGION_JAPAN = 3,         /**< Japan */
    SID_DEVICE_REGION_AUSTRALIA_NZ = 4,  /**< Australia / New Zealand */
    SID_DEVICE_REGION_NOT_SUPPORT = 0xFF,
};

/**
 * @brief Dynamic device information structure
 */
struct sid_device_dyn_info {
    enum sid_device_info_type type; /**< Type of device information */
    union {
        enum sid_device_env env;       /**< Device environment */
        uint8_t battery_level;         /**< Battery level in % */
        uint32_t ota_block_size;       /**< OTA block size in bytes */
        enum sid_device_region region; /**< Regulatory region */
    };
};

/**
 * @brief Static device information structure
 */
struct sid_device_info {
    /** Size of serial number (max 32 bytes) */
    uint8_t serial_number_size;

    /** Size of MAC address (must be 6 bytes) */
    uint8_t mac_address_size;

    /** Size of product firmware version (max 32 bytes) */
    uint8_t product_fw_version_size;

    /** Size of device kind (max 32 bytes) */
    uint8_t device_kind_size;

    /** Pointer to serial number buffer (up to 32 bytes) */
    uint8_t *serial_number;

    /** Pointer to MAC address buffer (6 bytes) */
    uint8_t *mac_address;

    /** Pointer to firmware version buffer (up to 32 bytes) */
    uint8_t *product_fw_version;

    /** Pointer to device kind buffer (up to 32 bytes) */
    uint8_t *device_kind;
};

/**
 * @brief Device information callback functions
 */
struct sid_device_info_callbacks {
    /**
     * @brief Callback function to get device information
     * @param[out] info Pointer to device information structure to populate
     * @param[in] context User-defined context pointer
     */
    bool (*get_info)(struct sid_device_dyn_info *info, void *context);

    /**
     * @brief Callback function to set device information
     * @param[in] info Pointer to device information structure to set
     * @param[in] context User-defined context pointer
     * @return true if successful, false otherwise
     */
    bool (*set_info)(const struct sid_device_dyn_info *info, void *context);

    void *context; /**< User-defined context pointer */
};

/**
 * @brief Device information configuration structure
 */
struct sid_device_info_config {
    const struct sid_device_info *info;          /**< Pointer to static device information */
    struct sid_device_info_callbacks *callbacks; /**< Pointer to callback functions */
};

/**
 * @brief Initializes the device information module and registers the GET_DEVICE_INFORMATION command handler
 *
 * @param[in] config Pointer to device information configuration structure. The pointed-to
 *                   configuration data must remain valid for the lifetime of the application.
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @return SID_ERROR_NONE on success, error code otherwise
 */
sid_error_t sid_device_information_init(const struct sid_device_info_config *config, struct sid_handle *handle);

/**
 * @brief Deinitializes the device information module and deregisters the GET_DEVICE_INFORMATION command handler
 *
 * @return SID_ERROR_NONE on success
 */
sid_error_t sid_device_information_deinit(void);

/**
 * @brief Returns whether the configured region permits registration / radio bring-up.
 *
 * Called by the SDK from sid_start() to enforce the region safety gate: a device whose region is
 * unset or set to an unsupported value (e.g. Japan, which is defined but not enabled) must not
 * bring up its radios. Backward compatible -- returns true when the device information module is
 * not initialized or the application does not report a region (legacy firmware without i18n
 * support), so pre-region products are unaffected.
 *
 * @return true if registration / radio bring-up is allowed, false if it must be blocked.
 */
bool sid_device_information_is_region_registerable(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_DEVICE_INFORMATION_H */
