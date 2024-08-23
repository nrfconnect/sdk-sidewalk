/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <sid_pal_crypto_ifc.h>
#include <zephyr/kernel.h>
#include <string.h>

#define RNG_BUFF_MAX_SIZE (128)

#define SHA256_LEN (32)
#define SHA512_LEN (64)
#define SHA_MAX_DIGEST_LEN (SHA512_LEN)

#define HMAC_MAX_BLOCK_SIZE (32)

#define HASH_TEST_DATA_BLOCK_SIZE (128)
#define AES_MAX_BLOCK_SIZE (16)
#define AES_TEST_DATA_BLOCK_SIZE (128)
#define AES_CMAC_TEST_DATA_BLOCK_SIZE (256)
#define AES_GCM_TEST_DATA_BLOCK_SIZE (128)
#define AES_CTR_TEST_DATA_BLOCK_SIZE (64)
#define HMAC_TEST_DATA_BLOCK_SIZE (128)
#define ECC_TEST_DATA_BLOCK_SIZE (128)

#define AES_IV_SIZE (16)
#define AES_GCM_IV_SIZE (12)
#define AES_CCM_IV_SIZE (13)

#define EC_ED25519_PRIV_KEY_LEN (32)
#define EC_CURVE25519_PRIV_KEY_LEN (32)
#define EC_SECP256R1_PRIV_KEY_LEN (32)
#define ECC_PRIVATE_KEY_MAX_LEN (EC_SECP256R1_PRIV_KEY_LEN)

#define EC_ED25519_PUB_KEY_LEN (32)
#define EC_CURVE25519_PUB_KEY_LEN (32)
#define EC_SECP256R1_PUB_KEY_LEN (64)
#define ECC_PUBLIC_KEY_MAX_LEN (EC_SECP256R1_PUB_KEY_LEN)

#define ECDH_SECRET_SIZE (32)

#define ECDSA_SIGNATURE_SIZE (64)

static const uint8_t test_string[AES_TEST_DATA_BLOCK_SIZE] = {
	"Nordic Semiconductor is a Norwegian fabless semiconductor company specializing in ... "
};

uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE] = { 0xAC, 0x1D, 0x05, 0x22, 0xAC, 0x1D, 0x05, 0x22,
						 0xFA, 0xD4, 0xCC, 0x29, 0xFA, 0xD4, 0xCC, 0x29 };

uint8_t hmac_test_key[HMAC_MAX_BLOCK_SIZE] = { 0xAC, 0x1D, 0x05, 0x22, 0xAC, 0x1D, 0x05, 0x22,
					       0xFA, 0xD4, 0xCC, 0x29, 0xFA, 0xD4, 0xCC, 0x29,
					       0xDA, 0x3C, 0xEE, 0xA4, 0x82, 0x0D, 0xAA, 0x50,
					       0xAC, 0xFE, 0xBB, 0x34, 0x1D, 0x05, 0x22, 0xAC };

static void sid_pal_crypto_aead_test_execute(sid_pal_aead_params_t *params)
{
	uint8_t data_copy[AES_GCM_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];

	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(encrypted_data, 0x00, sizeof(encrypted_data));
	memset(decrypted_data, 0x00, sizeof(decrypted_data));

	params->key = aes_128_test_key;
	params->key_size = sizeof(aes_128_test_key) * 8;
	params->aad = additional_data;
	params->aad_size = sizeof(additional_data);
	params->mac = mac;

	params->mode = SID_PAL_CRYPTO_ENCRYPT;
	params->in = data_copy;
	params->in_size = sizeof(data_copy);
	params->out = encrypted_data;
	params->out_size = sizeof(encrypted_data);
	params->mac_size = sizeof(mac);

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(params));

	// Key and IV are the same
	params->mode = SID_PAL_CRYPTO_DECRYPT;
	params->in = encrypted_data;
	params->in_size = sizeof(encrypted_data);
	params->out = decrypted_data;
	params->out_size = sizeof(decrypted_data);

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(params));
}

ZTEST(crypto, RNG)
{
	uint8_t rng_buff[RNG_BUFF_MAX_SIZE];

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_rand(rng_buff, sizeof(rng_buff)));
}

ZTEST(crypto, SHA_256)
{
	sid_pal_hash_params_t params;

	// SHA must be calculated from RAM, in another case result can be unpredicted.
	uint8_t data_copy[HASH_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(digest, 0x00, sizeof(digest));
	memcpy(data_copy, test_string, sizeof(data_copy));

	// Compute SHA2_256
	params.algo = SID_PAL_HASH_SHA256;
	params.data = data_copy;
	params.data_size = sizeof(data_copy);
	params.digest = digest;
	params.digest_size = SHA256_LEN;

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_hash(&params));
}

#if defined(CONFIG_SOC_NRF54L15)

ZTEST(crypto, SHA_512)
{
	sid_pal_hash_params_t params;

	// SHA must be calculated from RAM, in another case result can be unpredicted.
	uint8_t data_copy[HASH_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(digest, 0x00, sizeof(digest));
	memcpy(data_copy, test_string, sizeof(data_copy));

	// Compute SHA2_512
	params.algo = SID_PAL_HASH_SHA512;
	params.data = data_copy;
	params.data_size = sizeof(data_copy);
	params.digest = digest;
	params.digest_size = SHA512_LEN;

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_hash(&params));
}

#endif /* CONFIG_SOC_NRF54L15 */

/**********************************************
* HMAC
* ********************************************/

ZTEST(crypto, HMAC_SHA256)
{
	sid_pal_hmac_params_t params;
	uint8_t data_copy[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	// Test precondition
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(digest, 0x00, sizeof(digest));

	// SHA2_256
	params.algo = SID_PAL_HASH_SHA256;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data_copy;
	params.data_size = sizeof(data_copy);
	params.digest = digest;
	params.digest_size = SHA256_LEN;

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
}

#if defined(CONFIG_SOC_NRF54L15)

ZTEST(crypto, HMAC_SHA512)
{
	sid_pal_hmac_params_t params;
	uint8_t data_copy[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	// Test precondition
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(digest, 0x00, sizeof(digest));

	// SHA2_512
	params.algo = SID_PAL_HASH_SHA512;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data_copy;
	params.data_size = sizeof(data_copy);
	params.digest = digest;
	params.digest_size = SHA512_LEN;

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
}

#endif /* CONFIG_SOC_NRF54L15 */

/**********************************************
* END HMAC
* ********************************************/

ZTEST(crypto, AES_CMAC)
{
	sid_pal_aes_params_t params;

	uint8_t data_copy[AES_CMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t cmac[AES_MAX_BLOCK_SIZE];

	// CMAC calculation
	params.algo = SID_PAL_AES_CMAC_128;
	params.mode = SID_PAL_CRYPTO_MAC_CALCULATE;
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.out = cmac;
	params.out_size = sizeof(cmac);
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = NULL;

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));
}

ZTEST(crypto, AES_CTR)
{
	sid_pal_aes_params_t params;

	uint8_t data_copy[AES_CTR_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));

	// Common IV
	memset(iv, 0xC1, sizeof(iv));
	// Reset buffers
	memset(encrypted_data, 0x00, sizeof(encrypted_data));
	memset(decrypted_data, 0x00, sizeof(decrypted_data));

	// Encrypt/decrypt data
	params.algo = SID_PAL_AES_CTR_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));

	// Key and iv are the same.
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.in = encrypted_data;
	params.in_size = sizeof(encrypted_data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));
}

/**********************************************
* AEAD
* ********************************************/

ZTEST(crypto, AEAD_GCM)
{
	sid_pal_aead_params_t params;
	uint8_t iv[AES_GCM_IV_SIZE];

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(iv, 0xCC, AES_GCM_IV_SIZE);

	params.algo = SID_PAL_AEAD_GCM_128;
	params.iv = iv;
	params.iv_size = AES_GCM_IV_SIZE;
	sid_pal_crypto_aead_test_execute(&params);
}

ZTEST(crypto, AEAD_CCM)
{
	sid_pal_aead_params_t params;

	uint8_t data_copy[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t iv[AES_CCM_IV_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(iv, 0xB1, AES_CCM_IV_SIZE);
	memset(encrypted_data, 0x00, sizeof(encrypted_data));
	memset(decrypted_data, 0x00, sizeof(decrypted_data));

	// CCM_128
	params.algo = SID_PAL_AEAD_CCM_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_CCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));

	// Key and IV are the same
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.in = encrypted_data;
	params.in_size = sizeof(encrypted_data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);
	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	zassert_equal(0, memcmp(data_copy, decrypted_data, sizeof(data_copy)));
}

/**********************************************
* END AEAD
* ********************************************/

ZTEST(crypto, ECDSA_SECP256R1)
{
	sid_pal_dsa_params_t params;

	uint8_t data_copy[ECC_TEST_DATA_BLOCK_SIZE];

	uint8_t signature[ECDSA_SIGNATURE_SIZE];

	// Keys are generated in the openssl.
	uint8_t private_SECP256R1[EC_SECP256R1_PRIV_KEY_LEN] = {
		0xd0, 0xb2, 0x29, 0xdd, 0x13, 0x97, 0x62, 0xee, 0xca, 0x9a, 0xb9,
		0x28, 0x57, 0x82, 0xbf, 0x76, 0xd9, 0x5e, 0xf0, 0x7f, 0x5c, 0x84,
		0x5f, 0x51, 0x3f, 0x46, 0x19, 0x59, 0x91, 0x00, 0x10, 0xed
	};
	uint8_t public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN] = {
		0xe4, 0xe9, 0x19, 0x68, 0x99, 0xdd, 0x4e, 0x1e, 0xf6, 0xfd, 0xe6, 0x81, 0xa9,
		0x0d, 0x1e, 0x61, 0x65, 0xf6, 0x42, 0x2b, 0xe1, 0x9e, 0xf5, 0x0c, 0x8e, 0x4d,
		0x64, 0x64, 0x27, 0xfc, 0xda, 0x5a, 0xf1, 0xb2, 0x6d, 0x27, 0xbf, 0xe0, 0x79,
		0xdd, 0x50, 0x71, 0x89, 0xeb, 0x4a, 0xfb, 0x55, 0x4d, 0x79, 0x78, 0x93, 0xab,
		0xf5, 0x94, 0xec, 0x2c, 0x85, 0x33, 0x99, 0x87, 0x5c, 0x23, 0x1c, 0x37
	};

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(&signature, 0x00, sizeof(signature));
	memcpy(data_copy, test_string, sizeof(data_copy));

	// SID_PAL_ECDSA_SECP256R1
	params.algo = SID_PAL_ECDSA_SECP256R1;

	// Sign message
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = private_SECP256R1;
	params.key_size = sizeof(private_SECP256R1);
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.signature = signature;
	params.sig_size = sizeof(signature);
	// SID_ERROR_NONE means signature is match
	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

	// Verify message
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_SECP256R1;
	params.key_size = sizeof(public_SECP256R1);
	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
}

ZTEST(crypto, EDDSA_ED25519)
{
	sid_pal_dsa_params_t params;

	uint8_t data_copy[ECC_TEST_DATA_BLOCK_SIZE];

	uint8_t signature[ECDSA_SIGNATURE_SIZE];

	uint8_t private_Ed25519[EC_ED25519_PRIV_KEY_LEN] = {
		0x81, 0xa5, 0x9c, 0x6b, 0xd7, 0xcc, 0xe8, 0x07, 0xa1, 0xb7, 0xa7,
		0x25, 0xfc, 0xed, 0xb0, 0x65, 0xea, 0x3c, 0xac, 0x36, 0x90, 0x5e,
		0xf4, 0x1b, 0x05, 0x34, 0x1e, 0x5c, 0x0b, 0x72, 0x2d, 0x40
	};
	uint8_t public_Ed25519[EC_ED25519_PRIV_KEY_LEN] = {
		0x67, 0xcb, 0x27, 0xa7, 0xe4, 0x45, 0x28, 0x9c, 0x8f, 0x0a, 0xc2,
		0x42, 0x6d, 0x0c, 0x05, 0x32, 0xce, 0xda, 0x4b, 0xc2, 0x77, 0xfe,
		0x2a, 0x01, 0x5a, 0xc0, 0xb8, 0x91, 0xbe, 0xd3, 0x50, 0x29
	};

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(&signature, 0x00, sizeof(signature));
	memcpy(data_copy, test_string, sizeof(data_copy));

	memset(signature, 0x00, sizeof(signature));

	// SID_PAL_EDDSA_ED25519
	params.algo = SID_PAL_EDDSA_ED25519;

	// Sign message
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = private_Ed25519;
	params.key_size = sizeof(private_Ed25519);
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.signature = signature;
	params.sig_size = sizeof(signature);
	// SID_ERROR_NONE means signature is match
	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

	// Verify message
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_Ed25519;
	params.key_size = sizeof(public_Ed25519);
	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
}

ZTEST(crypto, GENERATE_ECC_KEY)
{
	sid_pal_ecc_key_gen_params_t params;

	size_t prk_size_test_vector[] = { EC_CURVE25519_PRIV_KEY_LEN, EC_SECP256R1_PRIV_KEY_LEN,
					  EC_ED25519_PRIV_KEY_LEN, EC_SECP256R1_PRIV_KEY_LEN };
	size_t puk_size_test_vector[] = { EC_CURVE25519_PUB_KEY_LEN, EC_SECP256R1_PUB_KEY_LEN,
					  EC_ED25519_PUB_KEY_LEN, EC_SECP256R1_PUB_KEY_LEN };

	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(private_key, 0x00, sizeof(private_key));
	memset(public_key, 0x00, sizeof(public_key));

	for (sid_pal_ecc_algo_t algo = SID_PAL_ECDH_CURVE25519; algo <= SID_PAL_ECDSA_SECP256R1;
	     algo++) {
		params.algo = algo;
		params.prk = private_key;
		params.prk_size = prk_size_test_vector[algo - 1];
		params.puk = public_key;
		params.puk_size = puk_size_test_vector[algo - 1];

		zassert_equal(SID_ERROR_NONE, sid_pal_crypto_ecc_key_gen(&params));
	}
}

ZTEST(crypto, ECDH_SECP256R1)
{
	/* Keys generated in the openssl */
	uint8_t private_secp256r1_a[EC_SECP256R1_PRIV_KEY_LEN] = {
		0xd0, 0xb2, 0x29, 0xdd, 0x13, 0x97, 0x62, 0xee, 0xca, 0x9a, 0xb9,
		0x28, 0x57, 0x82, 0xbf, 0x76, 0xd9, 0x5e, 0xf0, 0x7f, 0x5c, 0x84,
		0x5f, 0x51, 0x3f, 0x46, 0x19, 0x59, 0x91, 0x00, 0x10, 0xed
	};

	uint8_t public_secp256r1_b1[EC_SECP256R1_PUB_KEY_LEN] = {
		0x89, 0xed, 0x69, 0x79, 0xf6, 0x76, 0xd6, 0xe2, 0xe9, 0x4f, 0x5e, 0xef, 0x14,
		0x7e, 0x45, 0x1c, 0x6d, 0xec, 0x24, 0xdb, 0x75, 0xd2, 0xc5, 0x8c, 0x3c, 0xc1,
		0xa0, 0x2d, 0x8e, 0xb3, 0xda, 0x17, 0xf4, 0x65, 0xc8, 0x13, 0xf9, 0xf7, 0xc6,
		0x6a, 0xec, 0xc3, 0xde, 0x67, 0x5c, 0x51, 0xe0, 0x0a, 0x20, 0x96, 0xe1, 0x8a,
		0x72, 0x25, 0xc2, 0xc3, 0xc0, 0x72, 0x1b, 0x8c, 0xd0, 0xea, 0x1a, 0x28
	};

	uint8_t public_secp256r1_b2[EC_SECP256R1_PUB_KEY_LEN] = {
		0x09, 0x24, 0x02, 0x61, 0x43, 0x0d, 0x3f, 0xaa, 0x34, 0xca, 0xcc, 0x1c, 0xfa,
		0x9b, 0x17, 0xf5, 0x2d, 0xf8, 0x5b, 0x8b, 0x9a, 0x58, 0x8d, 0x98, 0x42, 0x73,
		0x45, 0x78, 0x2f, 0x03, 0xad, 0x9b, 0xe3, 0x2c, 0xc3, 0x71, 0x46, 0x41, 0x20,
		0xf4, 0x16, 0x0a, 0xa9, 0xe1, 0x0c, 0x3d, 0xa5, 0x83, 0x49, 0xe2, 0x40, 0x1a,
		0x17, 0x4d, 0xa7, 0xbc, 0xbf, 0x29, 0x3d, 0x3b, 0xe4, 0x86, 0xd4, 0x75
	};

	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	sid_pal_ecdh_params_t ecdh_params;

	/* Derive and verify secret with private key A and public key B1 */
	ecdh_params.algo = SID_PAL_ECDH_SECP256R1;
	ecdh_params.prk = private_secp256r1_a;
	ecdh_params.prk_size = EC_SECP256R1_PRIV_KEY_LEN;
	ecdh_params.puk = public_secp256r1_b1;
	ecdh_params.puk_size = EC_SECP256R1_PUB_KEY_LEN;
	ecdh_params.shared_secret = secret_result;
	ecdh_params.shared_secret_sz = ECDH_SECRET_SIZE;

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params));

	ecdh_params.puk = public_secp256r1_b2;
	ecdh_params.puk_size = EC_SECP256R1_PUB_KEY_LEN;

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params));
}

ZTEST(crypto, ECDH_CURVE25519)
{
	/* Keys generated in the openssl */
	uint8_t private_x25519_a[EC_CURVE25519_PRIV_KEY_LEN] = {
		0x28, 0x8e, 0x32, 0x0f, 0xc8, 0x74, 0x4e, 0x58, 0xd8, 0x49, 0x57,
		0x42, 0x72, 0xb6, 0xe2, 0x3c, 0x4a, 0x71, 0x36, 0xd4, 0x87, 0x28,
		0x34, 0x17, 0x5d, 0xc9, 0xf9, 0x5c, 0xfc, 0x45, 0x34, 0x65
	};

	uint8_t public_x25519_b[EC_CURVE25519_PUB_KEY_LEN] = {
		0x33, 0x01, 0x40, 0x0a, 0x7d, 0xb0, 0x3b, 0x29, 0xee, 0x5b, 0xc3,
		0x03, 0xbd, 0x85, 0x07, 0xe4, 0xbd, 0x3a, 0x11, 0x53, 0xce, 0x39,
		0xaf, 0x39, 0x10, 0x88, 0x64, 0x6a, 0xbb, 0xb5, 0x56, 0x71
	};

	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	sid_pal_ecdh_params_t ecdh_params;

	/* Derive and verify secret with private key A and public key B1 */
	ecdh_params.algo = SID_PAL_ECDH_CURVE25519;
	ecdh_params.prk = private_x25519_a;
	ecdh_params.prk_size = EC_CURVE25519_PRIV_KEY_LEN;
	ecdh_params.puk = public_x25519_b;
	ecdh_params.puk_size = EC_CURVE25519_PUB_KEY_LEN;
	ecdh_params.shared_secret = secret_result;
	ecdh_params.shared_secret_sz = ECDH_SECRET_SIZE;

	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params));
}

static void crypto_init(void *fixture)
{
	sid_pal_crypto_init();
}

static void crypto_deinit(void *fixture)
{
	sid_pal_crypto_deinit();
}

ZTEST_SUITE(crypto, NULL, NULL, crypto_init, crypto_deinit, NULL);
