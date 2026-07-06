/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <sid_crypto_keys.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_storage_kv_ifc.h>

#include <errno.h>
#include <string.h>

#define TEST_AES_KEY_SIZE 16
#define TEST_ED25519_KEY_SIZE 32
#define TEST_SECP256R1_KEY_SIZE 32
#define TEST_ED25519_PUBLIC_KEY_SIZE 32
#define TEST_SECP256R1_PUBLIC_KEY_SIZE 65

static psa_key_id_t test_key_id = SID_CRYPTO_KV_APP_KEY_KEY_ID;
static psa_key_id_t test_key_ecc_id = SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID;

struct test_key_spec {
	psa_key_id_t id;
	const uint8_t *key_data;
	size_t key_data_size;
	psa_key_type_t key_type;
	psa_algorithm_t alg;
	bool is_symmetric;
	size_t public_key_size;
};

static const uint8_t test_message[] = "Sidewalk crypto key test message";

static const uint8_t test_ed25519_key[TEST_ED25519_KEY_SIZE] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
	0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
	0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
};

static const uint8_t test_secp256r1_key[TEST_SECP256R1_KEY_SIZE] = {
	0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15,
	0x14, 0x13, 0x12, 0x11, 0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A,
	0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x01,
};

static const uint8_t test_wan_key[TEST_AES_KEY_SIZE] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5,
							 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB,
							 0xAC, 0xAD, 0xAE, 0xAF };

static const uint8_t test_app_key[TEST_AES_KEY_SIZE] = { 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5,
							 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
							 0xBC, 0xBD, 0xBE, 0xBF };

static const uint8_t test_d2d_key[TEST_AES_KEY_SIZE] = { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5,
							 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB,
							 0xCC, 0xCD, 0xCE, 0xCF };

static const struct test_key_spec import_key_specs[] = {
	{
		.id = SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID,
		.key_data = test_ed25519_key,
		.key_data_size = sizeof(test_ed25519_key),
		.key_type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_TWISTED_EDWARDS),
		.alg = PSA_ALG_PURE_EDDSA,
		.is_symmetric = false,
		.public_key_size = TEST_ED25519_PUBLIC_KEY_SIZE,
	},
	{
		.id = SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID,
		.key_data = test_secp256r1_key,
		.key_data_size = sizeof(test_secp256r1_key),
		.key_type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1),
		.alg = PSA_ALG_ECDSA(PSA_ALG_SHA_256),
		.is_symmetric = false,
		.public_key_size = TEST_SECP256R1_PUBLIC_KEY_SIZE,
	},
	{
		.id = SID_CRYPTO_KV_WAN_MASTER_KEY_ID,
		.key_data = test_wan_key,
		.key_data_size = sizeof(test_wan_key),
		.key_type = PSA_KEY_TYPE_AES,
		.alg = PSA_ALG_CMAC,
		.is_symmetric = true,
	},
	{
		.id = SID_CRYPTO_KV_APP_KEY_KEY_ID,
		.key_data = test_app_key,
		.key_data_size = sizeof(test_app_key),
		.key_type = PSA_KEY_TYPE_AES,
		.alg = PSA_ALG_CMAC,
		.is_symmetric = true,
	},
	{
		.id = SID_CRYPTO_KV_D2D_KEY_ID,
		.key_data = test_d2d_key,
		.key_data_size = sizeof(test_d2d_key),
		.key_type = PSA_KEY_TYPE_AES,
		.alg = PSA_ALG_CMAC,
		.is_symmetric = true,
	},
};

static const struct test_key_spec *const generate_key_specs[] = {
	&import_key_specs[0],
	&import_key_specs[1],
};

static psa_key_id_t import_plain_key(psa_algorithm_t alg, psa_key_type_t key_type,
				     psa_key_usage_t usage_flags, const uint8_t *key_data,
				     size_t key_data_size)
{
	psa_key_id_t key_id = PSA_KEY_ID_NULL;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_status_t status;

	psa_set_key_usage_flags(&attributes, usage_flags);
	psa_set_key_algorithm(&attributes, alg);
	psa_set_key_type(&attributes, key_type);

	status = psa_import_key(&attributes, key_data, key_data_size, &key_id);
	psa_reset_key_attributes(&attributes);

	zassert_equal(PSA_SUCCESS, status, "psa_import_key err: %d", status);
	zassert_not_equal(PSA_KEY_ID_NULL, key_id);

	return key_id;
}

static psa_key_id_t import_plain_verification_key(const struct test_key_spec *spec)
{
	return import_plain_key(spec->alg, spec->key_type, PSA_KEY_USAGE_VERIFY_MESSAGE,
				spec->key_data, spec->key_data_size);
}

static psa_key_id_t import_plain_public_key(const struct test_key_spec *spec,
					    const uint8_t *public_key, size_t public_key_size)
{
	const psa_key_type_t public_key_type = PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(spec->key_type);

	return import_plain_key(spec->alg, public_key_type, PSA_KEY_USAGE_VERIFY_MESSAGE,
				public_key, public_key_size);
}

static void assert_keys_equal(psa_key_id_t signing_key, psa_key_id_t verification_key,
			      const struct test_key_spec *spec)
{
	uint8_t signature[PSA_SIGNATURE_MAX_SIZE] = { 0 };
	size_t signature_size = 0;
	psa_status_t status;

	if (spec->is_symmetric) {
		status = psa_mac_compute(signing_key, spec->alg, test_message, sizeof(test_message),
					 signature, sizeof(signature), &signature_size);
		zassert_equal(PSA_SUCCESS, status, "psa_mac_compute err: %d", status);

		status = psa_mac_verify(verification_key, spec->alg, test_message,
					sizeof(test_message), signature, signature_size);
		zassert_equal(PSA_SUCCESS, status, "psa_mac_verify err: %d", status);
		return;
	}

	status = psa_sign_message(signing_key, spec->alg, test_message, sizeof(test_message),
				  signature, sizeof(signature), &signature_size);
	zassert_equal(PSA_SUCCESS, status, "psa_sign_message err: %d", status);

	status = psa_verify_message(verification_key, spec->alg, test_message, sizeof(test_message),
				    signature, signature_size);
	zassert_equal(PSA_SUCCESS, status, "psa_verify_message err: %d", status);
}

static void *setup(void)
{
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_init());
	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_init());

	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);
	sid_pal_crypto_deinit();
}

static void before(void *arg)
{
	ARG_UNUSED(arg);
	int err = sid_crypto_keys_init();
	zassert_equal(0, err, "err: %d", err);
}

static void after(void *arg)
{
	ARG_UNUSED(arg);
	int err = sid_crypto_keys_deinit();
	zassert_equal(0, err, "err: %d", err);
}

ZTEST(crypto_keys, test_sid_crypto_key_invalid_args)
{
	int err = -ENOEXEC;
	uint8_t key_data[TEST_AES_KEY_SIZE] = { 0 };
	uint8_t ecc_key_data[TEST_ED25519_PUBLIC_KEY_SIZE] = { 0 };

	/* Invalid data */
	psa_key_id_t new_key_id = PSA_KEY_ID_NULL;
	err = sid_crypto_keys_buffer_get(&new_key_id, NULL, TEST_AES_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_buffer_get(&new_key_id, key_data, 0);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_buffer_set(test_key_id, NULL, TEST_AES_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_buffer_set(test_key_id, key_data, 0);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_import(test_key_id, NULL, TEST_AES_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_import(test_key_id, key_data, 0);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_generate(test_key_ecc_id, NULL, TEST_AES_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_generate(test_key_ecc_id, key_data, 0);
	zassert_equal(-EINVAL, err, "err: %d", err);

	/* Invalid key id */
	err = sid_crypto_keys_buffer_get(NULL, key_data, TEST_AES_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_buffer_set(PSA_KEY_ID_NULL, key_data, TEST_AES_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_import(PSA_KEY_ID_NULL, key_data, TEST_AES_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_generate(PSA_KEY_ID_NULL, ecc_key_data,
					   TEST_ED25519_PUBLIC_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);
}

ZTEST(crypto_keys, test_sid_crypto_key_buffer)
{
	uint8_t test_key_data[TEST_AES_KEY_SIZE] = {
		0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
		0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
	};
	psa_key_id_t new_key_id = PSA_KEY_ID_NULL;
	int err = -ENOEXEC;

	err = sid_crypto_keys_new_import(test_key_id, test_key_data, sizeof(test_key_data));
	zassert_equal(0, err, "err: %d", err);

	err = sid_crypto_keys_buffer_set(test_key_id, test_key_data, sizeof(test_key_data));
	zassert_equal(0, err, "err: %d", err);

	err = sid_crypto_keys_buffer_get(&new_key_id, test_key_data, sizeof(test_key_data));
	zassert_equal(0, err, "err: %d", err);

	zassert_equal(new_key_id, test_key_id);

	err = sid_crypto_keys_delete(test_key_id);
	zassert_equal(0, err, "err: %d", err);
}

ZTEST(crypto_keys, test_sid_crypto_key_import)
{
	for (size_t i = 0; i < ARRAY_SIZE(import_key_specs); i++) {
		const struct test_key_spec *spec = &import_key_specs[i];
		psa_key_id_t verification_key;
		int err;

		err = sid_crypto_keys_new_import(spec->id, (uint8_t *)spec->key_data,
						 spec->key_data_size);
		zassert_equal(0, err, "err: %d id: %d", err, spec->id);

		verification_key = import_plain_verification_key(spec);
		assert_keys_equal(spec->id, verification_key, spec);

		(void)psa_destroy_key(verification_key);

		err = sid_crypto_keys_delete(spec->id);
		zassert_equal(0, err, "err: %d id: %d", err, spec->id);
	}
}

ZTEST(crypto_keys, test_sid_crypto_key_generate)
{
	for (size_t i = 0; i < ARRAY_SIZE(generate_key_specs); i++) {
		const struct test_key_spec *spec = generate_key_specs[i];
		uint8_t public_key[PSA_EXPORT_PUBLIC_KEY_MAX_SIZE] = { 0 };
		size_t public_key_size = spec->public_key_size;
		size_t public_key_offset = 0;
		psa_key_id_t verification_key;
		int err;

		if (spec->id == SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID) {
			// sid_crypto_keys_new_generate exports the public key without the prefix for SECP256R1
			public_key[public_key_offset++] = 0x04;
		}

		err = sid_crypto_keys_new_generate(spec->id, public_key + public_key_offset,
						   public_key_size - public_key_offset);
		zassert_equal(0, err, "err: %d id: %d", err, spec->id);

		verification_key = import_plain_public_key(spec, public_key, public_key_size);
		assert_keys_equal(spec->id, verification_key, spec);

		(void)psa_destroy_key(verification_key);

		err = sid_crypto_keys_delete(spec->id);
		zassert_equal(0, err, "err: %d id: %d", err, spec->id);
	}
}

ZTEST_SUITE(crypto_keys, NULL, setup, before, after, teardown);
