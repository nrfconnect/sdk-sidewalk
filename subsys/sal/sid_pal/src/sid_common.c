/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_error.h>
#include <sid_pal_common_ifc.h>
#include <sid_pal_temperature_ifc.h>
#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT) && defined(CONFIG_SOC_SERIES_NRF53X)
#include <zephyr/bluetooth/bluetooth.h>
#endif /* defined(CONFIG_SOC_SERIES_NRF53X) && defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT) */

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
#if defined(CONFIG_SOC_SERIES_NRF53X)
	(void)bt_enable(NULL);
	(void)bt_disable();
#endif /* defined(CONFIG_SOC_SERIES_NRF53X) */
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
