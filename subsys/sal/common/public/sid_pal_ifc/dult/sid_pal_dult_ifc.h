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

#ifndef SID_PAL_DULT_IFC_H
#define SID_PAL_DULT_IFC_H

#include <sid_detect_unwanted_location_tracker.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t sid_pal_dult_start(void);

sid_error_t sid_pal_dult_stop(void);

sid_error_t sid_pal_dult_identifier_payload_enter(void);

sid_error_t sid_pal_dult_update_motion_detection(bool status);

sid_error_t sid_pal_dult_set_motion_detection(bool set);

sid_error_t sid_pal_dult_update_find_event_status(bool status,
                                                  enum sid_non_owner_find_event_src src,
                                                  enum sid_non_owner_find_event_type type);

sid_error_t sid_pal_dult_set_current_owner_proximity_state(enum sid_owner_proximity_state state);

sid_error_t sid_pal_dult_set_battery_level(enum sid_detect_unwanted_location_tracker_accessory_battery_level level);

sid_error_t sid_pal_dult_init(const struct sid_detect_unwanted_location_tracker_accessory_info *info,
                              struct sid_detect_unwanted_location_tracker_event_callbacks *callbacks);
sid_error_t sid_pal_dult_deinit(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_PAL_DULT_IFC_H */
