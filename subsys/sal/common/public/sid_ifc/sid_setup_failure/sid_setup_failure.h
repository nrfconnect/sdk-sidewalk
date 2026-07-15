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

#ifndef SID_SETUP_FAILURE_H
#define SID_SETUP_FAILURE_H

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup SIDEWALK_BULK_DATA_TRANSFER_API Sidewalk Bulk Data Transfer API
 * @brief API for communicating with the Sidewalk file transfer
 * @{
 * @ingroup  SIDEWALK_BULK_DATA_TRANSFER_API
 */

#include <sid_api.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function pointer type for setup failure callback
 *
 * @param[in] context The context pointer for the callback
 * @param[out] product_failure Pointer to store the product specific failure code
 */
typedef void (*sid_on_setup_failure_t)(void *context, uint32_t *product_failure);

/**
 * Register callback for setup failure events
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] callback Function pointer to the setup failure callback
 *
 * @returns #SID_ERROR_NONE in case of success
 * @returns #SID_ERROR_INVALID_ARGS if handle or callback is NULL
 */
sid_error_t sid_register_setup_failure(struct sid_handle *handle, sid_on_setup_failure_t callback);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

/// @endcond

#endif /* SID_SETUP_FAILURE_H */
