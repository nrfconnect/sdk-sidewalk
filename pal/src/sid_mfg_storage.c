/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_mfg_storage.c
 *  @brief Sidewalk MFG storage.
 */

#include <sid_pal_mfg_store_ifc.h>
#include <sid_error.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <zephyr.h>
#include <device.h>
#include <drivers/flash.h>
#include <storage/flash_map.h>
#include <mfg_store_offsets.h>
#include <sys/byteorder.h>
#include <sys/types.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(sid_mfg, CONFIG_SIDEWALK_LOG_LEVEL);

// Manufacturing version define
#define MFG_VERSION_1_VAL   0x01000000
#define MFG_VERSION_2_VAL   0x2

#define FLASH_MEM_CHUNK   (128)

#ifndef DEV_ID_REG
#if defined (NRF52840_XXAA) || defined (NRF52833_XXAA) || defined (NRF52832_XXAA)
#define DEV_ID_REG  (uint32_t)(NRF_FICR->DEVICEID[0])
#elif defined (NRF5340_XXAA)
#if defined(NRF_APPLICATION)
#define DEV_ID_REG  (uint32_t)(NRF_FICR_S->DEVICEID[0])
#elif defined (NRF_NETWORK)
#define DEV_ID_REG  (uint32_t)(NRF_FICR_NS->DEVICEID[0])
#endif /* NRF5340_XXAA */
#else
#error "Unknow Device ID register."
#endif
#endif /* DEV_ID_REG */

// DEV_ID masks
#define ENCODED_DEV_ID_SIZE_5_BYTES_MASK    0xA0
#define DEV_ID_MSB_MASK 0x1F

#define MFG_WORD_SIZE_VER_1     (8)

struct sid_pal_mfg_store_value_to_address_offset {
	sid_pal_mfg_store_value_t value;
	off_t offset;
};

static void ntoh_buff(uint8_t *buffer, size_t buff_len);
static uint32_t default_app_value_to_offset(int value);
static off_t checked_addr_return(off_t offset, uintptr_t start_address, uintptr_t end_address);
static off_t value_to_offset(sid_pal_mfg_store_value_t value, uintptr_t start_address, uintptr_t end_address);

struct sid_pal_mfg_store_value_to_address_offset sid_pal_mfg_store_app_value_to_offset_table[] = {
	{ SID_PAL_MFG_STORE_VERSION,                      SID_PAL_MFG_STORE_OFFSET_VERSION },
	{ SID_PAL_MFG_STORE_DEVID,                        SID_PAL_MFG_STORE_OFFSET_DEVID },
	{ SID_PAL_MFG_STORE_SERIAL_NUM,                   SID_PAL_MFG_STORE_OFFSET_SERIAL_NUM },
	{ SID_PAL_MFG_STORE_SMSN,                         SID_PAL_MFG_STORE_OFFSET_SMSN },
	{ SID_PAL_MFG_STORE_APID,                         SID_PAL_MFG_STORE_OFFSET_APID },
	{ SID_PAL_MFG_STORE_APP_PUB_ED25519,              SID_PAL_MFG_STORE_OFFSET_APP_PUB_ED25519 },
	{ SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,          SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_ED25519 },
	{ SID_PAL_MFG_STORE_DEVICE_PUB_ED25519,           SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519 },
	{ SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE, SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519_SIGNATURE },
	{ SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1,           SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_P256R1 },
	{ SID_PAL_MFG_STORE_DEVICE_PUB_P256R1,            SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1 },
	{ SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,  SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1_SIGNATURE },
	{ SID_PAL_MFG_STORE_DAK_PUB_ED25519,              SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519 },
	{ SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE,    SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519_SIGNATURE },
	{ SID_PAL_MFG_STORE_DAK_ED25519_SERIAL,           SID_PAL_MFG_STORE_OFFSET_DAK_ED25519_SERIAL },
	{ SID_PAL_MFG_STORE_DAK_PUB_P256R1,               SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1 },
	{ SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE,     SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1_SIGNATURE },
	{ SID_PAL_MFG_STORE_DAK_P256R1_SERIAL,            SID_PAL_MFG_STORE_OFFSET_DAK_P256R1_SERIAL },
	{ SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519,          SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519 },
	{ SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE, SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519_SIGNATURE },
	{ SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL,       SID_PAL_MFG_STORE_OFFSET_PRODUCT_ED25519_SERIAL },
	{ SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1,           SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1 },
	{ SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE, SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1_SIGNATURE },
	{ SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL,        SID_PAL_MFG_STORE_OFFSET_PRODUCT_P256R1_SERIAL },
	{ SID_PAL_MFG_STORE_MAN_PUB_ED25519,              SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519 },
	{ SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE,    SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519_SIGNATURE },
	{ SID_PAL_MFG_STORE_MAN_ED25519_SERIAL,           SID_PAL_MFG_STORE_OFFSET_MAN_ED25519_SERIAL },
	{ SID_PAL_MFG_STORE_MAN_PUB_P256R1,               SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1 },
	{ SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE,     SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1_SIGNATURE },
	{ SID_PAL_MFG_STORE_MAN_P256R1_SERIAL,            SID_PAL_MFG_STORE_OFFSET_MAN_P256R1_SERIAL },
	{ SID_PAL_MFG_STORE_SW_PUB_ED25519,               SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519 },
	{ SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE,     SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519_SIGNATURE },
	{ SID_PAL_MFG_STORE_SW_ED25519_SERIAL,            SID_PAL_MFG_STORE_OFFSET_SW_ED25519_SERIAL },
	{ SID_PAL_MFG_STORE_SW_PUB_P256R1,                SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1 },
	{ SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE,      SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1_SIGNATURE },
	{ SID_PAL_MFG_STORE_SW_P256R1_SERIAL,             SID_PAL_MFG_STORE_OFFSET_SW_P256R1_SERIAL },
	{ SID_PAL_MFG_STORE_AMZN_PUB_ED25519,             SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_ED25519 },
	{ SID_PAL_MFG_STORE_AMZN_PUB_P256R1,              SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_P256R1 },
};

static sid_pal_mfg_store_region_t nrf_mfg_store_region = {
	.app_value_to_offset = default_app_value_to_offset,
};

static const struct device *flash_dev;

/**
 * @brief The function converts network byte order to host byte order on the whole buffer.
 *
 * @param buffer - input/output buffer.
 * @param buff_len - number of bytes in buffer.
 */
static void ntoh_buff(uint8_t *buffer, size_t buff_len)
{
	uint32_t val_l = 0;
	size_t i = 0;
	size_t mod_len = buff_len % sizeof(uint32_t);

	if (sizeof(uint32_t) <= buff_len) {
		for (i = 0; i < (buff_len - mod_len); i += sizeof(uint32_t)) {
			memcpy(&val_l, &buffer[i], sizeof(val_l));
			val_l = sys_be32_to_cpu(val_l);
			memcpy(&buffer[i], &val_l, sizeof(val_l));
		}
	}

	if (2 == mod_len) {
		uint16_t val_s = 0;
		memcpy(&val_s, &buffer[i], sizeof(val_s));
		val_s = sys_be16_to_cpu(val_s);
		memcpy(&buffer[i], &val_s, sizeof(val_s));
	} else if (3 == mod_len) {
		val_l = 0;
		memcpy(&val_l, &buffer[i], 3 * sizeof(uint8_t));
		val_l = sys_be24_to_cpu(val_l);
		memcpy(&buffer[i], &val_l, 3 * sizeof(uint8_t));
	}
}

/**
 * @brief The 'dummy' function used when application doesn't provide it's own
 *          implementation for this function.
 *
 * @param value - unused argument.
 * @return SID_PAL_MFG_STORE_INVALID_OFFSET
 */
static uint32_t default_app_value_to_offset(int value)
{
	ARG_UNUSED(value);
	LOG_WRN("No support for app_value_to_offset.");
	return SID_PAL_MFG_STORE_INVALID_OFFSET;
}

/**
 * @brief The function checks if offset is in address range of the manufacturing store partition.
 *
 * @param offset - memory offset.
 * @param start_address - mfg partition start address.
 * @param end_address - mfg partition end address.
 * @return Offset in physical flash memory or SID_PAL_MFG_STORE_INVALID_OFFSET when offest is out
 *          of manufacturing storage address range.
 */
static off_t checked_addr_return(off_t offset, uintptr_t start_address, uintptr_t end_address)
{
	if (start_address + offset >= end_address) {
		LOG_ERR("Offset past manufacturing store end: %d.", (int)offset);
		return SID_PAL_MFG_STORE_INVALID_OFFSET;
	}
	return (off_t)(start_address + offset);
}

/**
 * @brief The function calculates memory offset for desired value.
 *
 * @param value - Enum constant for the desired value.
 * @param start_address - mfg partition start address.
 * @param end_address - mfg partition end address.
 * @return Offset in physical flash memory when success otherwise SID_PAL_MFG_STORE_INVALID_OFFSET.
 */
static off_t value_to_offset(sid_pal_mfg_store_value_t value, uintptr_t start_address, uintptr_t end_address)
{
	const size_t table_count = ARRAY_SIZE(sid_pal_mfg_store_app_value_to_offset_table);

	for (uint32_t i = 0; i < table_count; i++) {
		if (value == sid_pal_mfg_store_app_value_to_offset_table[i].value) {
			const off_t offset = sid_pal_mfg_store_app_value_to_offset_table[i].offset;
			return (off_t)((SID_PAL_MFG_STORE_INVALID_OFFSET != offset) ?
				       (start_address + (offset << 2)) : SID_PAL_MFG_STORE_INVALID_OFFSET);
		}
	}

	if (value < 0 || value >= SID_PAL_MFG_STORE_CORE_VALUE_MAX) {
		// This is not a core value. Search for this value among those provided by the application.
		off_t custom_offset = nrf_mfg_store_region.app_value_to_offset(value);
		if (SID_PAL_MFG_STORE_INVALID_OFFSET != custom_offset) {
			return checked_addr_return(custom_offset, start_address, end_address);
		}
	}

	LOG_ERR("No Nordic manufacturing store offset for: %d.", value);
	return SID_PAL_MFG_STORE_INVALID_OFFSET;
}

void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region)
{
	nrf_mfg_store_region = mfg_store_region;

	if (!nrf_mfg_store_region.app_value_to_offset) {
		nrf_mfg_store_region.app_value_to_offset = default_app_value_to_offset;
	}

	flash_dev = device_get_binding(DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
	if (!flash_dev) {
		LOG_ERR("Flash device is not found.");
	}
}

int32_t sid_pal_mfg_store_write(int value, const uint8_t *buffer, uint8_t length)
{
#if defined (HALO_ENABLE_DIAGNOSTICS) && HALO_ENABLE_DIAGNOSTICS
	uint8_t __aligned(4) wr_array[SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN];

	if (0u == length) {
		return (int32_t)SID_ERROR_INVALID_ARGS;
	}

	if (length > sizeof(wr_array)) {
		return (int32_t)SID_ERROR_OUT_OF_RESOURCES;
	}

	if (length % sizeof(uint32_t)) {
		LOG_WRN("Length is not word-aligned.");
		return (int32_t)SID_ERROR_INCOMPATIBLE_PARAMS;
	}

	const off_t value_offset = value_to_offset(
		value, nrf_mfg_store_region.addr_start, nrf_mfg_store_region.addr_end);

	if (SID_PAL_MFG_STORE_INVALID_OFFSET == value_offset) {
		return (int32_t)SID_ERROR_NOT_FOUND;
	}

	if (NULL == buffer) {
		return (int32_t)SID_ERROR_NULL_POINTER;
	}

	memcpy(wr_array, buffer, length);
	if (flash_dev) {
		return (int32_t)flash_write(flash_dev, value_offset, wr_array, length);
	}

	return (int32_t)SID_ERROR_UNINITIALIZED;
#else
	return (int32_t)SID_ERROR_NOSUPPORT;
#endif
}

void sid_pal_mfg_store_read(int value, uint8_t *buffer, uint8_t length)
{
	const off_t value_offset = value_to_offset(
		value, nrf_mfg_store_region.addr_start, nrf_mfg_store_region.addr_end);

	if (!buffer) {
		LOG_ERR("Null pointer provided.");
		return;
	}

	if (SID_PAL_MFG_STORE_INVALID_OFFSET != value_offset) {
		if (flash_dev) {
			int rc = flash_read(flash_dev, value_offset, buffer, length);
			if (0 != rc) {
				LOG_ERR("Flash read fail %d", rc);
			}
		} else {
			LOG_ERR("MFG store is not initialized.");
		}
	}
}

int32_t sid_pal_mfg_store_erase(void)
{
#if defined (HALO_ENABLE_DIAGNOSTICS) && HALO_ENABLE_DIAGNOSTICS
	const size_t mfg_size = nrf_mfg_store_region.addr_end - nrf_mfg_store_region.addr_start;
	if (flash_dev) {
		return (int32_t)flash_erase(flash_dev, nrf_mfg_store_region.addr_start, mfg_size);
	}
	LOG_ERR("MFG store is not initialized");
	return (int32_t)SID_ERROR_UNINITIALIZED;
#else
	return (int32_t)SID_ERROR_NOSUPPORT;
#endif
}

bool sid_pal_mfg_store_is_empty(void)
{
#if defined (HALO_ENABLE_DIAGNOSTICS) && HALO_ENABLE_DIAGNOSTICS
	uint8_t empty_flash_mem[FLASH_MEM_CHUNK];
	uint8_t tmp_buff[FLASH_MEM_CHUNK];
	size_t length = sizeof(tmp_buff);

	memset(empty_flash_mem, 0xFF, sizeof(empty_flash_mem));

	if (flash_dev) {
		int rc;
		for (off_t offset = nrf_mfg_store_region.addr_start;
		     offset < nrf_mfg_store_region.addr_end;
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
	} else {
		LOG_ERR("MFG store is not initialized.");
	}
#else
	LOG_WRN("The sid_pal_mfg_store_is_empty function is not enabled.");
#endif
	return false;
}

uint32_t sid_pal_mfg_store_get_version(void)
{
	uint32_t version;

	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_VERSION,
			       (uint8_t *)&version, SID_PAL_MFG_STORE_VERSION_SIZE);
	// Assuming that we keep this behavior for both 1P & 3P
	return sys_be32_to_cpu(version);
}

bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE])
{
	bool dev_id_found = false;

	if (dev_id) {
		uint8_t unset_dev_id[SID_PAL_MFG_STORE_DEVID_SIZE];
		memset(unset_dev_id, 0xFF, SID_PAL_MFG_STORE_DEVID_SIZE);
		memset(dev_id, 0xFF, SID_PAL_MFG_STORE_DEVID_SIZE);
		sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVID,
				       dev_id, SID_PAL_MFG_STORE_DEVID_SIZE);

		if (0 == memcmp(dev_id, unset_dev_id, SID_PAL_MFG_STORE_DEVID_SIZE)) {
			uint32_t mcu_devid = DEV_ID_REG;
			dev_id[0] = 0xBF;
			mcu_devid = sys_cpu_to_be32(mcu_devid);
			memcpy(&dev_id[1], &mcu_devid, sizeof(mcu_devid));
		} else {
			const uint32_t version = sid_pal_mfg_store_get_version();
			if (MFG_VERSION_1_VAL == version || 0x1 == version) {
				/**
				 * Correct dev_id for mfg version 1
				 * For devices with mfg version 1, the device Id is stored as two words
				 * in network endian format.
				 * To read the device Id two words at SID_PAL_MFG_STORE_DEVID has to be
				 * read and each word needs to be changed to host endian format.
				 */
				uint8_t dev_id_buffer[MFG_WORD_SIZE_VER_1];
				sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVID, dev_id_buffer, sizeof(dev_id_buffer));
				ntoh_buff(dev_id_buffer, sizeof(dev_id_buffer));
				// Encode the size in the first 3 bits in MSB of the devId
				dev_id_buffer[0] = (dev_id_buffer[0] & DEV_ID_MSB_MASK) | ENCODED_DEV_ID_SIZE_5_BYTES_MASK;
				memcpy(dev_id, dev_id_buffer, SID_PAL_MFG_STORE_DEVID_SIZE);
			}
			dev_id_found = true;
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

	memset(unset_serial_num, 0xFF, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
	memset(serial_num, 0xFF, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SERIAL_NUM,
			       serial_num, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

	if (0 == memcmp(serial_num, unset_serial_num, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE)) {
		return false;
	}

	const uint32_t version = sid_pal_mfg_store_get_version();
	if (MFG_VERSION_1_VAL == version || 0x1 == version) {
		ntoh_buff(serial_num, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
	}
	return true;
}
