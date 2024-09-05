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

#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <fprotect.h>
#include <pm_config.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>

#include <tlv/tlv.h>
#include <tlv/tlv_storage_impl.h>
#include <sid_mfg_hex_parsers.h>

LOG_MODULE_REGISTER(sid_mfg, CONFIG_SIDEWALK_LOG_LEVEL);

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#include <sid_crypto_keys.h>
static int sid_mfg_storage_secure_read(uint16_t *p_value, uint8_t *buffer, uint16_t length);
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

static const struct device *flash_dev;
static uint32_t sid_mfg_version = INVALID_VERSION;
tlv_ctx tlv_flash;

void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region)
{
	/* Initialize Flash device*/
	flash_dev = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_flash_controller));
	if (!flash_dev) {
		LOG_ERR("Flash device is not found.");
		return;
	}

	tlv_flash = (tlv_ctx){ .storage_impl = { .write = tlv_storage_flash_write,
						 .erase = tlv_storage_flash_erase,
						 .read = tlv_storage_flash_read,
						 .ctx = (void *)flash_dev },
			       .start_offset = mfg_store_region.addr_start,
			       .end_offset = mfg_store_region.addr_end,
			       .tlv_storage_start_marker_size = sizeof(struct mfg_header) };

	struct mfg_header header = { 0 };
	int ret = tlv_read_start_marker(&tlv_flash, (uint8_t *)&header, sizeof(header));
	if (ret != 0 ||
	    strncmp(header.magic_string, MFG_HEADER_MAGIC, strlen(MFG_HEADER_MAGIC)) != 0) {
		if (ret != 0) {
			LOG_ERR("Failed to read mfg start marker errno %d", ret);
		}
#if CONFIG_SIDEWALK_ON_DEV_CERT
		LOG_INF("Please perform on_device_certification and reboot");
#endif
		return;
	}

	sid_mfg_version = header.raw_version[3] + (header.raw_version[2] << 8) +
			  (header.raw_version[1] << 16) + ((header.raw_version[0]) << 24);

	bool need_to_parse = false;
	if (sid_mfg_version == SID_PAL_MFG_STORE_TLV_VERSION) {
		// header is valid, version is supported
		struct mfg_flags flags = {};
		int ret = tlv_read(&tlv_flash, MFG_FLAGS_TYPE_ID, (uint8_t *)&flags, sizeof(flags));
		switch (ret) {
		case -ENODATA:
			need_to_parse = true;
			break;
		case 0:
			break;
		default: {
			LOG_ERR("Failed to read mfg data errno: %d", ret);
			return;
		}
		}

#if CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
		if ((flags.keys_in_psa) == 0) {
			// parse to move keys to psa
			need_to_parse = true;
		}
#else
		if (flags.keys_in_psa) {
			LOG_ERR("Replace mfg hex. Keys are stored in PSA, but application is not build to support it.");
			return;
		}
#endif
		if (flags.initialized == 0) {
			need_to_parse = true;
		}

	} else {
		need_to_parse = true;
	}

	if (need_to_parse) {
		LOG_INF("Need to parse MFG data");
		int err = 0;
		switch (sid_mfg_version) {
		case 8:
			err = parse_mfg_raw_tlv(&tlv_flash);
			break;
#if CONFIG_SIDEWALK_MFG_STORAGE_SUPPORT_HEX_v7
		case 1 ... 7:
		case 0x01000000:
			err = parse_mfg_const_offsets(&tlv_flash);
			break;
#endif /* CONFIG_SIDEWALK_MFG_STORAGE_SUPPORT_HEX_v7 */
		default: {
			LOG_ERR("Incompatible MFG hex version");
			return;
		}
		}
		if (err != 0) {
			LOG_ERR("Failed parsing MFG hex errno %d", err);
			return;
		}
		LOG_INF("MFG data succesfully parsed");
		sid_mfg_version = SID_PAL_MFG_STORE_TLV_VERSION;
	}
}

void sid_pal_mfg_store_deinit(void)
{
	memset(&tlv_flash, 0x0, sizeof(tlv_flash));
}

int32_t sid_pal_mfg_store_write(uint16_t value, const uint8_t *buffer, uint16_t length)
{
	if (value == SID_PAL_MFG_STORE_VERSION) {
		if (length != SID_PAL_MFG_STORE_VERSION_SIZE) {
			return -EINVAL;
		}
		struct mfg_header mfg_header = { .magic_string = MFG_HEADER_MAGIC };
		memcpy(mfg_header.raw_version, buffer, SID_PAL_MFG_STORE_VERSION_SIZE);
		return tlv_write_start_marker(&tlv_flash, (uint8_t *)&mfg_header,
					      sizeof(struct mfg_header));
	}

#if CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC
	return tlv_write(&tlv_flash, value, buffer, length);
#else
	return (int32_t)SID_ERROR_NOSUPPORT;
#endif
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
	if (value == SID_PAL_MFG_STORE_VERSION) {
		memcpy(buffer, (uint8_t[]){ 0, 0, 0, SID_PAL_MFG_STORE_TLV_VERSION },
		       SID_PAL_MFG_STORE_VERSION_SIZE);
		return;
	}
	int ret = tlv_read(&tlv_flash, value, buffer, length);
	if (ret != 0) {
		LOG_ERR("Failed to read tlv type %d with errno %d", value, ret);
	}
}

uint16_t sid_pal_mfg_store_get_length_for_value(uint16_t value)
{
	tlv_header header = {};

	int ret = tlv_lookup(&tlv_flash, value, &header);
	if (ret != 0) {
		LOG_ERR("Failed to find value %d in MFG storage errno: %d", value, ret);
		return 0;
	}
	return header.payload_size.data_size;
}

int32_t sid_pal_mfg_store_erase(void)
{
#if CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC
	const size_t mfg_size = tlv_flash.end_offset - tlv_flash.start_offset;
	return tlv_flash.storage_impl.erase(tlv_flash.storage_impl.ctx, tlv_flash.start_offset,
					    mfg_size);
#else
	return (int32_t)SID_ERROR_NOSUPPORT;
#endif /* CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC */
}

bool sid_pal_mfg_store_is_empty(void)
{
#if CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC
	// read header, if failed to read magic, it is empty

	uint8_t empty_flash_mem[FLASH_MEM_CHUNK];
	uint8_t tmp_buff[FLASH_MEM_CHUNK];
	size_t length = sizeof(tmp_buff);
	int rc;
	const struct flash_parameters *flash_params;

	if (!flash_dev) {
		LOG_ERR("No flash device to erase.");
		return false;
	}

	flash_params = flash_get_parameters(flash_dev);
	memset(empty_flash_mem, flash_params->erase_value, sizeof(empty_flash_mem));
	for (off_t offset = tlv_flash.start_offset; offset < tlv_flash.end_offset;
	     offset += length) {
		if ((offset + length) > tlv_flash.end_offset) {
			length = tlv_flash.end_offset - offset;
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
	return sid_mfg_version;
}

/* Functions specific to Sidewalk with special handling */

bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE])
{
	uint32_t mcu_devid = DEV_ID_REG;
	dev_id[0] = 0xBF;
	mcu_devid = sys_cpu_to_be32(mcu_devid);
	memcpy(&dev_id[1], &mcu_devid, sizeof(mcu_devid));
	return true;
}

bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE])
{
	int ret = tlv_read(&tlv_flash, SID_PAL_MFG_STORE_SERIAL_NUM, serial_num,
			   SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
	return ret == 0;
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
	}

	return err;
}

#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */
