/*
 * Copyright 2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_PLATFORM_INIT_TYPES_H
#define SID_PAL_PLATFORM_INIT_TYPES_H

#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
#if defined(CONFIG_RADIO_SX126X)
#include <sx126x_config.h>
#elif defined(CONFIG_RADIO_LR11XX)
#include <lr11xx_config.h>
#endif
#endif

typedef struct {
//place holder for platform specific init parameters
#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
#if defined(CONFIG_RADIO_SX126X)
    radio_sx126x_device_config_t * radio_cfg;
#elif defined(CONFIG_RADIO_LR11XX)
    radio_lr11xx_device_config_t * radio_cfg;
#endif
#endif
} platform_specific_init_parameters_t;

#endif
