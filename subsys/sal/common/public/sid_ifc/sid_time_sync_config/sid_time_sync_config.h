/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SID_TIME_SYNC_CONFIG_H
#define SID_TIME_SYNC_CONFIG_H

#include <stdint.h>

#define TIME_SYNC_INTERVAL_MIN_HOURS 2
#define TIME_SYNC_INTERVAL_MAX_HOURS 720

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Time synchronization configuration.
 * Array of adaptive intervals (in hours) between sync attempts.
 * Values should be increasing and in range [TIME_SYNC_INTERVAL_MIN_HOURS, TIME_SYNC_INTERVAL_MAX_HOURS], for example:
 * {4, 8, 12}. On successful sync, next interval advances through the array. On sync failure, interval resets to
 * implementation-defined initial value.
 */
struct sid_time_sync_config {
    /* Array of adaptive intervals for syncs */
    const uint16_t *adaptive_sync_intervals_h;
    /* Number of intervals */
    uint8_t num_intervals;
};

#ifdef __cplusplus
}
#endif

#endif
