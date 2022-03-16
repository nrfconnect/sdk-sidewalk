/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_log_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <storage/flash_map.h>
#include <sid_pal_assert_ifc.h>
#include <logging/log.h>
#include <zephyr.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#define MFG_EMPTY_VERSION       0xFFFFFFFFUL

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

static const sid_pal_mfg_store_region_t mfg_store_region = {
	.addr_start = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage)),
	.addr_end = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage) + FLASH_AREA_SIZE(mfg_storage)),
};

void main(void)
{
	uint32_t mfg_ver;

	LOG_INF("Hello Sidewalk World! %s\n", CONFIG_BOARD);

	if (SID_ERROR_NONE == sid_pal_storage_kv_init()) {
		LOG_INF("NVM memory is ready!\n");
	}

	SID_PAL_LOG_INFO("Hello Sidewalk Log! %d", 2022);

	sid_pal_mfg_store_init(mfg_store_region);
	mfg_ver = sid_pal_mfg_store_get_version();
	if (MFG_EMPTY_VERSION == mfg_ver) {
		SID_PAL_LOG_INFO("Mfg storage is not provisioned!");
	} else {
		uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE];
		SID_PAL_LOG_INFO("Mfg storage ver: 0x%04x", mfg_ver);
		sid_pal_mfg_store_dev_id_get(dev_id);
		SID_PAL_LOG_INFO("Mfg dev ID:");
		SID_PAL_HEXDUMP(SID_PAL_LOG_SEVERITY_INFO, dev_id, sizeof(dev_id));
	}

	SID_PAL_ASSERT(true);
}
