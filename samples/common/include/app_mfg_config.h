/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdbool.h>
#include <sid_pal_mfg_store_ifc.h>
#include <zephyr/storage/flash_map.h>

#if !FIXED_PARTITION_EXISTS(mfg_storage)
#error "Flash partition is not defined for the Sidewalk manufacturing storage!"
#endif

#define APP_MFG_CFG_FLASH_START FIXED_PARTITION_OFFSET(mfg_storage)
#define APP_MFG_CFG_FLASH_SIZE FIXED_PARTITION_SIZE(mfg_storage)
#define APP_MFG_CFG_FLASH_END (APP_MFG_CFG_FLASH_START + APP_MFG_CFG_FLASH_SIZE)

#define EMPTY_MFG_HEX_PARTITION (0xFFFFFFFF)

static inline bool app_mfg_cfg_is_valid(void)
{
	return sid_pal_mfg_store_get_version() == EMPTY_MFG_HEX_PARTITION;
}
