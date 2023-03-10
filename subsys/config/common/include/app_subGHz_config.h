/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef APP_900_CONFIG_H
#define APP_900_CONFIG_H

#include <sx126x_config.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_900_cfg.h>

const radio_sx126x_device_config_t *get_radio_cfg(void);
const sid_pal_mfg_store_region_t *get_mfg_cfg(void);
const struct sid_sub_ghz_links_config *app_get_sub_ghz_config(void);
#endif
