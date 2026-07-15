/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SID_RESET_INFO_TYPES_H
#define SID_RESET_INFO_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Masks that define data fields in the sid_reset_info structure
 */
enum {
    SID_RESET_INFO_PROGRAM_COUNTER_MSK = 0x01,
    SID_RESET_INFO_LINK_REGISTER_MSK = 0x02,
    SID_RESET_INFO_UPTIME_MSK = 0x04,
    SID_RESET_INFO_KEY_REFRESH_STATE_MSK = 0x08,
    SID_RESET_INFO_RESET_REASON_MSK = 0x10,
    SID_RESET_INFO_ALL_MSK = 0x1F,
};

/**
 * Describes the reset info to be populated by application.
 */
struct sid_reset_info {
    /** The PC where reset occured */
    uint32_t program_counter;

    /** The link register value	*/
    uint32_t link_register;

    /** System uptime in seconds */
    uint32_t uptime;

    /** Security Key Refresh State */
    uint8_t key_refresh_state;

    /** Reset reasons */
    uint8_t reset_reason;

    /** A mask that defines what data is present in the structure */
    uint8_t data_mask;

    /** Flag indicating write operation into non-init RAM */
    bool is_wr_operation;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_RESET_INFO_TYPES_H */
