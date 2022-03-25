/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_storage.c
 *  @brief Sidewalk nvm storage.
 */

#include <stdint.h>
#include <zephyr.h>
#include <fs/nvs.h>
#include <storage/flash_map.h>
#include <sid_pal_storage_kv_ifc.h>

/* Reserved space in the NVM memory. */
#define NVS_RES_SPACE   (32U)

/* Flash block size in bytes */
#define NVS_SECTOR_SIZE  (DT_PROP(DT_CHOSEN(zephyr_flash), erase_block_size))
/* Numbers of sectors */
#define NVS_SECTOR_COUNT 2
/* Start address of the filesystem in flash */
#define NVS_STORAGE_OFFSET FLASH_AREA_OFFSET(storage)
/* Number of group IDs */
#define SID_GROUP_ID_COUNT      1

static struct nvs_fs fs[SID_GROUP_ID_COUNT] = {
	{
		.sector_size = NVS_SECTOR_SIZE,
		.sector_count = NVS_SECTOR_COUNT,
		.offset = NVS_STORAGE_OFFSET,
	},
};

/**
 * @brief Initialize NVM in flash.
 *
 * @param fs pointer to file system.
 * @return SID_ERROR_NONE when success, SID_ERROR_GENERIC otherwise.
 */
static sid_error_t kv_storage_init(struct nvs_fs *fs)
{
	sid_error_t erc = SID_ERROR_NONE;

	fs->flash_device = device_get_binding(DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
	if (fs->flash_device == NULL) {
		erc = SID_ERROR_GENERIC;
	}

	if (SID_ERROR_NONE == erc) {
		if (0 > nvs_mount(fs)) {
			erc = SID_ERROR_GENERIC;
		}
	}
	return erc;
}

sid_error_t sid_pal_storage_kv_init()
{
#if defined(CONFIG_NVS)
	for (int cnt = 0; cnt < SID_GROUP_ID_COUNT; cnt++) {
		if (SID_ERROR_NONE != kv_storage_init(&fs[cnt])) {
			return SID_ERROR_GENERIC;
		}
	}
	return SID_ERROR_NONE;
#else
	return SID_ERROR_NOSUPPORT;
#endif
}

sid_error_t sid_pal_storage_kv_record_get(uint16_t group, uint16_t key, void *p_data, uint32_t len)
{
	ssize_t rc;
	uint8_t *buff;

	if (SID_GROUP_ID_COUNT <= group) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}

	if (!p_data) {
		return SID_ERROR_NULL_POINTER;
	}
	buff = (uint8_t *)p_data;

	rc = nvs_read(&fs[group], key, buff, len);
	if (0 > rc) {
		if (-ENOENT == rc) {
			return SID_ERROR_NOT_FOUND;
		}
		return SID_ERROR_STORAGE_READ_FAIL;
	}
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group, uint16_t key, uint32_t *p_len)
{
	uint8_t dummy_arg;
	ssize_t ret_len = 0;

	if (SID_GROUP_ID_COUNT <= group) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}

	if (!p_len) {
		return SID_ERROR_NULL_POINTER;
	}

	ret_len = nvs_read(&fs[group], key, &dummy_arg, sizeof(dummy_arg));
	if (0 < ret_len) {
		*p_len = ret_len;
		return SID_ERROR_NONE;
	} else if (-ENOENT == ret_len) {
		return SID_ERROR_NOT_FOUND;
	}
	return SID_ERROR_STORAGE_READ_FAIL;
}

sid_error_t sid_pal_storage_kv_record_set(uint16_t group, uint16_t key, void const *p_data, uint32_t len)
{
	ssize_t rc;
	uint8_t *buff;

	if (SID_GROUP_ID_COUNT <= group) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}

	if (0U == len) {
		return SID_ERROR_INVALID_ARGS;
	}

	if (len > (NVS_SECTOR_SIZE - NVS_RES_SPACE)) {
		return SID_ERROR_OUT_OF_RESOURCES;
	}

	if (!p_data) {
		return SID_ERROR_NULL_POINTER;
	}

	buff = (uint8_t *)p_data;

	rc = nvs_write(&fs[group], key, buff, len);
	if (0 > rc) {
		if (-ENOSPC == rc) {
			return SID_ERROR_STORAGE_FULL;
		}
		return SID_ERROR_STORAGE_WRITE_FAIL;
	}

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_delete(uint16_t group, uint16_t key)
{
	if (SID_GROUP_ID_COUNT <= group) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}

	if (0 != nvs_delete(&fs[group], key)) {
		return SID_ERROR_STORAGE_ERASE_FAIL;
	}
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_group_delete(uint16_t group)
{
	if (SID_GROUP_ID_COUNT <= group) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}

	if (0 != nvs_clear(&fs[group])) {
		return SID_ERROR_STORAGE_ERASE_FAIL;
	}

	/* The nvs needs to be reinitialized after clearing */
	return kv_storage_init(&fs[group]);
}
