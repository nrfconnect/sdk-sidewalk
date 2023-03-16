/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>

#include <pal_init.h>

#if defined(CONFIG_SIDEWALK_SUBGHZ)
#include <app_subGHz_config.h>
#endif
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_temperature_ifc.h>

LOG_MODULE_REGISTER(sid_board_init, CONFIG_SIDEWALK_LOG_LEVEL);

#define EMPTY_MFG_HEX_PARTITION (0xFFFFFFFF)

sid_error_t application_pal_init(void)
{
	sid_error_t ret_code;

	ret_code = sid_pal_storage_kv_init();
	if (ret_code) {
		LOG_ERR("Sidewalk KV store init failed, err: %d", ret_code);
		return ret_code;
	}

	ret_code = sid_pal_crypto_init();
	if (ret_code) {
		LOG_ERR("Sidewalk Init Crypto HAL, err: %d", ret_code);
		return ret_code;
	}
	#if defined(CONFIG_SIDEWALK_TEMPERATURE)
	ret_code = sid_pal_temperature_init();
	if (ret_code) {
		LOG_ERR("Sidewalk Init temperature pal  err: %d", ret_code);
	}
	#endif
	static const sid_pal_mfg_store_region_t mfg_store_region = {
		.addr_start = (uintptr_t)(FIXED_PARTITION_OFFSET(mfg_storage)),
		.addr_end = (uintptr_t)(FIXED_PARTITION_OFFSET(mfg_storage) + FIXED_PARTITION_SIZE(mfg_storage)),
	};

	sid_pal_mfg_store_init(mfg_store_region);

	if (sid_pal_mfg_store_get_version() == EMPTY_MFG_HEX_PARTITION) {
		LOG_ERR("The mfg.hex version mismatch");
		LOG_ERR("Check if the file has been generated and flashed properly");
		LOG_ERR("START ADDRESS: 0x%08x", FIXED_PARTITION_OFFSET(mfg_storage));
		LOG_ERR("SIZE: 0x%08x", FIXED_PARTITION_SIZE(mfg_storage));
		return SID_ERROR_NOT_FOUND;
	}

	#if defined(CONFIG_SIDEWALK_SUBGHZ)
	set_radio_sx126x_device_config(get_radio_cfg());
	#endif /* defined(CONFIG_SIDEWALK_SUBGHZ) */

	return SID_ERROR_NONE;
}
