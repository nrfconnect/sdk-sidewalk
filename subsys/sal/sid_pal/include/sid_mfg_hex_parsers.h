/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <tlv/tlv.h>
#include <sid_pal_mfg_store_ifc.h>

#define MFG_FLAGS_TYPE_ID SID_PAL_MFG_STORE_VALUE_MAX - 1
struct mfg_flags {
	uint8_t unused_bits : 6;
	uint8_t keys_in_psa : 1;
	uint8_t initialized : 1;
	uint8_t unused[3];
};

#define MFG_HEADER_MAGIC "SID0"
#define MFG_HEADER_MAGIC_SIZE sizeof(MFG_HEADER_MAGIC) - 1
#define REPORTED_VERSION SID_PAL_MFG_STORE_TLV_VERSION

#define INVALID_VERSION 0xFFFFFFFF
struct mfg_header {
	uint8_t magic_string[MFG_HEADER_MAGIC_SIZE];
	uint8_t raw_version[SID_PAL_MFG_STORE_VERSION_SIZE];
};

/**
 * @brief Parse content of the manufacturing partition v8, and write it as tlv.
 * The TLV will replace raw manufacturing partition
 * 
 * @param tlv [IN/OUT] configuration for tlv
 * @return int 0 on success, -ERRNO on error
 */
int parse_mfg_raw_tlv(tlv_ctx *tlv);

/**
 * @brief Parse content of the manufacturing partition v7, and write it as tlv.
 * The TLV will replace raw manufacturing partition
 * 
 * @param tlv [IN/OUT] configuration for tlv
 * @return int 0 on success, -ERRNO on error
 */
int parse_mfg_const_offsets(tlv_ctx *tlv);
