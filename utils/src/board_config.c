/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <board_config.h>
#include <app_900_config.h>
#include <sm_task.h>
#include <sm_buttons.h>
#include <sm_leds.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_temperature_ifc.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sid_demo_init, CONFIG_SIDEWALK_LOG_LEVEL);

#define EMPTY_MFG_HEX_PARTITION (0xFFFFFFFF)

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

sid_error_t board_config(void)
{
	int err = sm_buttons_init();

	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
		return SID_ERROR_IO_ERROR;
	}

	err = sm_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return SID_ERROR_IO_ERROR;
	}

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

	ret_code = sid_pal_temperature_init();
	if (ret_code) {
		LOG_ERR("Sidewalk Init temperature pal  err: %d", ret_code);
	}

	static const sid_pal_mfg_store_region_t mfg_store_region = {
		.addr_start = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage)),
		.addr_end = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage) + FLASH_AREA_SIZE(mfg_storage)),
	};

	sid_pal_mfg_store_init(mfg_store_region);

	if (sid_pal_mfg_store_get_version() == EMPTY_MFG_HEX_PARTITION) {
		LOG_ERR("The mfg.hex version mismatch");
		LOG_ERR("Check if the file has been generated and flashed properly");
		LOG_ERR("START ADDRESS: 0x%08x", FLASH_AREA_OFFSET(mfg_storage));
		LOG_ERR("SIZE: 0x%08x", FLASH_AREA_SIZE(mfg_storage));
		return SID_ERROR_NOT_FOUND;
	}

	#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	set_radio_sx126x_device_config(get_radio_cfg());
	#endif /* defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */

	return SID_ERROR_NONE;
}
