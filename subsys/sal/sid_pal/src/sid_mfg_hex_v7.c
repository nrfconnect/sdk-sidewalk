/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <sid_mfg_hex_parsers.h>
#include <sid_hal_memory_ifc.h>
#include <stdint.h>
#include <string.h>
#include <tlv/tlv.h>
#include <tlv/tlv_storage_impl.h>
#include <sid_pal_mfg_store_ifc.h>
#include <mfg_store_offsets.h>
#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#include <sid_crypto_keys.h>
#endif
LOG_MODULE_REGISTER(sid_mfg_parser_v7, CONFIG_SIDEWALK_LOG_LEVEL);

struct sid_pal_mfg_store_value_to_address_offset {
	sid_pal_mfg_store_value_t value;
	uint16_t size;
	uint32_t offset;
};

// clang-format off
struct sid_pal_mfg_store_value_to_address_offset sid_pal_mfg_store_app_value_to_offset_table[] = {
    {SID_PAL_MFG_STORE_SERIAL_NUM,                   SID_PAL_MFG_STORE_SERIAL_NUM_SIZE,                   SID_PAL_MFG_STORE_OFFSET_SERIAL_NUM},
    {SID_PAL_MFG_STORE_SMSN,                         SID_PAL_MFG_STORE_SMSN_SIZE,                         SID_PAL_MFG_STORE_OFFSET_SMSN},
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
    {SID_PAL_MFG_STORE_APID,                         SID_PAL_MFG_STORE_APID_SIZE,                         SID_PAL_MFG_STORE_OFFSET_APID},
};
// clang-format on

int parse_mfg_const_offsets(tlv_ctx *tlv)
{
	if (tlv->end_offset <= tlv->start_offset) {
		return -EINVAL;
	}

	uint32_t size = tlv->end_offset - tlv->start_offset;

	uint8_t *ram_tlv_data = sid_hal_malloc(size);
	if (ram_tlv_data == NULL) {
		return -ENOMEM;
	}
	memset(ram_tlv_data, 0xff, size);

	tlv_ctx ram_tlv = { .start_offset = 0,
			    .end_offset = size,
			    .tlv_storage_start_marker_size = tlv->tlv_storage_start_marker_size,
			    .storage_impl = { .ctx = ram_tlv_data,
					      .read = tlv_storage_ram_read,
					      .write = tlv_storage_ram_write } };

	struct mfg_header mfg_header = { .magic_string = MFG_HEADER_MAGIC,
					 .raw_version = { 0, 0, 0, REPORTED_VERSION } };

	int ret =
		tlv_write_start_marker(&ram_tlv, (uint8_t *)&mfg_header, sizeof(struct mfg_header));
	if (ret != 0) {
		LOG_ERR("Failed to write marker before mfg data");
		sid_hal_free(ram_tlv_data);
		return -EIO;
	}

	uint8_t payload_buffer[CONFIG_SIDEWALK_MFG_PARSER_MAX_ELEMENT_SIZE] = { 0 };

	for (int i = 0; i < ARRAY_SIZE(sid_pal_mfg_store_app_value_to_offset_table); i++) {
		struct sid_pal_mfg_store_value_to_address_offset element =
			sid_pal_mfg_store_app_value_to_offset_table[i];
		int ret = tlv->storage_impl.read(tlv->storage_impl.ctx,
						 tlv->start_offset + (element.offset * WORD_SIZE),
						 payload_buffer, element.size);
		if (ret != 0) {
			LOG_ERR("Failed to read data");
			sid_hal_free(ram_tlv_data);
			return -EIO;
		}
		if (element.size > SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN ||
		    memcmp(payload_buffer,
			   (char[]){ [0 ...(SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN - 1)] = 0xFF },
			   element.size) == 0) {
			// ignore empty values
			continue;
		}

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
		if (element.value == SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519) {
			int err = sid_crypto_keys_new_import(SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID,
							     payload_buffer, element.size);
			LOG_INF("MFG_ED25519 import %s", (0 == err) ? "success" : "failure");

			if (err != 0) {
				sid_hal_free(ram_tlv_data);
				return -EACCES;
			}
			continue;
		} else if (element.value == SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1) {
			int err = sid_crypto_keys_new_import(SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID,
							     payload_buffer, element.size);
			LOG_INF("MFG_SECP_256R1 import %s", (0 == err) ? "success" : "failure");

			if (err != 0) {
				sid_hal_free(ram_tlv_data);
				return -EACCES;
			}
			continue;
		}
#endif
		ret = tlv_write(&ram_tlv, element.value, payload_buffer, element.size);
		if (ret != 0) {
			LOG_ERR("Failed to write data");
			sid_hal_free(ram_tlv_data);
			return -EIO;
		}
	}

	struct mfg_flags flags = {
		.initialized = 1,
#if CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
		.keys_in_psa = 1,
#endif
	};
	ret = tlv_write(&ram_tlv, MFG_FLAGS_TYPE_ID, (uint8_t *)&flags, sizeof(flags));
	if (ret != 0) {
		LOG_ERR("Failed to write data");
		sid_hal_free(ram_tlv_data);
		return -EIO;
	}

	ret = tlv->storage_impl.erase(tlv->storage_impl.ctx, tlv->start_offset, size);
	if (ret != 0) {
		LOG_ERR("Failed to erase flash storage");
		sid_hal_free(ram_tlv_data);
		return -EIO;
	}

	ret = tlv->storage_impl.write(tlv->storage_impl.ctx, tlv->start_offset, ram_tlv_data, size);
	if (ret != 0) {
		LOG_ERR("Failed to write parsed tlv data to flash");
		sid_hal_free(ram_tlv_data);
		return -EIO;
	}
	sid_hal_free(ram_tlv_data);
	return 0;
}
