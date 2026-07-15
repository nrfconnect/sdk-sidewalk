/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#include <app_location_lr11xx_config.h>
#include <lr11xx_gnss_wifi_config.h>
#include <smtc_modem_geolocation_api.h>

static lr11xx_gnss_wifi_config_t gnss_wifi_config = {
	.constellation_type = SMTC_MODEM_GNSS_CONSTELLATION_GPS_BEIDOU,
	.scan_mode = SMTC_MODEM_GNSS_MODE_MOBILE
};

const lr11xx_gnss_wifi_config_t *get_location_cfg(void)
{
	return &gnss_wifi_config;
}
