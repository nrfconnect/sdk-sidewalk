/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_mfg_storage.c
 *  @brief Sidewalk MFG storage.
 */

#include <sid_pal_mfg_store_ifc.h>
#include <sid_error.h>
#include <mfg_store_offsets.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <fprotect.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_mfg, CONFIG_SIDEWALK_LOG_LEVEL);

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#include <sid_crypto_keys.h>
static int sid_mfg_storage_secure_read(uint16_t *p_value, uint8_t *buffer, uint16_t length);
static void sid_mfg_storage_secure_init(void);
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

#define FLASH_MEM_CHUNK (128)

#ifndef DEV_ID_REG
#if defined(NRF52840_XXAA) || defined(NRF52833_XXAA) || defined(NRF52832_XXAA)
#define DEV_ID_REG (uint32_t)(NRF_FICR->DEVICEID[0])
#elif defined(NRF5340_XXAA)
#if defined(NRF_APPLICATION)
#define DEV_ID_REG (uint32_t)(NRF_FICR_S->INFO.DEVICEID[0])
#elif defined(NRF_NETWORK)
#define DEV_ID_REG (uint32_t)(NRF_FICR_NS->INFO.DEVICEID[0])
#endif /* NRF5340_XXAA */
#elif defined(NRF54L15_ENGA_XXAA)
#define DEV_ID_REG (uint32_t)(NRF_FICR->INFO.DEVICEID[0])
#else
#error "Unknow Device ID register."
#endif
#endif /* DEV_ID_REG */

#define MFG_STORE_TLV_HEADER_SIZE (4)
#define MFG_STORE_TLV_EMPTY_TAG_VALUE (0xFFFF)

struct sid_pal_mfg_store_tlv_info {
	uint16_t tag;
	uint16_t length;
	/** TLV offset from mfg_store_region.addr_start in bytes */
	size_t offset;
};

struct sid_pal_mfg_store_value_to_address_offset {
	sid_pal_mfg_store_value_t value;
	uint16_t size;
	uint32_t offset;
};

// clang-format off
struct sid_pal_mfg_store_value_to_address_offset sid_pal_mfg_store_app_value_to_offset_table[] = {
    {SID_PAL_MFG_STORE_VERSION,                      SID_PAL_MFG_STORE_VERSION_SIZE,                      SID_PAL_MFG_STORE_OFFSET_VERSION},
    {SID_PAL_MFG_STORE_DEVID,                        SID_PAL_MFG_STORE_DEVID_SIZE,                        SID_PAL_MFG_STORE_OFFSET_DEVID},
    {SID_PAL_MFG_STORE_SERIAL_NUM,                   SID_PAL_MFG_STORE_SERIAL_NUM_SIZE,                   SID_PAL_MFG_STORE_OFFSET_SERIAL_NUM},
    {SID_PAL_MFG_STORE_SMSN,                         SID_PAL_MFG_STORE_SMSN_SIZE,                         SID_PAL_MFG_STORE_OFFSET_SMSN},
    {SID_PAL_MFG_STORE_APID,                         SID_PAL_MFG_STORE_APID_SIZE,                         SID_PAL_MFG_STORE_OFFSET_APID},
    {SID_PAL_MFG_STORE_APP_PUB_ED25519,              SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE,              SID_PAL_MFG_STORE_OFFSET_APP_PUB_ED25519},
    {SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,          SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_SIZE,          SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_ED25519},
    {SID_PAL_MFG_STORE_DEVICE_PUB_ED25519,           SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIZE,           SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519},
    {SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE, SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_SIZE, SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519_SIGNATURE},
    {SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1,           SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_SIZE,           SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_P256R1},
    {SID_PAL_MFG_STORE_DEVICE_PUB_P256R1,            SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIZE,            SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1},
    {SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,  SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_SIZE,  SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1_SIGNATURE},
    {SID_PAL_MFG_STORE_DAK_PUB_ED25519,              SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIZE,              SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519},
    {SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE,    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_SIZE,    SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519_SIGNATURE},
    {SID_PAL_MFG_STORE_DAK_ED25519_SERIAL,           SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_SIZE,           SID_PAL_MFG_STORE_OFFSET_DAK_ED25519_SERIAL},
    {SID_PAL_MFG_STORE_DAK_PUB_P256R1,               SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIZE,               SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1},
    {SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE,     SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_SIZE,     SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1_SIGNATURE},
    {SID_PAL_MFG_STORE_DAK_P256R1_SERIAL,            SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_SIZE,            SID_PAL_MFG_STORE_OFFSET_DAK_P256R1_SERIAL},
    {SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519,          SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIZE,          SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519},
    {SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE,SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_SIZE,SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519_SIGNATURE},
    {SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL,       SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_SIZE,       SID_PAL_MFG_STORE_OFFSET_PRODUCT_ED25519_SERIAL},
    {SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1,           SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIZE,           SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1},
    {SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE, SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_SIZE, SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1_SIGNATURE},
    {SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL,        SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_SIZE,        SID_PAL_MFG_STORE_OFFSET_PRODUCT_P256R1_SERIAL},
    {SID_PAL_MFG_STORE_MAN_PUB_ED25519,              SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIZE,              SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519},
    {SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE,    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_SIZE,    SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519_SIGNATURE},
    {SID_PAL_MFG_STORE_MAN_ED25519_SERIAL,           SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_SIZE,           SID_PAL_MFG_STORE_OFFSET_MAN_ED25519_SERIAL},
    {SID_PAL_MFG_STORE_MAN_PUB_P256R1,               SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIZE,               SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1},
    {SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE,     SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_SIZE,     SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1_SIGNATURE},
    {SID_PAL_MFG_STORE_MAN_P256R1_SERIAL,            SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_SIZE,            SID_PAL_MFG_STORE_OFFSET_MAN_P256R1_SERIAL},
    {SID_PAL_MFG_STORE_SW_PUB_ED25519,               SID_PAL_MFG_STORE_SW_PUB_ED25519_SIZE,               SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519},
    {SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE,     SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_SIZE,     SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519_SIGNATURE},
    {SID_PAL_MFG_STORE_SW_ED25519_SERIAL,            SID_PAL_MFG_STORE_SW_ED25519_SERIAL_SIZE,            SID_PAL_MFG_STORE_OFFSET_SW_ED25519_SERIAL},
    {SID_PAL_MFG_STORE_SW_PUB_P256R1,                SID_PAL_MFG_STORE_SW_PUB_P256R1_SIZE,                SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1},
    {SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE,      SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_SIZE,      SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1_SIGNATURE},
    {SID_PAL_MFG_STORE_SW_P256R1_SERIAL,             SID_PAL_MFG_STORE_SW_P256R1_SERIAL_SIZE,             SID_PAL_MFG_STORE_OFFSET_SW_P256R1_SERIAL},
    {SID_PAL_MFG_STORE_AMZN_PUB_ED25519,             SID_PAL_MFG_STORE_AMZN_PUB_ED25519_SIZE,             SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_ED25519},
    {SID_PAL_MFG_STORE_AMZN_PUB_P256R1,              SID_PAL_MFG_STORE_AMZN_PUB_P256R1_SIZE,              SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_P256R1},
};
// clang-format on

static uint32_t app_value_to_offset(int value);
static sid_pal_mfg_store_region_t nrf_mfg_store_region = {
	.app_value_to_offset = app_value_to_offset,
};

static const struct device *flash_dev;
static struct flash_parameters flash_params;

/**
 * @brief Search for tag value in mfg tlv.
 * 
 * @param tag [in] tlv tag (type) to look for
 * @param tlv_info [out] tag, length and value of found tag.
 * @return true if tag was found.
 * @return false if no tag found.
 */
static bool sid_pal_mfg_store_search_for_tag(uint16_t tag,
					     struct sid_pal_mfg_store_tlv_info *tlv_info)
{
	off_t address = (off_t)(nrf_mfg_store_region.addr_start +
				SID_PAL_MFG_STORE_OFFSET_VERSION * WORD_SIZE +
				SID_PAL_MFG_STORE_VERSION_SIZE);

	uint16_t current_tag, length;
	uint8_t header_raw[MFG_STORE_TLV_HEADER_SIZE] = { 0 };

	while (1) {
		/* Read next header (tag & length) */
		int rc = flash_read(flash_dev, address, header_raw, MFG_STORE_TLV_HEADER_SIZE);
		if (0 != rc) {
			LOG_ERR("Flash read fail %d", rc);
			return false;
		}
		current_tag = (header_raw[0] << 8) + header_raw[1];
		length = (header_raw[2] << 8) + header_raw[3];

		if (current_tag == tag) {
			/* Tag found */
			tlv_info->tag = tag;
			tlv_info->length = length;
			tlv_info->offset = address;
			return true;
		}

		if (current_tag == MFG_STORE_TLV_EMPTY_TAG_VALUE) {
			break;
		}

		/* Go to the next TLV */
		address += (MFG_STORE_TLV_HEADER_SIZE +
			    ROUND_UP(length, WORD_SIZE));

		if ((uintptr_t)(address + MFG_STORE_TLV_HEADER_SIZE + WORD_SIZE) >
		    nrf_mfg_store_region.addr_end) {
			break;
		}
	}
	return false;
}

/**
 * @brief The 'dummy' function used when application doesn't provide it's own
 *          implementation for this function.
 *
 * @param value - unused argument.
 * @return SID_PAL_MFG_STORE_INVALID_OFFSET
 */
static uint32_t app_value_to_offset(int value)
{
	ARG_UNUSED(value);
	LOG_WRN("No support for app_value_to_offset.");
	return SID_PAL_MFG_STORE_INVALID_OFFSET;
}

void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region)
{
	nrf_mfg_store_region = mfg_store_region;

	if (!nrf_mfg_store_region.app_value_to_offset) {
		nrf_mfg_store_region.app_value_to_offset = app_value_to_offset;
	}

	flash_dev = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_flash_controller));
	if (!flash_dev) {
		LOG_ERR("Flash device is not found.");
	}

	const struct flash_parameters *params = flash_get_parameters(flash_dev);
	memcpy(&flash_params, params, sizeof(struct flash_parameters));

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	sid_mfg_storage_secure_init();
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

	int err = fprotect_area(PM_MFG_STORAGE_ADDRESS, PM_MFG_STORAGE_SIZE);
	if (err) {
		LOG_ERR("Flash protect failed %d", err);
	}
}

void sid_pal_mfg_store_deinit(void)
{
	memset(&nrf_mfg_store_region, 0, sizeof(sid_pal_mfg_store_region_t));
}

int32_t sid_pal_mfg_store_write(uint16_t value, const uint8_t *buffer, uint16_t length)
{
	if (value == MFG_STORE_TLV_EMPTY_TAG_VALUE || !buffer || !length) {
		return (int32_t)SID_ERROR_INVALID_ARGS;
	}

	struct sid_pal_mfg_store_tlv_info tlv_info = { 0 };
	bool found = false;
	uintptr_t address = 0;
	uint16_t length_aligned = ROUND_UP(length, flash_params.write_block_size);
	int err = 0;

	/* mfg storage doesn't allow overwrites */
	if (sid_pal_mfg_store_search_for_tag(value, &tlv_info)) {
		LOG_ERR("tag value already exists: %d", value);
		return -1;
	}

	/* search for empty space */
	found = sid_pal_mfg_store_search_for_tag(MFG_STORE_TLV_EMPTY_TAG_VALUE, &tlv_info);
	if (!found) {
		LOG_ERR("MFG storage is full");
		return -1;
	}

	address = (uintptr_t)tlv_info.offset;
	if (address + MFG_STORE_TLV_HEADER_SIZE + length_aligned > nrf_mfg_store_region.addr_end) {
		LOG_ERR("Not enough space to store: %d", value);
		return -1;
	}

	/* write tag value and length */
	uint8_t header_buffer[MFG_STORE_TLV_HEADER_SIZE] = { 0 };
	header_buffer[0] = value >> 8;
	header_buffer[1] = value;
	header_buffer[2] = length_aligned >> 8;
	header_buffer[3] = length_aligned;

	err = flash_write(flash_dev, address, header_buffer, MFG_STORE_TLV_HEADER_SIZE);
	if (err != 0) {
		return err;
	}
	address += MFG_STORE_TLV_HEADER_SIZE;

	/* write data */
	uint8_t write_buffer[length_aligned];
	memset(write_buffer, flash_params.erase_value, length_aligned);
	memcpy(write_buffer, buffer, length);
	err = flash_write(flash_dev, address, write_buffer, length_aligned);
	if (err != 0) {
		LOG_ERR("Flash write filed for tag %d (code %d)", value, err);
		LOG_HEXDUMP_DBG(buffer, length, "buffer: ");
		return err;
	}

	return 0;
}

void sid_pal_mfg_store_read(uint16_t value, uint8_t *buffer, uint16_t length)
{
#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	int err_sec = sid_mfg_storage_secure_read(&value, buffer, length);
	if (err_sec != -ENOENT) {
		LOG_DBG("secure read tag %d status %d", value, err_sec);
		return;
	}
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

	struct sid_pal_mfg_store_tlv_info tlv_info;
	bool found = sid_pal_mfg_store_search_for_tag(value, &tlv_info);
	if (!found) {
		LOG_ERR("tag value not found %d", value);
		return;
	}

	if (length > tlv_info.length) {
		LOG_ERR("invalid length %d of tag value %d", length, value);
		return;
	}

	int err =
		flash_read(flash_dev, tlv_info.offset + MFG_STORE_TLV_HEADER_SIZE, buffer, length);
	if (err) {
		LOG_ERR("Flash read fail %d", err);
		memset(buffer, flash_params.erase_value, length);
	}
}

uint16_t sid_pal_mfg_store_get_length_for_value(uint16_t value)
{
	uint16_t length = 0;
	struct sid_pal_mfg_store_tlv_info tlv_info;
	if (sid_pal_mfg_store_search_for_tag(value, &tlv_info)) {
		length = tlv_info.length;
	}

	return length;
}

int32_t sid_pal_mfg_store_erase(void)
{
#if CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC
	const size_t mfg_size = nrf_mfg_store_region.addr_end - nrf_mfg_store_region.addr_start;
	if (flash_dev) {
		return (int32_t)flash_erase(flash_dev, nrf_mfg_store_region.addr_start, mfg_size);
	}
	LOG_ERR("MFG store is not initialized");
	return (int32_t)SID_ERROR_UNINITIALIZED;
#else
	return (int32_t)SID_ERROR_NOSUPPORT;
#endif /* CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC */
}

bool sid_pal_mfg_store_is_empty(void)
{
#if CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC
	if (!flash_dev) {
		LOG_ERR("MFG store is not initialized.");
		return false;
	}

	uint8_t empty_flash_mem[FLASH_MEM_CHUNK];
	uint8_t tmp_buff[FLASH_MEM_CHUNK];
	size_t length = sizeof(tmp_buff);
	int rc;

	memset(empty_flash_mem, flash_params.erase_value, sizeof(empty_flash_mem));
	for (off_t offset = nrf_mfg_store_region.addr_start; offset < nrf_mfg_store_region.addr_end;
	     offset += length) {
		if ((offset + length) > nrf_mfg_store_region.addr_end) {
			length = nrf_mfg_store_region.addr_end - offset;
		}

		rc = flash_read(flash_dev, offset, tmp_buff, length);
		if (0 != rc) {
			LOG_ERR("Read flash memory error: %d.", rc);
			return false;
		}

		if (0 != memcmp(empty_flash_mem, tmp_buff, length)) {
			return false;
		}
	}
	return true;
#else
	LOG_WRN("The sid_pal_mfg_store_is_empty function is not enabled.");
	return false;
#endif /* CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC */
}

bool sid_pal_mfg_store_is_tlv_support(void)
{
	return true;
}

uint32_t sid_pal_mfg_store_get_version(void)
{
	uint32_t version = 0;

	if (flash_dev) {
		int rc = flash_read(flash_dev,
				    nrf_mfg_store_region.addr_start +
					    (SID_PAL_MFG_STORE_OFFSET_VERSION * WORD_SIZE),
				    (uint8_t *)&version, SID_PAL_MFG_STORE_VERSION_SIZE);
		if (0 != rc) {
			LOG_ERR("Flash read fail %d", rc);
		}
	} else {
		LOG_ERR("MFG store is not initialized.");
	}

	return sys_be32_to_cpu(version);
}

/* Functions specific to Sidewalk with special handling */

bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE])
{
	bool dev_id_found = false;

	if (dev_id) {
		uint8_t unset_dev_id[SID_PAL_MFG_STORE_DEVID_SIZE];
		memset(unset_dev_id, flash_params.erase_value, SID_PAL_MFG_STORE_DEVID_SIZE);
		memset(dev_id, flash_params.erase_value, SID_PAL_MFG_STORE_DEVID_SIZE);
		sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVID, dev_id,
				       SID_PAL_MFG_STORE_DEVID_SIZE);

		if (0 == memcmp(dev_id, unset_dev_id, SID_PAL_MFG_STORE_DEVID_SIZE)) {
			uint32_t mcu_devid = DEV_ID_REG;
			dev_id[0] = 0xBF;
			mcu_devid = sys_cpu_to_be32(mcu_devid);
			memcpy(&dev_id[1], &mcu_devid, sizeof(mcu_devid));
		}
	}
	return dev_id_found;
}

bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE])
{
	uint8_t unset_serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE];

	if (!serial_num) {
		return false;
	}

	memset(unset_serial_num, flash_params.erase_value, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
	memset(serial_num, flash_params.erase_value, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SERIAL_NUM, serial_num,
			       SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

	if (0 == memcmp(serial_num, unset_serial_num, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE)) {
		return false;
	}

	return true;
}

void sid_pal_mfg_store_apid_get(uint8_t apid[SID_PAL_MFG_STORE_APID_SIZE])
{
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_APID, apid, SID_PAL_MFG_STORE_APID_SIZE);
}

void sid_pal_mfg_store_app_pub_key_get(uint8_t app_pub[SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE])
{
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_APP_PUB_ED25519, app_pub,
			       SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE);
}

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#define SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_RAW (100)
#define SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_RAW (101)
#define SID_PAL_MFG_STORE_PRIV_KEY_SIZE (32)

static int sid_mfg_storage_secure_read(uint16_t *p_value, uint8_t *buffer, uint16_t length)
{
	int err = -ENOENT;
	switch (*p_value) {
	case SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519:
		err = sid_crypto_keys_buffer_set(SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID, buffer,
						 length);
		break;
	case SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1:
		err = sid_crypto_keys_buffer_set(SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID, buffer,
						 length);
		break;
	case SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_RAW:
		*p_value = SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519;
		break;
	case SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_RAW:
		*p_value = SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1;
		break;
	}

	return err;
}

static int sid_mfg_storage_secure_clean_key_data(uint16_t value)
{
	// Supports only private keys
	uint16_t tag = 0;
	switch (value) {
	case SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_RAW:
		tag = SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519;
		break;
	case SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_RAW:
		tag = SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1;
		break;
	default:
		return -ENOENT;
	}

	// Overwrites with zeros
	struct sid_pal_mfg_store_tlv_info tlv_info = {};
	uintptr_t address;
	uint8_t __aligned(4) zeros[SID_PAL_MFG_STORE_PRIV_KEY_SIZE] = { 0 };
	int err = 0;

	bool found = sid_pal_mfg_store_search_for_tag(tag, &tlv_info);
	if (!found) {
		return -EIO;
	}
	address = (uintptr_t)tlv_info.offset + MFG_STORE_TLV_HEADER_SIZE;

	err = flash_write(flash_dev, address, zeros, SID_PAL_MFG_STORE_PRIV_KEY_SIZE);
	if (err != 0) {
		return err;
	}

	return 0;
}

static void sid_mfg_storage_secure_init(void)
{
	int err = 0;
	uint8_t raw_key[SID_PAL_MFG_STORE_PRIV_KEY_SIZE];
	uint8_t zeros[SID_PAL_MFG_STORE_PRIV_KEY_SIZE] = { 0 };

	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_RAW, raw_key, sizeof(raw_key));
	if (0 != memcmp(raw_key, zeros, sizeof(raw_key))) {
		err = sid_crypto_keys_new_import(SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID, raw_key,
						 sizeof(raw_key));
		LOG_INF("MFG_ED25519 import %s", (0 == err) ? "success" : "failure");
		LOG_HEXDUMP_INF(raw_key, sizeof(raw_key), "value:");

		memset(raw_key, 0, sizeof(raw_key));
		err = sid_mfg_storage_secure_clean_key_data(
			SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_RAW);
		LOG_INF("MFG_ED25519 clean status %d", err);
	}

	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_RAW, raw_key, sizeof(raw_key));
	if (0 != memcmp(raw_key, zeros, sizeof(raw_key))) {
		err = sid_crypto_keys_new_import(SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID, raw_key,
						 sizeof(raw_key));
		LOG_INF("MFG_SECP_256R1 import %s", (0 == err) ? "success" : "failure");
		LOG_HEXDUMP_INF(raw_key, sizeof(raw_key), "value:");

		memset(raw_key, 0, sizeof(raw_key));
		err = sid_mfg_storage_secure_clean_key_data(
			SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_RAW);
		LOG_INF("MFG_SECP_256R1 clean status %d", err);
	}
}
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */
