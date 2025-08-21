/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#include <sid_pal_mfg_store_ifc.h>
#include <sid_900_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

const void* get_radio_cfg(void);
const sid_pal_mfg_store_region_t* get_mfg_cfg(void);
struct sid_sub_ghz_links_config* app_get_sub_ghz_config(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_900_CONFIG_H */
