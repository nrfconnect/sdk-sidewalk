/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <sid_pal_mfg_store_ifc.h>
#include <sid_error.h>
#include <zephyr/storage/flash_map.h>

ZTEST(mfg, test_mfg_storage_read_write)
{
	uint8_t write_data[SID_PAL_MFG_STORE_DEVID_SIZE] = { 1, 2, 3, 4, 5 };
	uint8_t read_data[SID_PAL_MFG_STORE_DEVID_SIZE] = { 0 };

	sid_pal_mfg_store_region_t mfg_store_region = {
		.addr_start = (uintptr_t)(FIXED_PARTITION_OFFSET(mfg_storage)),
		.addr_end = (uintptr_t)(FIXED_PARTITION_OFFSET(mfg_storage) +
					FIXED_PARTITION_SIZE(mfg_storage)),
		.app_value_to_offset = NULL
	};
	sid_pal_mfg_store_init(mfg_store_region);

	zassert_equal(SID_ERROR_NONE, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_SERIAL_NUM, write_data, sizeof(write_data)));

	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVID, read_data, sizeof(read_data));
	zassert_mem_equal(write_data, read_data, sizeof(write_data),
			  "Read data does not match written data");

	sid_pal_mfg_store_deinit();
}

ZTEST_SUITE(mfg, NULL, NULL, NULL, NULL, NULL);
