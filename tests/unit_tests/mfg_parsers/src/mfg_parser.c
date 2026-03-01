/**
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * 
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#if CONFIG_BOARD_NATIVE_SIM
#define FIXED_PARTITION_OFFSET(x) 0xFF000
#else
#include "flash_map_pm.h"
#endif
#include "sid_hal_memory_ifc.h"
#include "zephyr/drivers/flash.h"
#include "zephyr/ztest_assert.h"
#include <string.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>
#include <tlv/tlv.h>
#include <tlv/tlv_storage_impl.h>
#include <sid_mfg_hex_parsers.h>

uint8_t TLV_RAM_STORAGE[0x1000] = { [0x0 ... 0xfff] = 0xff };

#define RAW_MFG_VERSION_8_VALUE 0x00, 0x00, 0x00, 0x08

/* RANDOM VALUES FOR TEST*/
/* 4 */
#define SID_PAL_MFG_STORE_SMSN_VALUE                                                               \
	0x25, 0xd6, 0xb6, 0xbf, 0x35, 0x58, 0xea, 0x6e, 0x0e, 0xec, 0x0d, 0x43, 0xc3, 0x6f, 0x01,  \
		0xf0, 0xf5, 0x74, 0x81, 0xfa, 0x3d, 0x43, 0x54, 0x0d, 0xdb, 0x31, 0x96, 0x7c,      \
		0xe9, 0xb1, 0xb8, 0xa6

/* 5 */
#define SID_PAL_MFG_STORE_APP_PUB_ED25519_VALUE                                                    \
	0xdd, 0x33, 0x8d, 0xd6, 0x47, 0xad, 0x3b, 0x52, 0xd2, 0x1c, 0xd4, 0xd2, 0xc7, 0x94, 0x8c,  \
		0x10, 0x25, 0xf2, 0xe8, 0x57, 0x2c, 0x46, 0xd4, 0x7b, 0xfa, 0x0c, 0x91, 0x60,      \
		0xf1, 0xc7, 0xed, 0xe6

/* 6 */
#define SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_VALUE                                                \
	0x85, 0xcb, 0x3f, 0xf2, 0x61, 0xa6, 0x33, 0x2e, 0xef, 0xa0, 0x60, 0xa5, 0x00, 0x73, 0x4b,  \
		0xf2, 0xcf, 0x11, 0xb8, 0xa7, 0xb7, 0x82, 0x15, 0x11, 0x29, 0x99, 0xf3, 0xa1,      \
		0x58, 0xed, 0x59, 0xaf

/* 7 */
#define SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_VALUE                                                 \
	0xa2, 0x43, 0x2d, 0x98, 0xfe, 0xae, 0x13, 0xa3, 0xfe, 0x5e, 0x5f, 0x31, 0x41, 0xfd, 0x6c,  \
		0x39, 0x98, 0xd6, 0xcf, 0x29, 0x34, 0x0c, 0x4f, 0xaf, 0x09, 0xd6, 0x5e, 0x50,      \
		0xe9, 0x8b, 0x56, 0x0a

/* 8 */
#define SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_VALUE                                       \
	0x69, 0x9d, 0x33, 0x97, 0x60, 0x08, 0x6b, 0xfc, 0xca, 0x9d, 0x69, 0xe5, 0x35, 0xf6, 0x6a,  \
		0xb6, 0x46, 0x07, 0x4b, 0x20, 0xbd, 0xa9, 0xcc, 0x7c, 0xef, 0x93, 0x0c, 0xfd,      \
		0xef, 0x90, 0x71, 0xcf, 0x25, 0xa7, 0x37, 0x1c, 0xa8, 0x01, 0x11, 0x36, 0x5f,      \
		0x00, 0x6c, 0xf6, 0x2f, 0xb6, 0x74, 0xe3, 0xc9, 0x68, 0xf8, 0x1c, 0x73, 0x3b,      \
		0x40, 0xde, 0x61, 0x32, 0x7e, 0x2a, 0xdf, 0x2e, 0x21, 0x63

/* 9 */
#define SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_VALUE                                                 \
	0x7d, 0x2a, 0x0a, 0xca, 0xe2, 0x3a, 0xc8, 0xb2, 0x7f, 0x3e, 0xa6, 0x0a, 0xa0, 0xfb, 0x44,  \
		0xf4, 0x4b, 0xf2, 0x0f, 0xa6, 0x97, 0x08, 0xbc, 0x52, 0x27, 0x5a, 0x23, 0x7c,      \
		0x53, 0x81, 0x99, 0x0b

/* 10 */
#define SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_VALUE                                                  \
	0xc9, 0xd3, 0x4d, 0xab, 0x57, 0x94, 0x6e, 0xe3, 0xd7, 0xb4, 0xe2, 0x29, 0x14, 0xdf, 0xb1,  \
		0x6c, 0xa3, 0xbc, 0x30, 0x81, 0x90, 0xaa, 0x17, 0xc3, 0x71, 0xd0, 0xf2, 0xbb,      \
		0xc2, 0x2b, 0xbd, 0x1b, 0xa5, 0x22, 0xfd, 0xac, 0xc8, 0xa3, 0xc3, 0xd7, 0xbd,      \
		0xec, 0xa1, 0xac, 0x9b, 0x67, 0xeb, 0x3e, 0xe0, 0x6c, 0xb7, 0x07, 0xc8, 0x50,      \
		0xd8, 0x09, 0xa8, 0xb4, 0x87, 0xac, 0xbd, 0x98, 0x72, 0x3b

/* 11 */
#define SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_VALUE                                        \
	0xe2, 0x51, 0x15, 0xfd, 0x41, 0x53, 0xbd, 0xc3, 0x01, 0x01, 0xa3, 0x0b, 0x4f, 0x5c, 0xfd,  \
		0x91, 0xaa, 0xf8, 0x12, 0xd3, 0x6e, 0xb8, 0xc0, 0x6a, 0xf9, 0x38, 0xa6, 0x81,      \
		0x0d, 0x9e, 0x67, 0xdd, 0xb6, 0xae, 0xe8, 0x92, 0x8a, 0xb0, 0x5b, 0xb4, 0xb1,      \
		0xe8, 0x46, 0x13, 0x83, 0xd0, 0x06, 0x09, 0xb2, 0xac, 0x8e, 0x6e, 0x75, 0x31,      \
		0x08, 0x80, 0x85, 0x69, 0xb6, 0x06, 0x43, 0x97, 0x6f, 0x22

/* 12 */
#define SID_PAL_MFG_STORE_DAK_PUB_ED25519_VALUE                                                    \
	0xbb, 0x41, 0xf6, 0x41, 0x1b, 0xa9, 0x84, 0x74, 0xa4, 0x49, 0x1e, 0x08, 0xd1, 0x72, 0x60,  \
		0x22, 0x62, 0x7d, 0xfa, 0xdf, 0x6e, 0x89, 0x5c, 0xf0, 0x84, 0xb1, 0x9c, 0xba,      \
		0xb9, 0x70, 0x40, 0x0d

/* 13 */
#define SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_VALUE                                          \
	0xb6, 0x93, 0xa8, 0xa5, 0x2a, 0xda, 0xe8, 0x1c, 0xaf, 0x16, 0x34, 0x53, 0x63, 0x21, 0xde,  \
		0x5a, 0x94, 0x16, 0x89, 0x8c, 0x35, 0x91, 0x91, 0xde, 0xc2, 0xc5, 0xdc, 0x1d,      \
		0x65, 0xd7, 0x4a, 0x76, 0xd7, 0x83, 0x1f, 0x9b, 0x42, 0x80, 0x55, 0x1a, 0x7e,      \
		0x9f, 0xe8, 0xbe, 0xdc, 0xef, 0x0b, 0x27, 0x22, 0xf6, 0x08, 0x49, 0x67, 0x9c,      \
		0xd0, 0xf6, 0xbe, 0x2e, 0x5c, 0x79, 0x9d, 0x68, 0x4f, 0x41

/* 14 */
#define SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_VALUE 0x17, 0xca, 0x3d, 0xc4

/* 15 */
#define SID_PAL_MFG_STORE_DAK_PUB_P256R1_VALUE                                                     \
	0x8c, 0x6b, 0x58, 0x7e, 0x7f, 0xb5, 0x8e, 0x8d, 0x89, 0xe7, 0x0e, 0x6d, 0x1a, 0x19, 0x46,  \
		0x16, 0xbf, 0x2a, 0x87, 0xc1, 0x55, 0xa3, 0x84, 0x3b, 0xae, 0x33, 0xdf, 0x18,      \
		0x10, 0x41, 0x12, 0xdf, 0x86, 0x84, 0x0c, 0xe0, 0x24, 0xf9, 0x55, 0x3d, 0xf8,      \
		0x78, 0x70, 0x7a, 0x68, 0x56, 0xdd, 0xb8, 0x0c, 0x29, 0xc8, 0x4c, 0xca, 0x8c,      \
		0x38, 0x9d, 0xcc, 0x3a, 0x46, 0xd5, 0xc8, 0x7a, 0xf3, 0x98

/* 16 */
#define SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_VALUE                                           \
	0xd3, 0xcd, 0x58, 0xbb, 0x17, 0x3b, 0x68, 0xf9, 0x3e, 0xbf, 0x0c, 0x38, 0x94, 0x80, 0x2c,  \
		0x80, 0xd2, 0x8c, 0x2a, 0x51, 0x8b, 0x5f, 0x7e, 0x5a, 0xe3, 0x79, 0x6d, 0x10,      \
		0x4f, 0x4a, 0x08, 0x72, 0xa2, 0x52, 0xff, 0xf4, 0x55, 0xcf, 0xc8, 0x53, 0x32,      \
		0x2f, 0xf6, 0xec, 0x12, 0x2e, 0x0f, 0xb2, 0xfe, 0xe2, 0x6f, 0xab, 0xdc, 0xcc,      \
		0x5d, 0x77, 0x5d, 0x24, 0xbc, 0x08, 0xdb, 0x3e, 0x26, 0x68

/* 17 */
#define SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_VALUE 0x98, 0x69, 0x6e, 0xa9
/* 18 */
#define SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_VALUE                                                \
	0x3c, 0x5d, 0xb9, 0x14, 0xb9, 0xd0, 0x5e, 0x7b, 0xfc, 0x98, 0xec, 0x31, 0xaf, 0xa0, 0x43,  \
		0x80, 0x0d, 0xe0, 0xb1, 0x9c, 0x2d, 0xf5, 0xe3, 0xf2, 0x53, 0x78, 0xc2, 0x21,      \
		0x25, 0x7d, 0x9b, 0x33

/* 19 */
#define SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_VALUE                                      \
	0x1f, 0xa1, 0x4a, 0x6d, 0xac, 0xb9, 0x1d, 0x85, 0xdc, 0x87, 0x44, 0x76, 0x75, 0x1e, 0xa2,  \
		0xc0, 0x40, 0xb0, 0x83, 0x70, 0x2b, 0x26, 0xf3, 0xc2, 0xa0, 0xa4, 0xb2, 0x38,      \
		0x01, 0x99, 0x84, 0x06, 0x36, 0xe8, 0x88, 0xcb, 0x55, 0xa7, 0xfb, 0x5e, 0x03,      \
		0x16, 0xb7, 0x9d, 0xbe, 0xab, 0xf9, 0x57, 0x1c, 0xc6, 0x2f, 0x06, 0x0e, 0xa1,      \
		0xf7, 0x8f, 0x61, 0x43, 0x33, 0x83, 0x6a, 0x67, 0x54, 0xf7

/* 20 */
#define SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_VALUE 0x0b, 0xab, 0x8a, 0xb2

/* 21 */
#define SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_VALUE                                                 \
	0x77, 0x49, 0xf9, 0x6c, 0x3e, 0x80, 0xb2, 0x84, 0x94, 0xe4, 0xe7, 0xb4, 0xab, 0x43, 0x25,  \
		0x26, 0x6c, 0xf9, 0xf1, 0x52, 0x3a, 0x0a, 0x75, 0x9a, 0x36, 0xe0, 0xde, 0x81,      \
		0x95, 0x7f, 0x90, 0xe4, 0xb3, 0x3a, 0x44, 0xe5, 0x8b, 0xaf, 0xf2, 0x03, 0xb9,      \
		0xe6, 0xb8, 0xf2, 0x24, 0xa4, 0x86, 0x54, 0xf8, 0x9f, 0x78, 0xb3, 0x5f, 0x44,      \
		0x36, 0xa8, 0x65, 0x09, 0xc5, 0xfc, 0x13, 0x66, 0x59, 0xf5

/* 22 */
#define SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_VALUE                                       \
	0x7a, 0x41, 0xf2, 0x8e, 0xbf, 0x5e, 0x03, 0xa8, 0x64, 0x75, 0x7b, 0x72, 0x6d, 0x84, 0xbd,  \
		0x94, 0x99, 0x77, 0xcc, 0xef, 0x5b, 0x8d, 0x72, 0xef, 0x86, 0xac, 0x78, 0x0d,      \
		0xdf, 0x22, 0xc3, 0x92, 0x6c, 0x75, 0xef, 0x46, 0x9b, 0x1e, 0xb4, 0x73, 0x36,      \
		0xa1, 0x16, 0x70, 0x4f, 0x9d, 0x59, 0xfc, 0x5c, 0x1f, 0x87, 0xcd, 0x49, 0xd4,      \
		0x50, 0x64, 0x70, 0x3d, 0xe7, 0x94, 0x72, 0xc4, 0xea, 0x8c

/* 23 */
#define SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_VALUE 0xff, 0x7e, 0x36, 0x4d

/* 24 */
#define SID_PAL_MFG_STORE_MAN_PUB_ED25519_VALUE                                                    \
	0x61, 0x12, 0xf3, 0x57, 0xa5, 0xc3, 0x82, 0x8f, 0x5d, 0x3a, 0x5f, 0x76, 0xbd, 0x47, 0x57,  \
		0x69, 0x11, 0x3b, 0x5a, 0x58, 0x6b, 0x10, 0x9b, 0xeb, 0x1d, 0x83, 0xc6, 0x2c,      \
		0xea, 0xce, 0xc1, 0xba

/* 25 */
#define SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_VALUE                                          \
	0xab, 0xfd, 0xd4, 0x30, 0x7d, 0x0d, 0x2b, 0x8a, 0x92, 0x5c, 0xd2, 0x89, 0x47, 0xec, 0x16,  \
		0x66, 0x92, 0x2b, 0xa7, 0x0a, 0xe2, 0x14, 0x97, 0x94, 0x46, 0xb7, 0x0c, 0x58,      \
		0x40, 0x91, 0x79, 0xa9, 0xdc, 0xcc, 0x32, 0x0c, 0x48, 0xac, 0x1a, 0x6b, 0x41,      \
		0xb9, 0x1e, 0xe0, 0x39, 0xaa, 0x70, 0x45, 0x23, 0x21, 0xc3, 0x04, 0x2e, 0x6d,      \
		0x72, 0x44, 0x28, 0x3c, 0x37, 0xca, 0xbd, 0xba, 0x10, 0x43

/* 26 */
#define SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_VALUE 0x61, 0x28, 0x1b, 0x29

/* 27 */
#define SID_PAL_MFG_STORE_MAN_PUB_P256R1_VALUE                                                     \
	0xa9, 0x5c, 0x61, 0xba, 0x1a, 0xdc, 0x4b, 0x6b, 0x2b, 0xfd, 0xd2, 0x02, 0xa9, 0xba, 0x50,  \
		0x17, 0xa5, 0x1a, 0x87, 0x39, 0x9a, 0x2b, 0x5d, 0x50, 0xa2, 0xc3, 0xed, 0xcc,      \
		0xdd, 0xff, 0x27, 0x0a, 0x32, 0xc8, 0xd8, 0x30, 0x60, 0xdb, 0x20, 0xfc, 0x46,      \
		0xe8, 0x3b, 0x1c, 0xf4, 0x19, 0xf1, 0xee, 0xc4, 0xef, 0x9f, 0x66, 0xb8, 0x61,      \
		0xd4, 0x78, 0x3a, 0x84, 0xc0, 0x50, 0x95, 0x14, 0xf6, 0x71

/* 28 */
#define SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_VALUE                                           \
	0xd3, 0x72, 0x99, 0x84, 0x54, 0xec, 0xec, 0x8f, 0x66, 0xcb, 0x06, 0x95, 0x13, 0x80, 0x81,  \
		0xd2, 0xb0, 0x02, 0xbb, 0x4f, 0x9e, 0x78, 0xff, 0xbc, 0x41, 0x87, 0x4e, 0xef,      \
		0xbf, 0x61, 0x87, 0x32, 0x0f, 0xd3, 0x25, 0xf0, 0x74, 0x0e, 0x91, 0xf3, 0x60,      \
		0x51, 0xd8, 0x9a, 0xe6, 0x7b, 0x56, 0xa6, 0x6d, 0x6b, 0xaf, 0x1e, 0xd0, 0x2c,      \
		0x7e, 0xc6, 0x91, 0xec, 0x76, 0x12, 0x30, 0xff, 0x24, 0x94

/* 29 */
#define SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_VALUE 0x72, 0x73, 0x8a, 0x7b

/* 30 */
#define SID_PAL_MFG_STORE_SW_PUB_ED25519_VALUE                                                     \
	0x78, 0x17, 0x5e, 0x1b, 0xc6, 0x2a, 0x37, 0x19, 0x0e, 0x8f, 0xb4, 0xa5, 0x76, 0x81, 0xf5,  \
		0x0a, 0x39, 0x2b, 0xd3, 0x40, 0x34, 0x0d, 0x90, 0x06, 0x0d, 0xda, 0x53, 0x26,      \
		0x2d, 0x11, 0x73, 0xb0

/* 31 */
#define SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_VALUE                                           \
	0x71, 0x29, 0xae, 0x6f, 0x94, 0x16, 0xd6, 0xf8, 0x62, 0xd7, 0xec, 0x9b, 0x45, 0x14, 0x1b,  \
		0xae, 0x0e, 0x53, 0x71, 0x39, 0x84, 0xbc, 0x80, 0x95, 0x8f, 0x8c, 0x90, 0x21,      \
		0xdb, 0xa3, 0xd3, 0x1f, 0x5b, 0x72, 0x21, 0x1e, 0x60, 0x46, 0x32, 0x3e, 0x72,      \
		0xdd, 0xfb, 0xb2, 0x8b, 0xff, 0xb9, 0x55, 0xbd, 0x84, 0xc7, 0x79, 0xf3, 0xfa,      \
		0x80, 0x45, 0x5f, 0x22, 0xaa, 0xa8, 0x30, 0x8f, 0x8a, 0x3d

/* 32 */
#define SID_PAL_MFG_STORE_SW_ED25519_SERIAL_VALUE 0x22, 0x51, 0x83, 0x55

/* 33 */
#define SID_PAL_MFG_STORE_SW_PUB_P256R1_VALUE                                                      \
	0xe6, 0x9b, 0x15, 0xcd, 0x78, 0x31, 0x02, 0xc3, 0x4d, 0x4b, 0x1a, 0x95, 0xaa, 0x57, 0x6d,  \
		0x3d, 0x0e, 0xac, 0xeb, 0x1f, 0xec, 0x62, 0xfb, 0xac, 0xc0, 0x8a, 0x7b, 0x3f,      \
		0x07, 0x36, 0x36, 0xe0, 0x56, 0x23, 0x65, 0x14, 0xbd, 0xfc, 0x38, 0x1e, 0xa2,      \
		0x68, 0xda, 0x3c, 0xa1, 0x28, 0xa5, 0x79, 0x9d, 0x39, 0xee, 0x4c, 0x96, 0x12,      \
		0x31, 0xea, 0x7a, 0x04, 0x49, 0xd8, 0x24, 0x93, 0x98, 0xea

/* 34 */
#define SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_VALUE                                            \
	0x0c, 0xdb, 0x18, 0x85, 0x05, 0xb0, 0x64, 0xe3, 0x58, 0x10, 0x2c, 0x67, 0xf1, 0xd2, 0x98,  \
		0x68, 0xbb, 0xf8, 0x75, 0xac, 0xcc, 0x67, 0xc2, 0x3d, 0xac, 0xe5, 0x8c, 0x47,      \
		0x7f, 0x2c, 0xf8, 0x40, 0xb5, 0xb1, 0x15, 0xe2, 0x9f, 0x78, 0x6c, 0x14, 0xce,      \
		0x82, 0x1c, 0xc8, 0xe9, 0xd7, 0x26, 0xba, 0x6d, 0x7e, 0xb0, 0xc9, 0x8d, 0x77,      \
		0x5e, 0xe3, 0x5f, 0xab, 0xbc, 0x14, 0x8f, 0xe3, 0xd5, 0x67

/* 35 */
#define SID_PAL_MFG_STORE_SW_P256R1_SERIAL_VALUE 0xe7, 0xe0, 0x9b, 0x03

/* 36 */
#define SID_PAL_MFG_STORE_AMZN_PUB_ED25519_VALUE                                                   \
	0x35, 0x7c, 0x3a, 0xca, 0x1e, 0xfc, 0x4a, 0x81, 0x25, 0xed, 0xce, 0xbc, 0xfc, 0xf2, 0x33,  \
		0xa6, 0xa7, 0x4a, 0x8c, 0xca, 0x7b, 0x85, 0x49, 0x79, 0xea, 0x46, 0xa0, 0xa6,      \
		0xa8, 0x55, 0x32, 0x17

/* 37 */
#define SID_PAL_MFG_STORE_AMZN_PUB_P256R1_VALUE                                                    \
	0x5f, 0x0a, 0xe1, 0x14, 0x88, 0x92, 0xd9, 0x70, 0xb8, 0xea, 0x80, 0x34, 0xe1, 0xf7, 0x22,  \
		0x08, 0xc1, 0x96, 0x58, 0x4c, 0xdc, 0xe0, 0x67, 0x18, 0x8a, 0x3b, 0x9e, 0xfe,      \
		0x5b, 0xfa, 0xb1, 0x39, 0x9a, 0x0e, 0x14, 0x4c, 0x21, 0xd3, 0x9d, 0x38, 0x64,      \
		0xe1, 0x46, 0x17, 0xe5, 0x5b, 0xb4, 0xef, 0x73, 0x1f, 0x62, 0x4a, 0xb7, 0x3e,      \
		0xf1, 0x53, 0x75, 0x2f, 0x67, 0x3a, 0x6c, 0xa6, 0x75, 0x6b

/* 38 */
#define SID_PAL_MFG_STORE_APID_VALUE 0x76, 0x9d, 0x02, 0x7f

unsigned int mfg_v8_bin_len = 1528;

unsigned char mfg_v8_bin_raw[] = {
	0x53, 0x49, 0x44, 0x30, RAW_MFG_VERSION_8_VALUE,

	0x00, 0x04, 0x00, 0x20, SID_PAL_MFG_STORE_SMSN_VALUE,

	0x00, 0x05, 0x00, 0x20, SID_PAL_MFG_STORE_APP_PUB_ED25519_VALUE,

	0x00, 0x06, 0x00, 0x20, SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_VALUE,

	0x00, 0x07, 0x00, 0x20, SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_VALUE,

	0x00, 0x08, 0x00, 0x40, SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x09, 0x00, 0x20, SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_VALUE,

	0x00, 0x0a, 0x00, 0x40, SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_VALUE,

	0x00, 0x0b, 0x00, 0x40, SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x0c, 0x00, 0x20, SID_PAL_MFG_STORE_DAK_PUB_ED25519_VALUE,

	0x00, 0x0d, 0x00, 0x40, SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x0e, 0x00, 0x04, SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_VALUE,

	0x00, 0x0f, 0x00, 0x40, SID_PAL_MFG_STORE_DAK_PUB_P256R1_VALUE,

	0x00, 0x10, 0x00, 0x40, SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x11, 0x00, 0x04, SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_VALUE,

	0x00, 0x12, 0x00, 0x20, SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_VALUE,

	0x00, 0x13, 0x00, 0x40, SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x14, 0x00, 0x04, SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_VALUE,

	0x00, 0x15, 0x00, 0x40, SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_VALUE,

	0x00, 0x16, 0x00, 0x40, SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x17, 0x00, 0x04, SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_VALUE,

	0x00, 0x18, 0x00, 0x20, SID_PAL_MFG_STORE_MAN_PUB_ED25519_VALUE,

	0x00, 0x19, 0x00, 0x40, SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x1a, 0x00, 0x04, SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_VALUE,

	0x00, 0x1b, 0x00, 0x40, SID_PAL_MFG_STORE_MAN_PUB_P256R1_VALUE,

	0x00, 0x1c, 0x00, 0x40, SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x1d, 0x00, 0x04, SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_VALUE,

	0x00, 0x1e, 0x00, 0x20, SID_PAL_MFG_STORE_SW_PUB_ED25519_VALUE,

	0x00, 0x1f, 0x00, 0x40, SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x20, 0x00, 0x04, SID_PAL_MFG_STORE_SW_ED25519_SERIAL_VALUE,

	0x00, 0x21, 0x00, 0x40, SID_PAL_MFG_STORE_SW_PUB_P256R1_VALUE,

	0x00, 0x22, 0x00, 0x40, SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x23, 0x00, 0x04, SID_PAL_MFG_STORE_SW_P256R1_SERIAL_VALUE,

	0x00, 0x24, 0x00, 0x20, SID_PAL_MFG_STORE_AMZN_PUB_ED25519_VALUE,

	0x00, 0x25, 0x00, 0x40, SID_PAL_MFG_STORE_AMZN_PUB_P256R1_VALUE,

	0x00, 0x26, 0x00, 0x04, SID_PAL_MFG_STORE_APID_VALUE
};

#define RAW_MFG_VERSION_7_VALUE 0x00, 0x00, 0x00, 0x07
enum {
	SID_PAL_MFG_STORE_OFFSET_MAGIC = 4 * 0,
	SID_PAL_MFG_STORE_OFFSET_VERSION = 4 * 1,
	SID_PAL_MFG_STORE_OFFSET_SERIAL_NUM = 4 * 2,
	SID_PAL_MFG_STORE_OFFSET_SMSN = 4 * 8,
	SID_PAL_MFG_STORE_OFFSET_APID = 4 * 16,
	SID_PAL_MFG_STORE_OFFSET_APP_PUB_ED25519 = 4 * 18,
	SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_ED25519 = 4 * 26,
	SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519 = 4 * 34,
	SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519_SIGNATURE = 4 * 42,
	SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_P256R1 = 4 * 58,
	SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1 = 4 * 66,
	SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1_SIGNATURE = 4 * 82,
	SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519 = 4 * 98,
	SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519_SIGNATURE = 4 * 106,
	SID_PAL_MFG_STORE_OFFSET_DAK_ED25519_SERIAL = 4 * 122,
	SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1 = 4 * 123,
	SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1_SIGNATURE = 4 * 139,
	SID_PAL_MFG_STORE_OFFSET_DAK_P256R1_SERIAL = 4 * 155,
	SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519 = 4 * 156,
	SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519_SIGNATURE = 4 * 164,
	SID_PAL_MFG_STORE_OFFSET_PRODUCT_ED25519_SERIAL = 4 * 180,
	SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1 = 4 * 181,
	SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1_SIGNATURE = 4 * 197,
	SID_PAL_MFG_STORE_OFFSET_PRODUCT_P256R1_SERIAL = 4 * 213,
	SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519 = 4 * 214,
	SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519_SIGNATURE = 4 * 222,
	SID_PAL_MFG_STORE_OFFSET_MAN_ED25519_SERIAL = 4 * 238,
	SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1 = 4 * 239,
	SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1_SIGNATURE = 4 * 255,
	SID_PAL_MFG_STORE_OFFSET_MAN_P256R1_SERIAL = 4 * 271,
	SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519 = 4 * 272,
	SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519_SIGNATURE = 4 * 280,
	SID_PAL_MFG_STORE_OFFSET_SW_ED25519_SERIAL = 4 * 296,
	SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1 = 4 * 297,
	SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1_SIGNATURE = 4 * 313,
	SID_PAL_MFG_STORE_OFFSET_SW_P256R1_SERIAL = 4 * 329,
	SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_ED25519 = 4 * 330,
	SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_P256R1 = 4 * 338,
	SID_PAL_MFG_STORE_SID_V0_MAX_OFFSET = 4 * 346,
};

unsigned char mfg_v7_bin_raw[] = {
	[SID_PAL_MFG_STORE_OFFSET_MAGIC] = 0x53,
	0x49,
	0x44,
	0x30,
	[SID_PAL_MFG_STORE_OFFSET_VERSION] = RAW_MFG_VERSION_7_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_SMSN] = SID_PAL_MFG_STORE_SMSN_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_APP_PUB_ED25519] = SID_PAL_MFG_STORE_APP_PUB_ED25519_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_ED25519] =
		SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519] = SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519_SIGNATURE] =
		SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_P256R1] = SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1] = SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1_SIGNATURE] =
		SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519] = SID_PAL_MFG_STORE_DAK_PUB_ED25519_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519_SIGNATURE] =
		SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DAK_ED25519_SERIAL] = SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1] = SID_PAL_MFG_STORE_DAK_PUB_P256R1_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1_SIGNATURE] =
		SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_DAK_P256R1_SERIAL] = SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519] =
		SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519_SIGNATURE] =
		SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_PRODUCT_ED25519_SERIAL] =
		SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1] = SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1_SIGNATURE] =
		SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_PRODUCT_P256R1_SERIAL] =
		SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519] = SID_PAL_MFG_STORE_MAN_PUB_ED25519_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519_SIGNATURE] =
		SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_MAN_ED25519_SERIAL] = SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1] = SID_PAL_MFG_STORE_MAN_PUB_P256R1_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1_SIGNATURE] =
		SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_MAN_P256R1_SERIAL] = SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519] = SID_PAL_MFG_STORE_SW_PUB_ED25519_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519_SIGNATURE] =
		SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_SW_ED25519_SERIAL] = SID_PAL_MFG_STORE_SW_ED25519_SERIAL_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1] = SID_PAL_MFG_STORE_SW_PUB_P256R1_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1_SIGNATURE] =
		SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_SW_P256R1_SERIAL] = SID_PAL_MFG_STORE_SW_P256R1_SERIAL_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_ED25519] = SID_PAL_MFG_STORE_AMZN_PUB_ED25519_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_P256R1] = SID_PAL_MFG_STORE_AMZN_PUB_P256R1_VALUE,
	[SID_PAL_MFG_STORE_OFFSET_APID] = SID_PAL_MFG_STORE_APID_VALUE
};

///////////////////////

#define MFG_HEADER_FLAGS_INITIALIZED 0x80
#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#define PSA_USED 0x40
#else
#define PSA_USED 0x00
#endif

#define MFG_FLAGS MFG_HEADER_FLAGS_INITIALIZED | PSA_USED

unsigned char expected_parsed_mfg[] = {
	0x53, 0x49, 0x44, 0x30, 0x00,
	0x00, 0x00, 0x08,

	0x00, 0x04, 0x00, 0x20, SID_PAL_MFG_STORE_SMSN_VALUE,

	0x00, 0x05, 0x00, 0x20, SID_PAL_MFG_STORE_APP_PUB_ED25519_VALUE,
#ifndef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	0x00, 0x06, 0x00, 0x20, SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_VALUE,
#endif
	0x00, 0x07, 0x00, 0x20, SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_VALUE,

	0x00, 0x08, 0x00, 0x40, SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_VALUE,

#ifndef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	0x00, 0x09, 0x00, 0x20, SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_VALUE,
#endif

	0x00, 0x0a, 0x00, 0x40, SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_VALUE,

	0x00, 0x0b, 0x00, 0x40, SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x0c, 0x00, 0x20, SID_PAL_MFG_STORE_DAK_PUB_ED25519_VALUE,

	0x00, 0x0d, 0x00, 0x40, SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x0e, 0x00, 0x04, SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_VALUE,

	0x00, 0x0f, 0x00, 0x40, SID_PAL_MFG_STORE_DAK_PUB_P256R1_VALUE,

	0x00, 0x10, 0x00, 0x40, SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x11, 0x00, 0x04, SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_VALUE,

	0x00, 0x12, 0x00, 0x20, SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_VALUE,

	0x00, 0x13, 0x00, 0x40, SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x14, 0x00, 0x04, SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_VALUE,

	0x00, 0x15, 0x00, 0x40, SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_VALUE,

	0x00, 0x16, 0x00, 0x40, SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x17, 0x00, 0x04, SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_VALUE,

	0x00, 0x18, 0x00, 0x20, SID_PAL_MFG_STORE_MAN_PUB_ED25519_VALUE,

	0x00, 0x19, 0x00, 0x40, SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x1a, 0x00, 0x04, SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_VALUE,

	0x00, 0x1b, 0x00, 0x40, SID_PAL_MFG_STORE_MAN_PUB_P256R1_VALUE,

	0x00, 0x1c, 0x00, 0x40, SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x1d, 0x00, 0x04, SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_VALUE,

	0x00, 0x1e, 0x00, 0x20, SID_PAL_MFG_STORE_SW_PUB_ED25519_VALUE,

	0x00, 0x1f, 0x00, 0x40, SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_VALUE,

	0x00, 0x20, 0x00, 0x04, SID_PAL_MFG_STORE_SW_ED25519_SERIAL_VALUE,

	0x00, 0x21, 0x00, 0x40, SID_PAL_MFG_STORE_SW_PUB_P256R1_VALUE,

	0x00, 0x22, 0x00, 0x40, SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_VALUE,

	0x00, 0x23, 0x00, 0x04, SID_PAL_MFG_STORE_SW_P256R1_SERIAL_VALUE,

	0x00, 0x24, 0x00, 0x20, SID_PAL_MFG_STORE_AMZN_PUB_ED25519_VALUE,

	0x00, 0x25, 0x00, 0x40, SID_PAL_MFG_STORE_AMZN_PUB_P256R1_VALUE,

	0x00, 0x26, 0x00, 0x04, SID_PAL_MFG_STORE_APID_VALUE,
	0x6F, 0xFD, 0x00, 0x04, MFG_FLAGS,
	0x00, 0x00, 0x00
};

/**
 * @brief Set up for test.
 * 
 * @param f Pointer to fixture.
 */
static void setUp(void *f)
{
	memset(TLV_RAM_STORAGE, 0xff, sizeof(TLV_RAM_STORAGE));
}

ZTEST_SUITE(real_case, NULL, NULL, setUp, NULL, NULL);

ZTEST(real_case, test_valid_mfg_hex_v8)
{
	memcpy(TLV_RAM_STORAGE, mfg_v8_bin_raw, mfg_v8_bin_len);
	tlv_ctx tlv = (tlv_ctx){ .start_offset = 0,
				 .end_offset = sizeof(TLV_RAM_STORAGE),
				 .tlv_storage_start_marker_size = 8,
				 .storage_impl = { .ctx = TLV_RAM_STORAGE,
						   .read = tlv_storage_ram_read,
						   .erase = tlv_storage_ram_erase,
						   .write = tlv_storage_ram_write } };

	int ret = parse_mfg_raw_tlv(&tlv);
	zassert_equal(ret, 0);
	zassert_mem_equal(TLV_RAM_STORAGE, expected_parsed_mfg, sizeof(expected_parsed_mfg));
	uint32_t empty_bytes_after_tlv_size = sizeof(TLV_RAM_STORAGE) - sizeof(expected_parsed_mfg);
	uint8_t *empty_bytes = sid_hal_malloc(empty_bytes_after_tlv_size);
	memset(empty_bytes, 0xFF, empty_bytes_after_tlv_size);
	zassert_mem_equal(&TLV_RAM_STORAGE[sizeof(expected_parsed_mfg)], empty_bytes,
			  empty_bytes_after_tlv_size);
	sid_hal_free(empty_bytes);
}

ZTEST(real_case, test_valid_mfg_hex_v8_flash)
{
	memcpy(TLV_RAM_STORAGE, mfg_v8_bin_raw, mfg_v8_bin_len);

	const struct device *flash_dev = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_flash_controller));

	tlv_ctx tlv = (tlv_ctx){ .start_offset = FIXED_PARTITION_OFFSET(mfg_storage),
				 .end_offset = FIXED_PARTITION_OFFSET(mfg_storage) + 0x1000,
				 .tlv_storage_start_marker_size = 8,
				 .storage_impl = { .ctx = (void *)flash_dev,
						   .read = tlv_storage_flash_read,
						   .erase = tlv_storage_flash_erase,
						   .write = tlv_storage_flash_write } };

	flash_erase(flash_dev, FIXED_PARTITION_OFFSET(mfg_storage), 0x1000);
	flash_write(flash_dev, FIXED_PARTITION_OFFSET(mfg_storage), TLV_RAM_STORAGE, 0x1000);
	memset(TLV_RAM_STORAGE, 0xff, sizeof(TLV_RAM_STORAGE));

	int ret = parse_mfg_raw_tlv(&tlv);
	zassert_equal(ret, 0);

	flash_read(flash_dev, FIXED_PARTITION_OFFSET(mfg_storage), TLV_RAM_STORAGE, 0x1000);

	zassert_mem_equal(TLV_RAM_STORAGE, expected_parsed_mfg, sizeof(expected_parsed_mfg));
	uint32_t empty_bytes_after_tlv_size = sizeof(TLV_RAM_STORAGE) - sizeof(expected_parsed_mfg);
	uint8_t *empty_bytes = sid_hal_malloc(empty_bytes_after_tlv_size);
	memset(empty_bytes, 0xFF, empty_bytes_after_tlv_size);
	zassert_mem_equal(&TLV_RAM_STORAGE[sizeof(expected_parsed_mfg)], empty_bytes,
			  empty_bytes_after_tlv_size);
	sid_hal_free(empty_bytes);
}

static void fill_storage_v7()
{
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_MAGIC,
	       (uint8_t[]){ 0x53, 0x49, 0x44, 0x30 },
	       sizeof((uint8_t[]){ 0x53, 0x49, 0x44, 0x30 }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_VERSION,
	       (uint8_t[]){ RAW_MFG_VERSION_7_VALUE },
	       sizeof((uint8_t[]){ RAW_MFG_VERSION_7_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_SMSN,
	       (uint8_t[]){ SID_PAL_MFG_STORE_SMSN_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_SMSN_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_APP_PUB_ED25519,
	       (uint8_t[]){ SID_PAL_MFG_STORE_APP_PUB_ED25519_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_APP_PUB_ED25519_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_ED25519,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_ED25519_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DEVICE_PRIV_P256R1,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DEVICE_PUB_P256R1_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DAK_PUB_ED25519_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DAK_PUB_ED25519_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DAK_PUB_ED25519_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DAK_ED25519_SERIAL,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DAK_PUB_P256R1_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DAK_PUB_P256R1_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DAK_PUB_P256R1_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_DAK_P256R1_SERIAL,
	       (uint8_t[]){ SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519,
	       (uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_ED25519_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_PRODUCT_ED25519_SERIAL,
	       (uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1,
	       (uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_PRODUCT_PUB_P256R1_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_PRODUCT_P256R1_SERIAL,
	       (uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519,
	       (uint8_t[]){ SID_PAL_MFG_STORE_MAN_PUB_ED25519_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_MAN_PUB_ED25519_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_MAN_PUB_ED25519_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_MAN_ED25519_SERIAL,
	       (uint8_t[]){ SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1,
	       (uint8_t[]){ SID_PAL_MFG_STORE_MAN_PUB_P256R1_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_MAN_PUB_P256R1_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_MAN_PUB_P256R1_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_MAN_P256R1_SERIAL,
	       (uint8_t[]){ SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519,
	       (uint8_t[]){ SID_PAL_MFG_STORE_SW_PUB_ED25519_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_SW_PUB_ED25519_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_SW_PUB_ED25519_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_SW_ED25519_SERIAL,
	       (uint8_t[]){ SID_PAL_MFG_STORE_SW_ED25519_SERIAL_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_SW_ED25519_SERIAL_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1,
	       (uint8_t[]){ SID_PAL_MFG_STORE_SW_PUB_P256R1_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_SW_PUB_P256R1_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_SW_PUB_P256R1_SIGNATURE,
	       (uint8_t[]){ SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_SW_P256R1_SERIAL,
	       (uint8_t[]){ SID_PAL_MFG_STORE_SW_P256R1_SERIAL_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_SW_P256R1_SERIAL_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_ED25519,
	       (uint8_t[]){ SID_PAL_MFG_STORE_AMZN_PUB_ED25519_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_AMZN_PUB_ED25519_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_AMZN_PUB_P256R1,
	       (uint8_t[]){ SID_PAL_MFG_STORE_AMZN_PUB_P256R1_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_AMZN_PUB_P256R1_VALUE }));
	memcpy(TLV_RAM_STORAGE + SID_PAL_MFG_STORE_OFFSET_APID,
	       (uint8_t[]){ SID_PAL_MFG_STORE_APID_VALUE },
	       sizeof((uint8_t[]){ SID_PAL_MFG_STORE_APID_VALUE }));
}

ZTEST(real_case, test_valid_mfg_hex_v7)
{
	fill_storage_v7();

	tlv_ctx tlv = (tlv_ctx){ .start_offset = 0,
				 .end_offset = sizeof(TLV_RAM_STORAGE),
				 .tlv_storage_start_marker_size = 8,
				 .storage_impl = { .ctx = TLV_RAM_STORAGE,
						   .read = tlv_storage_ram_read,
						   .erase = tlv_storage_ram_erase,
						   .write = tlv_storage_ram_write } };

	int ret = parse_mfg_const_offsets(&tlv);
	zassert_equal(ret, 0);
	zassert_mem_equal(TLV_RAM_STORAGE, expected_parsed_mfg, sizeof(expected_parsed_mfg));
	uint32_t empty_bytes_after_tlv_size = sizeof(TLV_RAM_STORAGE) - sizeof(expected_parsed_mfg);
	uint8_t *empty_bytes = sid_hal_malloc(empty_bytes_after_tlv_size);
	memset(empty_bytes, 0xFF, empty_bytes_after_tlv_size);
	zassert_mem_equal(&TLV_RAM_STORAGE[sizeof(expected_parsed_mfg)], empty_bytes,
			  empty_bytes_after_tlv_size);
	sid_hal_free(empty_bytes);
}

ZTEST(real_case, test_valid_mfg_hex_v7_flash)
{
	fill_storage_v7();
	// memcpy(TLV_RAM_STORAGE, mfg_v8_bin_raw, mfg_v8_bin_len);

	const struct device *flash_dev = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_flash_controller));

	tlv_ctx tlv = (tlv_ctx){ .start_offset = FIXED_PARTITION_OFFSET(mfg_storage),
				 .end_offset = FIXED_PARTITION_OFFSET(mfg_storage) + 0x1000,
				 .tlv_storage_start_marker_size = 8,
				 .storage_impl = { .ctx = (void *)flash_dev,
						   .read = tlv_storage_flash_read,
						   .erase = tlv_storage_flash_erase,
						   .write = tlv_storage_flash_write } };

	flash_erase(flash_dev, FIXED_PARTITION_OFFSET(mfg_storage), 0x1000);
	flash_write(flash_dev, FIXED_PARTITION_OFFSET(mfg_storage), TLV_RAM_STORAGE, 0x1000);
	memset(TLV_RAM_STORAGE, 0xff, sizeof(TLV_RAM_STORAGE));

	int ret = parse_mfg_const_offsets(&tlv);
	zassert_equal(ret, 0);

	flash_read(flash_dev, FIXED_PARTITION_OFFSET(mfg_storage), TLV_RAM_STORAGE, 0x1000);

	zassert_mem_equal(TLV_RAM_STORAGE, expected_parsed_mfg, sizeof(expected_parsed_mfg));
	uint32_t empty_bytes_after_tlv_size = sizeof(TLV_RAM_STORAGE) - sizeof(expected_parsed_mfg);
	uint8_t *empty_bytes = sid_hal_malloc(empty_bytes_after_tlv_size);
	memset(empty_bytes, 0xFF, empty_bytes_after_tlv_size);
	zassert_mem_equal(&TLV_RAM_STORAGE[sizeof(expected_parsed_mfg)], empty_bytes,
			  empty_bytes_after_tlv_size);
	sid_hal_free(empty_bytes);
}
