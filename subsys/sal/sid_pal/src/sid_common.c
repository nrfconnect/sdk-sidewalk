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

#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
#include <sx126x_config.h>
#endif

#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_common, LOG_LEVEL_DBG);

sid_error_t sid_pal_common_init(const platform_specific_init_parameters_t *platform_init_parameters)
{
	if (!platform_init_parameters
#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
	    || !platform_init_parameters->radio_cfg
#endif
	) {
		return SID_ERROR_INCOMPATIBLE_PARAMS;
	}
#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
	set_radio_sx126x_device_config(platform_init_parameters->radio_cfg);
#endif /* defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT) */

#if defined(CONFIG_SIDEWALK_TEMPERATURE)
	sid_error_t ret_code = sid_pal_temperature_init();
	if (ret_code) {
		LOG_ERR("Sidewalk Init temperature pal  err: %d", ret_code);
	}
#endif

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_common_deinit(void)
{
#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
	sid_pal_radio_deinit();
#endif
	return SID_ERROR_NONE;
}
