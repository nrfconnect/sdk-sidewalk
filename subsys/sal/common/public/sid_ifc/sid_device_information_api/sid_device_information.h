/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates.  All rights reserved.
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
 * Structure containing device information
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
 * Initializes the device information module and registers the GET_DEVICE_INFORMATION command handler.
 *
 * @param[in] config    Pointer to device information configuration structure. The pointed-to
 *                      configuration data must remain valid for the lifetime of the application.
 * @param[in] handle    A pointer to the handle returned by sid_init()
 *
 * @returns returns SID_ERROR_NONE in case of success.
 */
sid_error_t sid_device_information_init(const struct sid_device_info *config, struct sid_handle *handle);

/**
 * Deinitializes the device information module and deregisters the GET_DEVICE_INFORMATION command handler.
 *
 * @return SID_ERROR_NONE
 */
sid_error_t sid_device_information_deinit(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_DEVICE_INFORMATION_H */
