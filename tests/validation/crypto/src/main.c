/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>

#include <sid_pal_crypto_ifc.h>
#include <string.h>

#define SHA256_SZ 32
#define SHA512_SZ 64

static const uint8_t zero_buffer[64] = { 0 };

static const uint8_t hash_string[] = "halo_crypto_test_run_hash";

static const uint8_t sha256_result[SHA256_SZ] = { 0x16, 0xf6, 0x09, 0xe1, 0x5f, 0x33, 0x75, 0xe2,
						  0xcb, 0xa0, 0x77, 0x32, 0x65, 0xdf, 0xcf, 0x93,
						  0xab, 0x45, 0xf5, 0xb9, 0x90, 0x36, 0x05, 0x06,
						  0x8e, 0xd9, 0x8c, 0xc0, 0xdf, 0x97, 0xf6, 0x22 };

static const uint8_t sha512_result[SHA512_SZ] = {
	0x08, 0x89, 0x42, 0x97, 0x01, 0x29, 0x4b, 0x44, 0xa8, 0x02, 0xba, 0xf5, 0xff,
	0x61, 0x58, 0xfb, 0x35, 0xce, 0x42, 0xfd, 0x31, 0x14, 0x07, 0x97, 0x5b, 0x56,
	0x85, 0xe0, 0xe1, 0x25, 0x75, 0x3c, 0xb2, 0x8c, 0xdd, 0x87, 0x66, 0x7e, 0xcb,
	0x3a, 0x10, 0x0b, 0xe9, 0xd5, 0x64, 0xfb, 0xa8, 0xed, 0x2f, 0xd3, 0x21, 0x4f,
	0xf9, 0xe0, 0xdc, 0xce, 0xea, 0x27, 0xd5, 0x86, 0xf1, 0x4c, 0x3b, 0x9e
};

static const uint8_t hmac_hash_string[] = "halo_crypto_test_run_hmac";

static const uint8_t hmac_key[16] = { 0x16, 0xf6, 0x09, 0xe1, 0x5f, 0x33, 0x75, 0xe2,
				      0xcb, 0xa0, 0x77, 0x32, 0x65, 0xdf, 0xcf, 0x93 };

static const uint8_t hmac_invalid_key[16] = { 0x16, 0xf6, 0x09, 0xe1, 0x5f, 0x33, 0x75, 0xe2,
					      0xcb, 0xa0, 0x77, 0x32, 0x65, 0xdf, 0xcf, 0xFF };
_Static_assert(sizeof(hmac_key) == sizeof(hmac_invalid_key), "Hmac key sizes are not equal");

static const uint8_t hmac_sha256_result[SHA256_SZ] = { 0xfd, 0x71, 0x58, 0xaa, 0x22, 0xe6, 0x34,
						       0x1d, 0x19, 0x9a, 0x98, 0x04, 0x6f, 0x6e,
						       0x3e, 0x6f, 0x3d, 0xc0, 0xdf, 0xdb, 0x1c,
						       0x51, 0xe9, 0x8a, 0xcd, 0xf5, 0x97, 0xd6,
						       0x73, 0xf1, 0xea, 0x33 };

static const uint8_t hmac_sha512_result[SHA512_SZ] = {
	0x64, 0x84, 0xa8, 0x5b, 0x48, 0x62, 0x41, 0xa6, 0xbf, 0x2e, 0xff, 0xe6, 0xed,
	0x24, 0xf5, 0xd7, 0x8b, 0x84, 0x10, 0xe8, 0x14, 0xaf, 0x3f, 0xed, 0x4a, 0x6d,
	0x98, 0xcc, 0x66, 0xc7, 0x4b, 0x48, 0xe5, 0xaa, 0x7c, 0x14, 0x5f, 0x57, 0xea,
	0xda, 0x6b, 0xb2, 0xc7, 0x42, 0x98, 0xfe, 0x86, 0x7a, 0xc9, 0xc8, 0xff, 0x2b,
	0x3a, 0x31, 0xde, 0xec, 0xc9, 0x6a, 0x81, 0xd4, 0xce, 0x0d, 0x5c, 0x5d
};

// Test vector
// https://tools.ietf.org/html/rfc4493
static const uint8_t aes_plaintext[16] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
					   0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };

static const uint8_t aes_invalid_plaintext[16] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
						   0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2b };

static const uint8_t aes_key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
				     0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
uint8_t const aes_mac[16] = { 0x07, 0x0a, 0x16, 0xb4, 0x6b, 0x4d, 0x41, 0x44,
			      0xf7, 0x9b, 0xdd, 0x9d, 0xd0, 0x4a, 0x28, 0x7c };
static const uint8_t aes_invalid_key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
					     0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0xFF };
_Static_assert(sizeof(aes_key) == sizeof(aes_invalid_key), "AES key sizes are not equal");

// Test vector from
// http://www.ieee802.org/1/files/public/docs2011/bn-randall-test-vectors-0511-v1.pdf
static const uint8_t aes_gcm_key[] = { 0x07, 0x1B, 0x11, 0x3B, 0x0C, 0xA7, 0x43, 0xFE,
				       0xCC, 0xCF, 0x3D, 0x05, 0x1F, 0x73, 0x73, 0x82 };
static const uint8_t aes_gcm_plaintext[] = { 0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
					     0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E,
					     0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
					     0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
					     0x31, 0x32, 0x33, 0x34, 0x00, 0x04 };
static const uint8_t aes_gcm_aad[] = { 0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D, 0xF0, 0x76, 0x1E, 0x8D,
				       0xCD, 0x3D, 0x88, 0xE5, 0x4C, 0x2A, 0x76, 0xD4, 0x57, 0xED };
static const uint8_t aes_gcm_iv[] = { 0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D,
				      0x00, 0x01, 0x76, 0xD4, 0x57, 0xED };
static const uint8_t aes_gcm_ciphertext[] = { 0x13, 0xB4, 0xC7, 0x2B, 0x38, 0x9D, 0xC5, 0x01, 0x8E,
					      0x72, 0xA1, 0x71, 0xDD, 0x85, 0xA5, 0xD3, 0x75, 0x22,
					      0x74, 0xD3, 0xA0, 0x19, 0xFB, 0xCA, 0xED, 0x09, 0xA4,
					      0x25, 0xCD, 0x9B, 0x2E, 0x1C, 0x9B, 0x72, 0xEE, 0xE7,
					      0xC9, 0xDE, 0x7D, 0x52, 0xB3, 0xF3 };
static const uint8_t aes_gcm_mac[] = { 0xD6, 0xA5, 0x28, 0x4F, 0x4A, 0x6D, 0x3F, 0xE2,
				       0x2A, 0x5D, 0x6C, 0x2B, 0x96, 0x04, 0x94, 0xC3 };

static const uint8_t aes_gcm_invalid_key[] = { 0x07, 0x1B, 0x11, 0x3B, 0x0C, 0xA7, 0x43, 0xFE,
					       0xCC, 0xCF, 0x3D, 0x05, 0x1F, 0x73, 0x73, 0xFF };
static const uint8_t aes_gcm_invalid_aad[] = { 0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D, 0xF0,
					       0x76, 0x1E, 0x8D, 0xCD, 0x3D, 0x88, 0xE5,
					       0x4C, 0x2A, 0x76, 0xD4, 0x57, 0xFF };
static const uint8_t aes_gcm_invalid_iv[] = { 0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D,
					      0x00, 0x01, 0x76, 0xD4, 0x57, 0xFF };
static const uint8_t aes_gcm_invalid_ciphertext[] = {
	0x13, 0xB4, 0xC7, 0x2B, 0x38, 0x9D, 0xC5, 0x01, 0x8E, 0x72, 0xA1, 0x71, 0xDD, 0x85,
	0xA5, 0xD3, 0x75, 0x22, 0x74, 0xD3, 0xA0, 0x19, 0xFB, 0xCA, 0xED, 0x09, 0xA4, 0x25,
	0xCD, 0x9B, 0x2E, 0x1C, 0x9B, 0x72, 0xEE, 0xE7, 0xC9, 0xDE, 0x7D, 0x52, 0xB3, 0xFF
};
static const uint8_t aes_gcm_invalid_mac[] = { 0xD6, 0xA5, 0x28, 0x4F, 0x4A, 0x6D, 0x3F, 0xE2,
					       0x2A, 0x5D, 0x6C, 0x2B, 0x96, 0x04, 0x94, 0xFF };
_Static_assert(sizeof(aes_gcm_key) == sizeof(aes_gcm_invalid_key),
	       "AES GCM key sizes are not equal");
_Static_assert(sizeof(aes_gcm_aad) == sizeof(aes_gcm_invalid_aad),
	       "AES GCM AAD sizes are not equal");
_Static_assert(sizeof(aes_gcm_iv) == sizeof(aes_gcm_invalid_iv), "AES GCM IV sizes are not equal");
_Static_assert(sizeof(aes_gcm_ciphertext) == sizeof(aes_gcm_invalid_ciphertext),
	       "AES GCM Ciphertext sizes are not equal");
_Static_assert(sizeof(aes_gcm_mac) == sizeof(aes_gcm_invalid_mac),
	       "AES GCM key sizes are not equal");

// Test vector
// https://tools.ietf.org/html/rfc3686
static const uint8_t aes_ctr_plaintext[32] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
					       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
					       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
					       0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
static const uint8_t aes_ctr_key[16] = { 0x7E, 0x24, 0x06, 0x78, 0x17, 0xFA, 0xE0, 0xD7,
					 0x43, 0xD6, 0xCE, 0x1F, 0x32, 0x53, 0x91, 0x63 };
static const uint8_t aes_ctr_iv[16] = { 0x00, 0x6C, 0xB6, 0xDB, 0xC0, 0x54, 0x3B, 0x59,
					0xDA, 0x48, 0xD9, 0x0B, 0x00, 0x00, 0x00, 0x01 };
static const uint8_t aes_ctr_ciphertext[32] = { 0x51, 0x04, 0xA1, 0x06, 0x16, 0x8A, 0x72, 0xD9,
						0x79, 0x0D, 0x41, 0xEE, 0x8E, 0xDA, 0xD3, 0x88,
						0xEB, 0x2E, 0x1E, 0xFC, 0x46, 0xDA, 0x57, 0xC8,
						0xFC, 0xE6, 0x30, 0xDF, 0x91, 0x41, 0xBE, 0x28 };
static const uint8_t aes_ctr_invalid_key[16] = { 0x7E, 0x24, 0x06, 0x78, 0x17, 0xFA, 0xE0, 0xD7,
						 0x43, 0xD6, 0xCE, 0x1F, 0x32, 0x53, 0x91, 0xFF };
static const uint8_t aes_ctr_invalid_iv[16] = { 0x00, 0x6C, 0xB6, 0xDB, 0xC0, 0x54, 0x3B, 0x59,
						0xDA, 0x48, 0xD9, 0x0B, 0x00, 0x00, 0x00, 0xFF };
static const uint8_t aes_ctr_invalid_ciphertext[32] = { 0x51, 0x04, 0xA1, 0x06, 0x16, 0x8A, 0x72,
							0xD9, 0x79, 0x0D, 0x41, 0xEE, 0x8E, 0xDA,
							0xD3, 0x88, 0xEB, 0x2E, 0x1E, 0xFC, 0x46,
							0xDA, 0x57, 0xC8, 0xFC, 0xE6, 0x30, 0xDF,
							0x91, 0x41, 0xBE, 0xFF };
_Static_assert(sizeof(aes_ctr_key) == sizeof(aes_ctr_invalid_key),
	       "AES CTR key sizes are not equal");
_Static_assert(sizeof(aes_ctr_iv) == sizeof(aes_ctr_invalid_iv), "AES CTR IV sizes are not equal");
_Static_assert(sizeof(aes_ctr_ciphertext) == sizeof(aes_ctr_invalid_ciphertext),
	       "AES CTR Ciphertext sizes are not equal");

// Generated from openssl
static const uint8_t curve25519_prk[32] = {
	0xB0, 0x76, 0x51, 0xEA, 0x20, 0xF0, 0x28, 0xA8, 0x16, 0xEE, 0x01,
	0xB0, 0xD1, 0x06, 0x2A, 0x7C, 0x81, 0x58, 0xE8, 0x84, 0xE9, 0xBC,
	0xC6, 0x1C, 0x5D, 0xAB, 0xDB, 0x4E, 0x38, 0x2F, 0x96, 0x69,
};
static const uint8_t curve25519_puk[32] = {
	0x87, 0xD8, 0x6B, 0xDA, 0xAC, 0x38, 0x3C, 0x85, 0xA6, 0xBC, 0xF8,
	0xFC, 0xC6, 0x26, 0xD6, 0x14, 0x36, 0xE4, 0x8F, 0xDB, 0xFA, 0x5A,
	0x45, 0xFE, 0x0C, 0x9E, 0xA8, 0x4B, 0x35, 0x3E, 0xF1, 0x37,
};
static const uint8_t curve25519_remote_prk[32] = {
	0x98, 0x2E, 0xB6, 0x7D, 0x0A, 0x01, 0x57, 0x90, 0xE1, 0x45, 0xF3,
	0x67, 0xF6, 0xDA, 0xA6, 0x44, 0x2C, 0x87, 0xC0, 0xED, 0x3C, 0x36,
	0x71, 0xA6, 0x89, 0xC7, 0x49, 0xAC, 0x0D, 0xFE, 0x43, 0x6E,
};
static const uint8_t curve25519_remote_puk[32] = {
	0x0C, 0x04, 0x10, 0x5B, 0xE8, 0x7C, 0xAB, 0x37, 0x21, 0x15, 0x7A,
	0x8D, 0x49, 0x85, 0x8C, 0x7A, 0x9F, 0xC1, 0x46, 0xDA, 0xCC, 0x96,
	0xEF, 0x6E, 0xD4, 0xDA, 0x71, 0xBF, 0xED, 0x32, 0x0D, 0x76,
};
static const uint8_t curve25519_shared_secret[32] = {
	0xF2, 0xE6, 0x0E, 0x1C, 0xB7, 0x64, 0xBC, 0x48, 0xF2, 0x9D, 0xBB,
	0x12, 0xFB, 0x12, 0x17, 0x31, 0x32, 0x1D, 0x79, 0xAF, 0x0A, 0x9F,
	0xAB, 0xAD, 0x34, 0x05, 0xA2, 0x07, 0x39, 0x9C, 0x5F, 0x15,
};

// P256R1
static const uint8_t p256r1_prk[32] = {
	0xB7, 0xF6, 0x14, 0xC6, 0x31, 0x01, 0x65, 0x13, 0x65, 0x9C, 0xF9,
	0xD9, 0xC2, 0x2B, 0x69, 0x39, 0x71, 0x4E, 0xC9, 0x6B, 0x08, 0x4F,
	0x43, 0xDA, 0x35, 0x88, 0xFD, 0x46, 0xA9, 0xF6, 0xDC, 0x7F,
};
static const uint8_t p256r1_puk[64] = {
	0xF8, 0x0D, 0xDB, 0x3D, 0xD0, 0x94, 0x42, 0xFE, 0x53, 0x50, 0x3F, 0xE6, 0x9F,
	0xE4, 0x0F, 0xFE, 0x56, 0x74, 0x5C, 0x51, 0x20, 0x23, 0x55, 0x91, 0x5F, 0x1B,
	0xA3, 0x74, 0x2C, 0x39, 0xAE, 0x6F, 0x25, 0x16, 0xED, 0xDE, 0x12, 0xDA, 0xD2,
	0x3E, 0x2E, 0x6E, 0xD1, 0x1E, 0x08, 0x9A, 0x6F, 0x1D, 0x6A, 0x21, 0xCC, 0xCB,
	0x90, 0x4D, 0x06, 0x36, 0x25, 0xC8, 0xDA, 0xE5, 0x06, 0x34, 0xE9, 0xAF,
};
static const uint8_t p256r1_remote_prk[32] = {
	0xFE, 0x31, 0xF4, 0xA8, 0x01, 0xE2, 0xA9, 0x4B, 0x71, 0xB7, 0xCA,
	0xEC, 0x91, 0xD2, 0xC6, 0x3C, 0x4D, 0x5B, 0x8A, 0x03, 0xB8, 0x17,
	0xDB, 0x2C, 0x8D, 0x6A, 0x3A, 0x90, 0xD0, 0x7A, 0x6D, 0xAF,
};
static const uint8_t p256r1_remote_puk[64] = {
	0xA9, 0xCD, 0xD3, 0x47, 0xA9, 0xFB, 0x3B, 0xC9, 0xB8, 0x53, 0x8E, 0x04, 0x6F,
	0xD5, 0xBF, 0x3B, 0x3B, 0x72, 0xC2, 0x12, 0xE6, 0xFA, 0xE2, 0x4C, 0x54, 0x54,
	0x44, 0x92, 0x71, 0x2D, 0x52, 0x58, 0xA8, 0x4A, 0x7D, 0xB0, 0xB4, 0x0A, 0x2F,
	0x86, 0x8C, 0x63, 0xBA, 0xFF, 0xA1, 0x15, 0xC9, 0x95, 0x36, 0xDF, 0x69, 0xF9,
	0x87, 0x29, 0x1E, 0x8E, 0x09, 0x29, 0x4A, 0xA0, 0x3A, 0xFC, 0x4C, 0x27,
};
static const uint8_t p256r1_shared_secret[32] = {
	0x62, 0x4C, 0xFB, 0xA0, 0xDB, 0x79, 0x81, 0x1F, 0x0F, 0x3F, 0x46,
	0x82, 0xE6, 0x30, 0x57, 0xA7, 0x0F, 0xA6, 0xD5, 0x25, 0x73, 0x87,
	0x75, 0x3F, 0x2D, 0xC8, 0xB3, 0x7A, 0x21, 0x16, 0x32, 0x9E,
};

// ed25519
static const uint8_t ed25519_prk[32] = { 0xc6, 0x2a, 0x22, 0x9f, 0x15, 0x2a, 0x7f, 0x78,
					 0x2e, 0xa0, 0x62, 0x8c, 0x53, 0x8e, 0xd2, 0x5b,
					 0xe2, 0x7b, 0xa3, 0xc8, 0xcd, 0x07, 0x25, 0xe1,
					 0x5c, 0x4a, 0x4a, 0xb8, 0x6e, 0x57, 0x8f, 0x54 };
static const uint8_t ed25519_puk[32] = { 0x7e, 0x49, 0x3f, 0x82, 0xbe, 0x1d, 0x69, 0xc6,
					 0x4a, 0xf1, 0x80, 0x6e, 0x8a, 0x91, 0x08, 0xf3,
					 0x8b, 0xee, 0x3b, 0xc8, 0x82, 0x3f, 0x54, 0xca,
					 0x29, 0x59, 0x4c, 0xc5, 0x89, 0x20, 0x66, 0x40 };

static const uint8_t sign_hash_msg[40] = {
	0x16, 0xf6, 0x09, 0xe1, 0x5f, 0x33, 0x75, 0xe2, 0xcb, 0xa0, 0x77, 0x32, 0x65, 0xdf,
	0xcf, 0x93, 0xab, 0x45, 0xf5, 0xb9, 0x90, 0x36, 0x05, 0x06, 0x8e, 0xd9, 0x8c, 0xc0,
	0xdf, 0x97, 0xf6, 0x22, 0xB8, 0x53, 0x8E, 0x04, 0x6F, 0xD5, 0xBF, 0x3B,
};

static const uint8_t sign_hash_msg_noalign[45] = {
	0x16, 0xf6, 0x09, 0xe1, 0x5f, 0x33, 0x75, 0xe2, 0xcb, 0xa0, 0x77, 0x32, 0x65, 0xdf, 0xcf,
	0x93, 0xab, 0x45, 0xf5, 0xb9, 0x90, 0x36, 0x05, 0x06, 0x8e, 0xd9, 0x8c, 0xc0, 0xdf, 0x97,
	0xf6, 0x22, 0xB8, 0x53, 0x8E, 0x04, 0x6F, 0xD5, 0xBF, 0x3B, 0x67, 0xa3, 0x71, 0xfd, 0xc0,
};

static size_t get_digest_size(sid_pal_hash_algo_t algo)
{
	switch (algo) {
	case SID_PAL_HASH_SHA256:
		return SHA256_SZ;
	case SID_PAL_HASH_SHA512:
		return SHA512_SZ;
	default:
		TEST_ASSERT(0);
		return 0;
	}
}

static void test_hash_invalid_param(sid_pal_hash_algo_t algo)
{
	uint8_t out_buf[64];
	sid_pal_hash_params_t hash_params = {
		.data = hash_string,
		.data_size = strlen((char *)hash_string),
		.digest = out_buf,
		.digest_size = get_digest_size(algo),
		.algo = algo,
	};

	hash_params.algo = 0;
	sid_error_t ret = sid_pal_crypto_hash(&hash_params);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hash_params.algo = algo;
	hash_params.data = NULL;
	ret = sid_pal_crypto_hash(&hash_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hash_params.data = hash_string;
	hash_params.data_size = 0;
	ret = sid_pal_crypto_hash(&hash_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hash_params.data_size = sizeof(hash_string);
	hash_params.digest = NULL;
	ret = sid_pal_crypto_hash(&hash_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hash_params.digest = out_buf;
	hash_params.digest_size = 0;
	ret = sid_pal_crypto_hash(&hash_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

static void test_hmac_invalid_param(sid_pal_hash_algo_t algo)
{
	uint8_t out_buf[64];
	sid_pal_hmac_params_t hmac_params = {
		.algo = algo,
		.key = hmac_key,
		.key_size = sizeof(hmac_key),
		.data = hmac_hash_string,
		.data_size = strlen((char *)hmac_hash_string),
		.digest = out_buf,
		.digest_size = get_digest_size(algo),
	};

	hmac_params.algo = 0;
	sid_error_t ret = sid_pal_crypto_hmac(&hmac_params);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hmac_params.algo = algo;
	hmac_params.key = NULL;
	ret = sid_pal_crypto_hmac(&hmac_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hmac_params.key = hmac_key;
	hmac_params.key_size = 0;
	ret = sid_pal_crypto_hmac(&hmac_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hmac_params.key_size = sizeof(hmac_key);
	hmac_params.data = NULL;
	ret = sid_pal_crypto_hmac(&hmac_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hmac_params.data = hmac_hash_string;
	hmac_params.data_size = 0;
	ret = sid_pal_crypto_hmac(&hmac_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hmac_params.data_size = sizeof(hmac_hash_string);
	hmac_params.digest = NULL;
	ret = sid_pal_crypto_hmac(&hmac_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	hmac_params.digest = out_buf;
	hmac_params.digest_size = 0;
	ret = sid_pal_crypto_hmac(&hmac_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

static void test_gcm_invalid_params(sid_pal_aes_mode_t mode)
{
	uint8_t out_buf[64];
	uint8_t out_mac_buf[16];

	sid_pal_aead_params_t aead_params = {
		.algo = SID_PAL_AEAD_GCM_128,
		.mode = mode,
		.iv = aes_gcm_iv,
		.iv_size = sizeof(aes_gcm_iv),
		.aad = aes_gcm_aad,
		.aad_size = sizeof(aes_gcm_aad),
		.key = aes_gcm_key,
		.key_size = 16 * 8, // 128 bits
		.in = aes_gcm_plaintext,
		.in_size = sizeof(aes_gcm_plaintext),
		.out = out_buf,
		.out_size = sizeof(aes_gcm_ciphertext),
		.mac = out_mac_buf,
		.mac_size = sizeof(aes_gcm_mac),
	};

	aead_params.algo = 0;
	sid_error_t ret = sid_pal_crypto_aead_crypt(&aead_params);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.algo = SID_PAL_AEAD_GCM_128;
	aead_params.iv = NULL;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.iv = aes_gcm_iv;
	aead_params.iv_size = 0;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.iv_size = sizeof(aes_gcm_iv);
	aead_params.aad = NULL;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.aad = aes_gcm_aad;
	aead_params.aad_size = 0;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.aad_size = sizeof(aes_gcm_aad);
	aead_params.key = NULL;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.key = aes_gcm_key;
	aead_params.key_size = 0;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.key_size = 16 * 8; // 128 bits
	aead_params.mode = 0;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.mode = mode;
	aead_params.in = NULL;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.in = aes_gcm_plaintext;
	aead_params.in_size = 0;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.in_size = sizeof(aes_gcm_plaintext);
	aead_params.out = NULL;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.out_size = sizeof(aes_gcm_ciphertext);
	aead_params.mac = NULL;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.mac = out_mac_buf;
	aead_params.mac_size = 0;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

static void test_ctr_invalid_params(sid_pal_aes_mode_t mode)
{
	uint8_t out_buf[64];
	sid_pal_aes_params_t aes_params = {
		.algo = SID_PAL_AES_CTR_128,
		.mode = mode,
		.iv = aes_ctr_iv,
		.iv_size = sizeof(aes_ctr_iv),
		.key = aes_ctr_key,
		.key_size = 16 * 8, // 128 bits
		.in = aes_ctr_plaintext,
		.in_size = sizeof(aes_ctr_plaintext),
		.out = out_buf,
		.out_size = sizeof(aes_ctr_ciphertext),
	};

	aes_params.algo = 0;
	sid_error_t ret = sid_pal_crypto_aes_crypt(&aes_params);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.algo = SID_PAL_AES_CTR_128;
	aes_params.mode = 0;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.mode = mode;
	aes_params.iv = NULL;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.iv = aes_ctr_iv;
	aes_params.iv_size = 0;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.iv_size = sizeof(aes_ctr_iv);
	aes_params.key = NULL;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.key = aes_ctr_key;
	aes_params.key_size = 0;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.key_size = 16 * 8; // 128 bits
	aes_params.in = NULL;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.in = aes_ctr_plaintext;
	aes_params.in_size = 0;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.in_size = sizeof(aes_ctr_plaintext);
	aes_params.out = NULL;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.out = out_buf;
	aes_params.out_size = 0;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

static sid_error_t ecc_key_gen_positive(sid_pal_ecc_algo_t algo, uint8_t *const prk, size_t prk_sz,
					uint8_t *const puk, size_t puk_sz)
{
	sid_pal_ecc_key_gen_params_t key_params = {
		.algo = algo,
		.prk = prk,
		.prk_size = prk_sz,
		.puk = puk,
		.puk_size = puk_sz,
	};

	return sid_pal_crypto_ecc_key_gen(&key_params);
}

static void test_ecc_key_gen_invalid_param(sid_pal_ecc_algo_t algo, size_t prk_sz, size_t puk_sz)
{
	uint8_t prk[64], puk[64];

	sid_pal_ecc_key_gen_params_t key_params = {
		.algo = algo,
		.prk = prk,
		.prk_size = prk_sz,
		.puk = puk,
		.puk_size = puk_sz,
	};

	key_params.algo = 0;
	sid_error_t ret = sid_pal_crypto_ecc_key_gen(&key_params);

	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	key_params.algo = algo;
	key_params.prk = NULL;
	ret = sid_pal_crypto_ecc_key_gen(&key_params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	key_params.prk = prk;
	key_params.prk_size = 0;
	ret = sid_pal_crypto_ecc_key_gen(&key_params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	key_params.prk_size = prk_sz;
	key_params.puk = NULL;
	ret = sid_pal_crypto_ecc_key_gen(&key_params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	key_params.puk = puk;
	key_params.puk_size = 0;
	ret = sid_pal_crypto_ecc_key_gen(&key_params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);
}

static const uint8_t *get_ecc_prk(sid_pal_ecc_algo_t algo)
{
	switch (algo) {
	case SID_PAL_ECDH_CURVE25519:
		return curve25519_prk;
	case SID_PAL_ECDH_SECP256R1:
	case SID_PAL_ECDSA_SECP256R1:
		return p256r1_prk;
	case SID_PAL_EDDSA_ED25519:
		return ed25519_prk;
	default:
		return NULL;
	}
}

static const uint8_t *get_ecc_puk(sid_pal_ecc_algo_t algo)
{
	switch (algo) {
	case SID_PAL_ECDH_CURVE25519:
		return curve25519_puk;
	case SID_PAL_ECDH_SECP256R1:
	case SID_PAL_ECDSA_SECP256R1:
		return p256r1_puk;
	case SID_PAL_EDDSA_ED25519:
		return ed25519_puk;
	default:
		return NULL;
	}
}

static const uint8_t *get_ecc_remote_prk(sid_pal_ecc_algo_t algo)
{
	switch (algo) {
	case SID_PAL_ECDH_CURVE25519:
		return curve25519_remote_prk;
	case SID_PAL_ECDH_SECP256R1:
	case SID_PAL_ECDSA_SECP256R1:
		return p256r1_remote_prk;
	default:
		return NULL;
	}
}

static const uint8_t *get_ecc_remote_puk(sid_pal_ecc_algo_t algo)
{
	switch (algo) {
	case SID_PAL_ECDH_CURVE25519:
		return curve25519_remote_puk;
	case SID_PAL_ECDH_SECP256R1:
	case SID_PAL_ECDSA_SECP256R1:
		return p256r1_remote_puk;
	default:
		return NULL;
	}
}

static const uint8_t *get_ecc_shared_secret(sid_pal_ecc_algo_t algo)
{
	switch (algo) {
	case SID_PAL_ECDH_CURVE25519:
		return curve25519_shared_secret;
	case SID_PAL_ECDH_SECP256R1:
		return p256r1_shared_secret;
	default:
		return NULL;
	}
}

static void ecc_ecdh_positive(sid_pal_ecc_algo_t algo, size_t prk_sz, size_t puk_sz)
{
	sid_pal_ecdh_params_t params;
	sid_pal_ecc_key_gen_params_t key_params;
	uint8_t prk[64], puk[64];
	uint8_t prk_remote[64], puk_remote[64];
	uint8_t shared_secret[64], shared_secret_remote[64];

	key_params.algo = algo;
	key_params.prk_size = prk_sz;
	key_params.puk_size = puk_sz;
	key_params.prk = prk;
	key_params.puk = puk;
	sid_error_t ret = sid_pal_crypto_ecc_key_gen(&key_params);

	/* Not all platforms support ECDH_SECP256R1, stop testing this now if so */
	if (SID_ERROR_NOSUPPORT == ret && SID_PAL_ECDH_SECP256R1 == algo) {
		return;
	}
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	key_params.prk_size = prk_sz;
	key_params.puk_size = puk_sz;
	key_params.prk = prk_remote;
	key_params.puk = puk_remote;
	ret = sid_pal_crypto_ecc_key_gen(&key_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.prk = prk;
	params.prk_size = prk_sz;
	params.puk = puk_remote;
	params.puk_size = puk_sz;
	params.shared_secret = shared_secret;
	params.shared_secret_sz = 32;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.prk = prk_remote;
	params.prk_size = prk_sz;
	params.puk = puk;
	params.puk_size = puk_sz;
	params.shared_secret = shared_secret_remote;
	params.shared_secret_sz = 32;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(shared_secret, shared_secret_remote, params.shared_secret_sz));

	// Combo with predefined keys
	params.algo = key_params.algo;
	params.prk = prk;
	params.prk_size = prk_sz;
	params.puk = get_ecc_remote_puk(algo);
	params.puk_size = puk_sz;
	params.shared_secret = shared_secret;
	params.shared_secret_sz = 32;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.prk = get_ecc_remote_prk(algo);
	params.prk_size = prk_sz;
	params.puk = puk;
	params.puk_size = puk_sz;
	params.shared_secret = shared_secret_remote;
	params.shared_secret_sz = 32;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(shared_secret, shared_secret_remote, params.shared_secret_sz));

	// All predefined information
	params.algo = key_params.algo;
	params.prk = get_ecc_prk(algo);
	params.prk_size = prk_sz;
	params.puk = get_ecc_remote_puk(algo);
	params.puk_size = puk_sz;
	params.shared_secret = shared_secret;
	params.shared_secret_sz = 32;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.prk = get_ecc_remote_prk(algo);
	params.prk_size = prk_sz;
	params.puk = get_ecc_puk(algo);
	params.puk_size = puk_sz;
	params.shared_secret = shared_secret_remote;
	params.shared_secret_sz = 32;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(shared_secret, shared_secret_remote, params.shared_secret_sz));
	TEST_ASSERT_EQUAL(0, memcmp(get_ecc_shared_secret(algo), shared_secret,
				    params.shared_secret_sz));
}

static void test_ecc_ecdh_invalid_param(sid_pal_ecc_algo_t algo, size_t prk_sz, size_t puk_sz)
{
	uint8_t shared_secret[32];
	uint8_t prk[64], puk[64];
	sid_pal_ecdh_params_t params = {
		.algo = algo,
		.prk = prk,
		.prk_size = prk_sz,
		.puk = puk,
		.puk_size = 32,
		.shared_secret = shared_secret,
		.shared_secret_sz = sizeof(shared_secret),
	};

	params.algo = 0;
	sid_error_t ret = sid_pal_crypto_ecc_ecdh(&params);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = algo;
	params.prk = NULL;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	params.prk = prk;
	params.prk_size = 0;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	params.prk_size = prk_sz;
	params.puk = NULL;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	params.puk = puk;
	params.puk_size = 0;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	params.puk_size = puk_sz;
	params.shared_secret = NULL;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	params.shared_secret = shared_secret;
	params.shared_secret_sz = 0;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

static void ecc_ecdh_negative(sid_pal_ecc_algo_t algo, size_t prk_sz, size_t puk_sz)
{
	sid_pal_ecdh_params_t params;
	sid_pal_ecc_key_gen_params_t key_params;
	uint8_t prk[64], puk[64];
	uint8_t shared_secret[64], shared_secret_remote[64];

	// Prk + remote_puk != Remote prk + different puk

	key_params.algo = algo;
	key_params.prk_size = prk_sz;
	key_params.puk_size = puk_sz;
	key_params.prk = prk;
	key_params.puk = puk;
	sid_error_t ret = sid_pal_crypto_ecc_key_gen(&key_params);

	/* Not all platforms support ECDH_SECP256R1, stop testing this now if so */
	if (SID_ERROR_NOSUPPORT == ret && SID_PAL_ECDH_SECP256R1 == algo) {
		return;
	}
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.prk = prk;
	params.prk_size = prk_sz;
	params.puk = get_ecc_remote_puk(algo);
	params.puk_size = puk_sz;
	params.shared_secret = shared_secret;
	params.shared_secret_sz = 32;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.prk = get_ecc_remote_prk(algo);
	params.prk_size = prk_sz;
	params.puk = get_ecc_puk(algo);
	params.puk_size = puk_sz;
	params.shared_secret = shared_secret_remote;
	params.shared_secret_sz = 32;
	ret = sid_pal_crypto_ecc_ecdh(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0,
			      memcmp(shared_secret, shared_secret_remote, params.shared_secret_sz));
}

static void ecc_dsa_positive(sid_pal_ecc_algo_t algo, size_t prk_sz, size_t puk_sz)
{
	sid_error_t ret;
	sid_pal_dsa_params_t params;
	sid_pal_ecc_key_gen_params_t key_params;
	uint8_t prk_key[64];
	uint8_t puk_key[64];
	uint8_t signature[64];

	// Local generate keypair
	key_params.algo = algo;
	key_params.prk_size = prk_sz;
	key_params.puk_size = puk_sz;
	key_params.prk = prk_key;
	key_params.puk = puk_key;
	ret = sid_pal_crypto_ecc_key_gen(&key_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = prk_key;
	params.key_size = key_params.prk_size;
	params.in = sign_hash_msg;
	params.in_size = sizeof(sign_hash_msg);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = puk_key;
	params.key_size = key_params.puk_size;
	params.in = sign_hash_msg;
	params.in_size = sizeof(sign_hash_msg);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	// use non word aligned msg
	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = prk_key;
	params.key_size = key_params.prk_size;
	params.in = sign_hash_msg_noalign;
	params.in_size = sizeof(sign_hash_msg_noalign);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = puk_key;
	params.key_size = key_params.puk_size;
	params.in = sign_hash_msg_noalign;
	params.in_size = sizeof(sign_hash_msg_noalign);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	// Use external keypair
	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = get_ecc_prk(algo);
	params.key_size = key_params.prk_size;
	params.in = sign_hash_msg;
	params.in_size = sizeof(sign_hash_msg);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = get_ecc_puk(algo);
	params.key_size = key_params.puk_size;
	params.in = sign_hash_msg;
	params.in_size = sizeof(sign_hash_msg);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = get_ecc_prk(algo);
	params.key_size = key_params.prk_size;
	params.in = sign_hash_msg_noalign;
	params.in_size = sizeof(sign_hash_msg_noalign);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = get_ecc_puk(algo);
	params.key_size = key_params.puk_size;
	params.in = sign_hash_msg_noalign;
	params.in_size = sizeof(sign_hash_msg_noalign);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
}

static void test_ecc_dsa_invalid_params(sid_pal_ecc_algo_t algo, size_t prk_sz, size_t puk_sz)
{
	uint8_t sig[64];
	sid_pal_dsa_params_t params = {
		.algo = algo,
		.mode = SID_PAL_CRYPTO_SIGN,
		.key = get_ecc_prk(algo),
		.key_size = prk_sz,
		.in = sign_hash_msg,
		.in_size = sizeof(sign_hash_msg),
		.signature = sig,
		.sig_size = sizeof(sig),
	};

	params.mode = 0;
	sid_error_t ret = sid_pal_crypto_ecc_dsa(&params);

	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = NULL;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.key = get_ecc_prk(algo);
	params.key_size = 0;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.key_size = prk_sz;
	params.in = NULL;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.in = sign_hash_msg;
	params.in_size = 0;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.in_size = 0;
	params.signature = NULL;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.signature = sig;
	params.sig_size = 0;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	// VERIFY
	params.mode = 0;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = NULL;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.key = get_ecc_puk(algo);
	params.key_size = 0;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.key_size = puk_sz;
	params.in = NULL;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.in = sign_hash_msg;
	params.in_size = 0;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.in_size = 0;
	params.signature = NULL;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);

	params.signature = sig;
	params.sig_size = 0;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(ret, SID_ERROR_NONE);
}

static void ecc_dsa_negative(sid_pal_ecc_algo_t algo, size_t prk_sz, size_t puk_sz)
{
	sid_error_t ret;
	sid_pal_dsa_params_t params;
	sid_pal_ecc_key_gen_params_t key_params;
	uint8_t prk_key[64];
	uint8_t puk_key[64];
	uint8_t signature[64];

	// Use invalid keypair to try and verify signature

	key_params.algo = algo;
	key_params.prk_size = prk_sz;
	key_params.puk_size = puk_sz;
	key_params.prk = prk_key;
	key_params.puk = puk_key;
	ret = sid_pal_crypto_ecc_key_gen(&key_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = prk_key;
	params.key_size = key_params.prk_size;
	params.in = sign_hash_msg;
	params.in_size = sizeof(sign_hash_msg);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = get_ecc_puk(algo);
	params.key_size = key_params.puk_size;
	params.in = sign_hash_msg;
	params.in_size = sizeof(sign_hash_msg);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = get_ecc_prk(algo);
	params.key_size = key_params.prk_size;
	params.in = sign_hash_msg;
	params.in_size = sizeof(sign_hash_msg);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

	params.algo = key_params.algo;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = puk_key;
	params.key_size = key_params.puk_size;
	params.in = sign_hash_msg;
	params.in_size = sizeof(sign_hash_msg);
	params.signature = signature;
	params.sig_size = 64;
	ret = sid_pal_crypto_ecc_dsa(&params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

void setUp(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());
}

void tearDown(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_deinit());
}

void test_random_number_gen_positive(void)
{
	uint8_t buf1[sizeof(zero_buffer)] = { 0 };
	uint8_t buf2[sizeof(zero_buffer)] = { 0 };
	sid_error_t ret = sid_pal_crypto_rand(buf1, sizeof(buf1));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(zero_buffer, buf1, sizeof(buf1)));

	ret = sid_pal_crypto_rand(buf2, sizeof(buf2));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(zero_buffer, buf2, sizeof(buf2)));

	TEST_ASSERT_NOT_EQUAL(0, memcmp(buf1, buf2, sizeof(buf1)));
}

void test_random_number_gen_invalid_param(void)
{
	uint8_t buf1[sizeof(zero_buffer)];
	sid_error_t ret = sid_pal_crypto_rand(buf1, 0);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	ret = sid_pal_crypto_rand(NULL, sizeof(buf1));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

void test_hash_positive(void)
{
	sid_pal_hash_params_t hash_params;
	uint8_t out_buf[64];

	hash_params.data = hash_string;
	hash_params.data_size = strlen((char *)hash_string);
	hash_params.digest = out_buf;
	hash_params.digest_size = SHA256_SZ;
	hash_params.algo = SID_PAL_HASH_SHA256;

	sid_error_t ret = sid_pal_crypto_hash(&hash_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, sha256_result, hash_params.digest_size));

	hash_params.algo = SID_PAL_HASH_SHA512;
	hash_params.digest_size = SHA512_SZ;
	ret = sid_pal_crypto_hash(&hash_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, sha512_result, hash_params.digest_size));
}

void test_hash_invalid_param_sha256(void)
{
	test_hash_invalid_param(SID_PAL_HASH_SHA256);
}

void test_hash_invalid_param_sha512(void)
{
	test_hash_invalid_param(SID_PAL_HASH_SHA512);
}

void test_hmac_positive(void)
{
	uint8_t out_buf[64];
	sid_pal_hmac_params_t hmac_params;

	hmac_params.algo = SID_PAL_HASH_SHA256;
	hmac_params.key = hmac_key;
	hmac_params.key_size = 16;
	hmac_params.data = hmac_hash_string;
	hmac_params.data_size = strlen((char *)hmac_hash_string);
	hmac_params.digest = out_buf;
	hmac_params.digest_size = SHA256_SZ;

	sid_error_t ret = sid_pal_crypto_hmac(&hmac_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, hmac_sha256_result, hmac_params.digest_size));

	hmac_params.algo = SID_PAL_HASH_SHA512;
	hmac_params.digest_size = SHA512_SZ;
	ret = sid_pal_crypto_hmac(&hmac_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, hmac_sha512_result, hmac_params.digest_size));
}

void test_hmac_invalid_param_sha256(void)
{
	test_hmac_invalid_param(SID_PAL_HASH_SHA256);
}

void test_hmac_invalid_param_sha512(void)
{
	test_hmac_invalid_param(SID_PAL_HASH_SHA256);
}

void test_hmac_negative(void)
{
	uint8_t out_buf[64];
	sid_pal_hmac_params_t hmac_params = {
		.algo = SID_PAL_HASH_SHA256,
		.key = hmac_invalid_key,
		.key_size = 16,
		.data = hmac_hash_string,
		.data_size = strlen((char *)hmac_hash_string),
		.digest = out_buf,
		.digest_size = SHA256_SZ,
	};

	sid_error_t ret = sid_pal_crypto_hmac(&hmac_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, hmac_sha256_result, hmac_params.digest_size));

	hmac_params.algo = SID_PAL_HASH_SHA512;
	hmac_params.digest_size = SHA512_SZ;
	ret = sid_pal_crypto_hmac(&hmac_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, hmac_sha512_result, hmac_params.digest_size));
}

void test_cmac_positive(void)
{
	sid_pal_aes_params_t aes_params = { 0 };
	uint8_t out_buf[64];

	aes_params.algo = SID_PAL_AES_CMAC_128;
	aes_params.mode = SID_PAL_CRYPTO_MAC_CALCULATE;
	aes_params.key = aes_key;
	aes_params.key_size = 16 * 8; // 128 bits
	aes_params.in = aes_plaintext;
	aes_params.in_size = sizeof(aes_plaintext);
	aes_params.out = out_buf;
	aes_params.out_size = 16;

	sid_error_t ret = sid_pal_crypto_aes_crypt(&aes_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, aes_mac, aes_params.out_size));
}

void test_cmac_invalid_param(void)
{
	uint8_t out_buf[16];
	sid_pal_aes_params_t aes_params = {
		.algo = SID_PAL_AES_CMAC_128,
		.mode = SID_PAL_CRYPTO_MAC_CALCULATE,
		.key = aes_key,
		.key_size = 16 * 8,
		.in = aes_plaintext,
		.in_size = sizeof(aes_plaintext),
		.out = out_buf,
		.out_size = 16,
	};

	aes_params.algo = 0;
	sid_error_t ret = sid_pal_crypto_aes_crypt(&aes_params);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.algo = SID_PAL_AES_CMAC_128;
	aes_params.mode = 0;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.mode = SID_PAL_CRYPTO_MAC_CALCULATE;
	aes_params.key = NULL;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.key = aes_key;
	aes_params.key_size = 16; // 16 bits
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.key_size = 16 * 8; // 128 bits
	aes_params.in = NULL;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.in = aes_plaintext;
	aes_params.in_size = 0;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.in_size = sizeof(aes_plaintext);
	aes_params.out = NULL;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.out = out_buf;
	aes_params.out_size = 0;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

void test_cmac_invalid_input_combo(void)
{
	uint8_t out_buf[64];
	sid_pal_aes_params_t aes_params = {
		.algo = SID_PAL_AES_CMAC_128,
		.mode = SID_PAL_CRYPTO_MAC_CALCULATE,
		.key = aes_key,
		.key_size = 16 * 8, // 128 bits
		.in = aes_plaintext,
		.in_size = sizeof(aes_plaintext),
		.out = out_buf,
		.out_size = 16,
	};

	aes_params.mode = 0;
	sid_error_t ret = sid_pal_crypto_aes_crypt(&aes_params);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.mode = SID_PAL_CRYPTO_MAC_CALCULATE;
	aes_params.key_size = 16; // 16 bits
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.key_size = 16 * 8; // 128 bits
	aes_params.out_size = 8;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

void test_cmac_negative(void)
{
	uint8_t out_buf[64];
	sid_pal_aes_params_t aes_params = {
		.algo = SID_PAL_AES_CMAC_128,
		.mode = SID_PAL_CRYPTO_MAC_CALCULATE,
		.key = aes_invalid_key,
		.key_size = 16 * 8, // 128 bits
		.in = aes_plaintext,
		.in_size = sizeof(aes_plaintext),
		.out = out_buf,
		.out_size = 16,
	};

	sid_error_t ret = sid_pal_crypto_aes_crypt(&aes_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_mac, aes_params.out_size));

	aes_params.key = aes_key;
	aes_params.in = aes_invalid_plaintext;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_mac, aes_params.out_size));
}

void test_gcm_positive(void)
{
	sid_pal_aead_params_t aead_params = { 0 };
	uint8_t out_buf[64];
	uint8_t out_mac_buf[16];

	aead_params.algo = SID_PAL_AEAD_GCM_128;
	aead_params.iv = aes_gcm_iv;
	aead_params.iv_size = sizeof(aes_gcm_iv);
	aead_params.aad = aes_gcm_aad;
	aead_params.aad_size = sizeof(aes_gcm_aad);
	aead_params.key = aes_gcm_key;
	aead_params.key_size = 16 * 8; // 128 bits
	aead_params.mode = SID_PAL_CRYPTO_ENCRYPT;
	aead_params.in = aes_gcm_plaintext;
	aead_params.in_size = sizeof(aes_gcm_plaintext);
	aead_params.out = out_buf;
	aead_params.out_size = sizeof(aes_gcm_ciphertext);
	aead_params.mac = out_mac_buf;
	aead_params.mac_size = sizeof(aes_gcm_mac);

	sid_error_t ret = sid_pal_crypto_aead_crypt(&aead_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, aes_gcm_ciphertext, aead_params.out_size));
	TEST_ASSERT_EQUAL(0, memcmp(out_mac_buf, aes_gcm_mac, 16));

	aead_params.mode = SID_PAL_CRYPTO_DECRYPT;
	aead_params.in = aes_gcm_ciphertext;
	aead_params.in_size = sizeof(aes_gcm_ciphertext);
	aead_params.out = out_buf;
	aead_params.out_size = sizeof(aes_gcm_plaintext);
	aead_params.mac = (uint8_t *)aes_gcm_mac;
	aead_params.mac_size = sizeof(aes_gcm_mac);

	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, aes_gcm_plaintext, aead_params.out_size));
}

void test_gcm_invalid_params_encrypt(void)
{
	test_gcm_invalid_params(SID_PAL_CRYPTO_ENCRYPT);
}

void test_gcm_invalid_params_decrypt(void)
{
	test_gcm_invalid_params(SID_PAL_CRYPTO_DECRYPT);
}

void test_gcm_negative(void)
{
	uint8_t out_buf[64];
	uint8_t out_mac_buf[16];

	sid_pal_aead_params_t aead_params = {
		.algo = SID_PAL_AEAD_GCM_128,
		.mode = SID_PAL_CRYPTO_ENCRYPT,
		.iv = aes_gcm_iv,
		.iv_size = sizeof(aes_gcm_iv),
		.aad = aes_gcm_aad,
		.aad_size = sizeof(aes_gcm_aad),
		.key = aes_gcm_key,
		.key_size = 16 * 8, // 128 bits
		.in = aes_gcm_plaintext,
		.in_size = sizeof(aes_gcm_plaintext),
		.out = out_buf,
		.out_size = sizeof(aes_gcm_ciphertext),
		.mac = out_mac_buf,
		.mac_size = sizeof(aes_gcm_mac),
	};

	aead_params.iv = aes_gcm_invalid_iv;
	sid_error_t ret = sid_pal_crypto_aead_crypt(&aead_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_gcm_ciphertext, aead_params.out_size));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_mac_buf, aes_gcm_mac, aead_params.mac_size));

	aead_params.iv = aes_gcm_iv;
	aead_params.aad = aes_gcm_invalid_aad;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_mac_buf, aes_gcm_mac, aead_params.mac_size));

	aead_params.aad = aes_gcm_aad;
	aead_params.key = aes_gcm_invalid_key;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_gcm_ciphertext, aead_params.out_size));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_mac_buf, aes_gcm_mac, aead_params.mac_size));

	aead_params.key = aes_gcm_key;

	// Decrypt
	aead_params.mode = SID_PAL_CRYPTO_DECRYPT;
	aead_params.in = aes_gcm_ciphertext;
	aead_params.out = out_buf;
	aead_params.mac = (uint8_t *)aes_gcm_mac;

	aead_params.iv = aes_gcm_invalid_iv;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.iv = aes_gcm_iv;
	aead_params.aad = aes_gcm_invalid_aad;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.aad = aes_gcm_aad;
	aead_params.key = aes_gcm_invalid_key;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.key = aes_gcm_key;
	aead_params.in = aes_gcm_invalid_ciphertext;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.key = aes_gcm_key;
	aead_params.in = (uint8_t *)aes_gcm_invalid_ciphertext;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aead_params.in = (uint8_t *)aes_gcm_ciphertext;
	aead_params.mac = (uint8_t *)aes_gcm_invalid_mac;
	ret = sid_pal_crypto_aead_crypt(&aead_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

void test_ctr_positive(void)
{
	sid_pal_aes_params_t aes_params = { 0 };
	uint8_t out_buf[64];

	aes_params.algo = SID_PAL_AES_CTR_128;
	aes_params.mode = SID_PAL_CRYPTO_ENCRYPT;
	aes_params.iv = aes_ctr_iv;
	aes_params.iv_size = sizeof(aes_ctr_iv);
	aes_params.key = aes_ctr_key;
	aes_params.key_size = 16 * 8; // 128 bits
	aes_params.in = aes_ctr_plaintext;
	aes_params.in_size = sizeof(aes_ctr_plaintext);
	aes_params.out = out_buf;
	aes_params.out_size = sizeof(aes_ctr_ciphertext);

	sid_error_t ret = sid_pal_crypto_aes_crypt(&aes_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, aes_ctr_ciphertext, aes_params.out_size));

	aes_params.mode = SID_PAL_CRYPTO_DECRYPT;
	aes_params.in = aes_ctr_ciphertext;
	aes_params.in_size = sizeof(aes_ctr_ciphertext);
	aes_params.out = out_buf;
	aes_params.out_size = sizeof(aes_ctr_plaintext);

	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_EQUAL(0, memcmp(out_buf, aes_ctr_plaintext, aes_params.out_size));
}

void test_ctr_invalid_params_encrypt(void)
{
	test_ctr_invalid_params(SID_PAL_CRYPTO_ENCRYPT);
}

void test_ctr_invalid_params_decrypt(void)
{
	test_ctr_invalid_params(SID_PAL_CRYPTO_DECRYPT);
}

void test_ctr_invalid_input_combo(void)
{
	uint8_t out_buf[64];
	sid_pal_aes_params_t aes_params = {
		.algo = SID_PAL_AES_CTR_128,
		.mode = SID_PAL_CRYPTO_ENCRYPT,
		.iv = aes_ctr_iv,
		.iv_size = sizeof(aes_ctr_iv),
		.key = aes_ctr_key,
		.key_size = 16 * 8, // 128 bits
		.in = aes_ctr_plaintext,
		.in_size = sizeof(aes_ctr_plaintext),
		.out = out_buf,
		.out_size = sizeof(aes_ctr_ciphertext),
	};

	aes_params.iv_size -= 1;
	sid_error_t ret = sid_pal_crypto_aes_crypt(&aes_params);

	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.iv_size += 1;
	aes_params.key_size = 16; // 16 bits
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.mode = SID_PAL_CRYPTO_DECRYPT;
	aes_params.in = aes_ctr_ciphertext;
	aes_params.in_size = sizeof(aes_ctr_ciphertext);
	aes_params.out = out_buf;
	aes_params.out_size = sizeof(aes_ctr_plaintext);

	aes_params.iv_size -= 1;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

	aes_params.iv_size += 1;
	aes_params.key_size = 16; // 16 bits
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

void test_ctr_negative(void)
{
	uint8_t out_buf[64];
	sid_pal_aes_params_t aes_params = {
		.algo = SID_PAL_AES_CTR_128,
		.mode = SID_PAL_CRYPTO_ENCRYPT,
		.iv = aes_ctr_iv,
		.iv_size = sizeof(aes_ctr_iv),
		.key = aes_ctr_key,
		.key_size = 16 * 8, // 128 bits
		.in = aes_ctr_plaintext,
		.in_size = sizeof(aes_ctr_plaintext),
		.out = out_buf,
		.out_size = sizeof(aes_ctr_ciphertext),
	};

	aes_params.iv = aes_ctr_invalid_iv;
	sid_error_t ret = sid_pal_crypto_aes_crypt(&aes_params);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_ctr_ciphertext, aes_params.out_size));

	aes_params.iv = aes_ctr_iv;
	aes_params.key = aes_ctr_invalid_key;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_ctr_ciphertext, aes_params.out_size));

	aes_params.mode = SID_PAL_CRYPTO_DECRYPT;
	aes_params.in = aes_ctr_ciphertext;
	aes_params.in_size = sizeof(aes_ctr_ciphertext);
	aes_params.out = out_buf;
	aes_params.out_size = sizeof(aes_ctr_plaintext);

	aes_params.iv = aes_ctr_invalid_iv;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_ctr_plaintext, aes_params.out_size));

	aes_params.iv = aes_ctr_iv;
	aes_params.key = aes_ctr_invalid_key;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_ctr_plaintext, aes_params.out_size));

	aes_params.key = aes_ctr_key;
	aes_params.in = aes_ctr_invalid_ciphertext;
	ret = sid_pal_crypto_aes_crypt(&aes_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(out_buf, aes_ctr_plaintext, aes_params.out_size));
}

void test_ecc_key_gen_positive(void)
{
	uint8_t prk[64], puk[64];

	sid_error_t ret = ecc_key_gen_positive(SID_PAL_ECDH_CURVE25519, prk, 32, puk, 32);

	TEST_ASSERT_EQUAL(ret, SID_ERROR_NONE);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(prk, zero_buffer, 32));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(puk, zero_buffer, 32));

	ret = ecc_key_gen_positive(SID_PAL_EDDSA_ED25519, prk, 32, puk, 32);
	TEST_ASSERT_EQUAL(ret, SID_ERROR_NONE);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(prk, zero_buffer, 32));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(puk, zero_buffer, 32));

	ret = ecc_key_gen_positive(SID_PAL_ECDSA_SECP256R1, prk, 32, puk, 64);
	TEST_ASSERT_EQUAL(ret, SID_ERROR_NONE);
	TEST_ASSERT_NOT_EQUAL(0, memcmp(prk, zero_buffer, 32));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(puk, zero_buffer, 64));
}

void test_ecc_key_gen_invalid_param_ecdh_curve25519(void)
{
	test_ecc_key_gen_invalid_param(SID_PAL_ECDH_CURVE25519, 32, 32);
}

void test_ecc_key_gen_invalid_param_eddsa_ed25519(void)
{
	test_ecc_key_gen_invalid_param(SID_PAL_EDDSA_ED25519, 32, 32);
}

void test_ecc_key_gen_invalid_param_ecdsa_secp256r1(void)
{
	test_ecc_key_gen_invalid_param(SID_PAL_ECDSA_SECP256R1, 32, 64);
}

void test_ecc_ecdh_curve25519_positive(void)
{
	ecc_ecdh_positive(SID_PAL_ECDH_CURVE25519, 32, 32);
}

void test_ecc_ecdh_secp256r1_positive(void)
{
	ecc_ecdh_positive(SID_PAL_ECDH_SECP256R1, 32, 64);
}

void test_ecc_ecdh_invalid_param_ecdh_curve25519(void)
{
	test_ecc_ecdh_invalid_param(SID_PAL_ECDH_CURVE25519, 32, 32);
}

void test_ecc_ecdh_invalid_param_ecdh_secp256r1(void)
{
	test_ecc_ecdh_invalid_param(SID_PAL_ECDH_SECP256R1, 32, 64);
}

void test_ecc_ecdh_curve25519_negative(void)
{
	ecc_ecdh_negative(SID_PAL_ECDH_CURVE25519, 32, 32);
}

void test_ecc_ecdh_secp256r1_negative(void)
{
	ecc_ecdh_negative(SID_PAL_ECDH_SECP256R1, 32, 64);
}

void test_ecc_dsa_ed25519_positive(void)
{
	ecc_dsa_positive(SID_PAL_EDDSA_ED25519, 32, 32);
}

void test_ecc_dsa_secp256r1_positive(void)
{
	ecc_dsa_positive(SID_PAL_ECDSA_SECP256R1, 32, 64);
}

void test_ecc_dsa_invalid_param_eddsa_ed25519(void)
{
	test_ecc_dsa_invalid_params(SID_PAL_EDDSA_ED25519, 32, 32);
}

void test_ecc_dsa_invalid_param_ecdsa_secp256r1(void)
{
	test_ecc_dsa_invalid_params(SID_PAL_ECDSA_SECP256R1, 32, 64);
}

void test_ecc_dsa_ed25519_negative(void)
{
	ecc_dsa_negative(SID_PAL_EDDSA_ED25519, 32, 32);
}

void test_ecc_dsa_secp256r1_negative(void)
{
	ecc_dsa_negative(SID_PAL_ECDSA_SECP256R1, 32, 64);
}

extern int unity_main(void);

int main(void)
{
	return unity_main();
}
