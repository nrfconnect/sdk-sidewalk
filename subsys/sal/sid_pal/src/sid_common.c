/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_error.h>
#include <sid_pal_common_ifc.h>
#include <sid_pal_temperature_ifc.h>

#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
#include <lr11xx_gnss_wifi_config.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */

#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_common, CONFIG_SIDEWALK_LOG_LEVEL);

#if defined(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X)
#include <sx126x_config.h>
static void set_radio_config(const void *cfg)
{
	set_radio_sx126x_device_config((const radio_sx126x_device_config_t *)cfg);
}
#elif defined(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
#include <lr11xx_config.h>
static void set_radio_config(const void *cfg)
{
	set_radio_lr11xx_device_config((const radio_lr11xx_device_config_t *)cfg);
}
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO */

#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT) && (defined(REGION_ALL) || defined(REGION_LITE))
#include <sid_900_cfg.h>
#include <sid_pal_radio_ifc.h>
#include "sid_reg_priv.h"

static void update_link_max_tx_power(struct sid_sub_ghz_links_config *sub_ghz_cfg, uint8_t region)
{
	sid_pal_radio_data_rate_t link3_data_rate = (region == SID_PAL_RADIO_RC_EU) ?
							    SID_PAL_RADIO_DATA_RATE_1_76KBPS :
							    SID_PAL_RADIO_DATA_RATE_2KBPS;
	int8_t tx_power = 0;

	if (!sub_ghz_cfg) {
		return;
	}
	if (!sid_pal_radio_get_max_tx_power(SID_PAL_RADIO_DATA_RATE_50KBPS, &tx_power)) {
		sub_ghz_cfg->link2_max_tx_power_in_dbm = tx_power;
	}
	if (!sid_pal_radio_get_max_tx_power(link3_data_rate, &tx_power)) {
		sub_ghz_cfg->link3_max_tx_power_in_dbm = tx_power;
	}
}

static sid_error_t region_init(const platform_specific_init_parameters_t *params)
{
	if (params->region == SID_PAL_RADIO_RC_NONE) {
		LOG_WRN("Region not set, sub-GHz links unavailable");
		return SID_ERROR_NONE;
	}

	/* Update region to regulatory framework */
	sid_reg_set_region_ready(false);
	if (sid_reg_set_regional_info(params->region, params->iso_country)) {
		LOG_ERR("Region %d not supported", params->region);
		return SID_ERROR_NOSUPPORT;
	}
	sid_reg_set_region_ready(true);

	uint8_t region = sid_reg_get_regional_info()->region;

	if (sid_pal_radio_set_region(region)) {
		LOG_ERR("No radio parameters for region %d", region);
		return SID_ERROR_UNINITIALIZED;
	}
	update_link_max_tx_power(params->sub_ghz_cfg, region);

	return SID_ERROR_NONE;
}
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT && (REGION_ALL || REGION_LITE) */

sid_error_t sid_pal_common_init(const platform_specific_init_parameters_t *platform_init_parameters)
{
	if (!platform_init_parameters) {
		return SID_ERROR_INCOMPATIBLE_PARAMS;
	}

#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
	if (!platform_init_parameters->radio_cfg) {
		return SID_ERROR_INCOMPATIBLE_PARAMS;
	}
	set_radio_config(platform_init_parameters->radio_cfg);
#if defined(REGION_ALL) || defined(REGION_LITE)
	sid_error_t region_err = region_init(platform_init_parameters);

	if (region_err) {
		return region_err;
	}
#endif /* REGION_ALL || REGION_LITE */
#endif /* defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT) */

#if defined(CONFIG_SIDEWALK_TEMPERATURE)
	sid_error_t ret_code = sid_pal_temperature_init();
	if (ret_code) {
		LOG_ERR("Sidewalk Init temperature pal  err: %d", ret_code);
	}
#endif

#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
	set_lr11xx_gnss_wifi_config(platform_init_parameters->gnss_wifi_cfg);
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_common_deinit(void)
{
#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
	sid_pal_radio_deinit();
#endif
	return SID_ERROR_NONE;
}
