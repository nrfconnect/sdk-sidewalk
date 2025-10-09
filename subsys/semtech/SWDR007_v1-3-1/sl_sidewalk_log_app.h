/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SL_SIDEWALK_LOG_H
#define SL_SIDEWALK_LOG_H


#ifdef __cplusplus
extern "C" {
#endif

#include <sid_pal_log_ifc.h>

#ifndef BRED
#define BRED
#endif

#ifndef BYEL
#define BYEL
#endif

#ifndef COLOR_RESET
#define COLOR_RESET
#endif

// Error logging macro
#define SL_SID_LOG_APP_ERROR(...) SID_PAL_LOG_ERROR(__VA_ARGS__)

// Info logging macro
#define SL_SID_LOG_APP_INFO(...) SID_PAL_LOG_INFO(__VA_ARGS__)

// Warning logging macro
#define SL_SID_LOG_APP_WARNING(...) SID_PAL_LOG_WARNING(__VA_ARGS__)

// Debug logging macro
#define SL_SID_LOG_APP_DEBUG(...) SID_PAL_LOG_DEBUG(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // SL_SIDEWALK_LOG_H
