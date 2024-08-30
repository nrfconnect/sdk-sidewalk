/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <sid_mfg_hex_parsers.h>
#include <sid_hal_memory_ifc.h>
#include <stdint.h>
#include <tlv/tlv.h>
#include <tlv/tlv_storage_impl.h>
#include <zephyr/sys/byteorder.h>
#include <string.h>
#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#include <sid_pal_mfg_store_ifc.h>
#include <sid_crypto_keys.h>
#endif

#define MFG_STORE_TLV_TAG_EMPTY 0xFFFF

LOG_MODULE_REGISTER(sid_mfg_parser_v8, CONFIG_SIDEWALK_LOG_LEVEL);

int parse_mfg_raw_tlv(tlv_ctx *tlv)
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

	uint32_t offset = tlv->start_offset + tlv->tlv_storage_start_marker_size;

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
	for (; offset < tlv->end_offset;) {
		uint8_t key[2];
		uint8_t size[2];
		int ret = tlv->storage_impl.read(tlv->storage_impl.ctx, offset, key, sizeof(key));
		if (ret != 0) {
			LOG_ERR("Failed to read data");
			sid_hal_free(ram_tlv_data);
			return -EIO;
		}
		offset += 2;
		uint16_t key_decoded = (key[0] << 8) + key[1];
		if (key_decoded == MFG_STORE_TLV_TAG_EMPTY) {
			break;
		}
		ret = tlv->storage_impl.read(tlv->storage_impl.ctx, offset, size, sizeof(size));
		if (ret != 0) {
			LOG_ERR("Failed to read data");
			sid_hal_free(ram_tlv_data);
			return -EIO;
		}
		offset += 2;
		uint16_t size_decoded = (size[0] << 8) | size[1];
		ret = tlv->storage_impl.read(tlv->storage_impl.ctx, offset, payload_buffer,
					     size_decoded);
		if (ret != 0) {
			LOG_ERR("Failed to read data");
			sid_hal_free(ram_tlv_data);
			return -EIO;
		}
		offset += size_decoded;

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
		if (key_decoded == SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519) {
			int err = sid_crypto_keys_new_import(SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID,
							     payload_buffer, size_decoded);
			LOG_INF("MFG_ED25519 import %s", (0 == err) ? "success" : "failure");

			if (err != 0) {
				sid_hal_free(ram_tlv_data);
				return -EACCES;
			}
			continue;
		}
		if (key_decoded == SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1) {
			int err = sid_crypto_keys_new_import(SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID,
							     payload_buffer, size_decoded);
			LOG_INF("MFG_SECP_256R1 import %s", (0 == err) ? "success" : "failure");

			if (err != 0) {
				sid_hal_free(ram_tlv_data);
				return -EACCES;
			}
			continue;
		}
#endif

		ret = tlv_write(&ram_tlv, key_decoded, payload_buffer, size_decoded);
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

	ret = tlv->storage_impl.write(tlv->storage_impl.ctx, tlv->start_offset, ram_tlv_data,
				      ram_tlv.end_offset);
	if (ret != 0) {
		LOG_ERR("Failed to write parsed tlv data to flash");
		sid_hal_free(ram_tlv_data);
		return -EIO;
	}
	sid_hal_free(ram_tlv_data);
	return 0;
}
