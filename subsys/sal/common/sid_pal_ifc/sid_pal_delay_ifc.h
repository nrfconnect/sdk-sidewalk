/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PAL_DELAY_IFC_H
#define SID_PAL_DELAY_IFC_H

/** @file
 *
 * @defgroup sid_pal_delay_ifc SID delay interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details Provides a way for Sub-Ghz protocol to control delay
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Implements a busy wait delay safe to be used in SWI context.
 *
 * Due to busy wait implementation, should not be used for long durations.
 * If you need delay >1ms then use sid_pal_scheduler_delay_ms instead.
 *
 * @param[in]   delay          Time in us to delay
 *
 * @retval none
 */
void sid_pal_delay_us(uint32_t delay);

/**
 * Implements a delay function using RTOS API call.
 * This function will block the calling thread, do not use in ISR context.
 * This will allow the RTOS scheduler to run other tasks or switch to an
 * IDLE state while the delay is pending.
 *
 * If you need sub-millisecond delays then use sid_pal_delay_us instead.
 *
 * Do not use this function for internal Sidewalk stack delays - use the
 * sid_pal_timer API instead.
 *
 * @param[in]   delay          Time in ms to delay
 *
 * @retval none
 */
void sid_pal_scheduler_delay_ms(uint32_t delay);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SID_PAL_DELAY_IFC_H */
