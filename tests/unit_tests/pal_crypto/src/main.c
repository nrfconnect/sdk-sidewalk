/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sid_pal_crypto_ifc.h>
#include <zephyr/sys/util.h>

#include <zephyr/fff.h>
#include <psa/crypto.h>
#include <psa/crypto_types.h>
#include <psa/crypto_values.h>

DEFINE_FFF_GLOBALS;

/*************************************************************************
* Create fake functions for tests.
* ***********************************************************************/
FAKE_VALUE_FUNC(psa_status_t, psa_crypto_init);
FAKE_VALUE_FUNC(psa_status_t, psa_generate_random, uint8_t *, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_hash_compute, psa_algorithm_t, const uint8_t *, size_t, uint8_t *,
		size_t, size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_import_key, const psa_key_attributes_t *, const uint8_t *, size_t,
		mbedtls_svc_key_id_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_destroy_key, mbedtls_svc_key_id_t);
FAKE_VOID_FUNC(psa_reset_key_attributes, psa_key_attributes_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_mac_sign_setup, psa_mac_operation_t *, mbedtls_svc_key_id_t,
		psa_algorithm_t);
FAKE_VALUE_FUNC(psa_status_t, psa_mac_update, psa_mac_operation_t *, const uint8_t *, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_mac_sign_finish, psa_mac_operation_t *, uint8_t *, size_t,
		size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_set_key_domain_parameters, psa_key_attributes_t *, psa_key_type_t,
		const uint8_t *, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_mac_compute, mbedtls_svc_key_id_t, psa_algorithm_t,
		const uint8_t *, size_t, uint8_t *, size_t, size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_cipher_encrypt_setup, psa_cipher_operation_t *,
		mbedtls_svc_key_id_t, psa_algorithm_t);
FAKE_VALUE_FUNC(psa_status_t, psa_cipher_decrypt_setup, psa_cipher_operation_t *,
		mbedtls_svc_key_id_t, psa_algorithm_t);
FAKE_VALUE_FUNC(psa_status_t, psa_cipher_abort, psa_cipher_operation_t *)
FAKE_VALUE_FUNC(psa_status_t, psa_cipher_set_iv, psa_cipher_operation_t *, const uint8_t *, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_cipher_update, psa_cipher_operation_t *, const uint8_t *, size_t,
		uint8_t *, size_t, size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_cipher_finish, psa_cipher_operation_t *, uint8_t *, size_t,
		size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_verify_message, psa_key_handle_t, psa_algorithm_t,
		const uint8_t *, size_t, const uint8_t *, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_sign_message, psa_key_handle_t, psa_algorithm_t, const uint8_t *,
		size_t, uint8_t *, size_t, size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_generate_key, const psa_key_attributes_t *,
		mbedtls_svc_key_id_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_export_key, mbedtls_svc_key_id_t, uint8_t *, size_t, size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_export_public_key, mbedtls_svc_key_id_t, uint8_t *, size_t,
		size_t *);

FAKE_VALUE_FUNC(psa_status_t, psa_aead_set_lengths, psa_aead_operation_t *, size_t, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_aead_set_nonce, psa_aead_operation_t *, const uint8_t *, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_aead_update_ad, psa_aead_operation_t *, const uint8_t *, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_aead_update, psa_aead_operation_t *, const uint8_t *, size_t,
		uint8_t *, size_t, size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_aead_finish, psa_aead_operation_t *, uint8_t *, size_t, size_t *,
		uint8_t *, size_t, size_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_aead_verify, psa_aead_operation_t *, uint8_t *, size_t, size_t *,
		const uint8_t *, size_t);
FAKE_VALUE_FUNC(psa_status_t, psa_aead_encrypt_setup, psa_aead_operation_t *, mbedtls_svc_key_id_t,
		psa_algorithm_t);
FAKE_VALUE_FUNC(psa_status_t, psa_aead_decrypt_setup, psa_aead_operation_t *, mbedtls_svc_key_id_t,
		psa_algorithm_t);
FAKE_VALUE_FUNC(psa_status_t, psa_aead_abort, psa_aead_operation_t *);
FAKE_VALUE_FUNC(psa_status_t, psa_raw_key_agreement, psa_algorithm_t, mbedtls_svc_key_id_t,
		const uint8_t *, size_t, uint8_t *, size_t, size_t *);
/*************************************************************************
* Create fake functions for tests end.
* ***********************************************************************/

#define FFF_FAKES_LIST(FAKE)                                                                       \
	FAKE(psa_crypto_init)                                                                      \
	FAKE(psa_generate_random)                                                                  \
	FAKE(psa_hash_compute)                                                                     \
	FAKE(psa_import_key)                                                                       \
	FAKE(psa_destroy_key)                                                                      \
	FAKE(psa_reset_key_attributes)                                                             \
	FAKE(psa_mac_sign_setup)                                                                   \
	FAKE(psa_mac_update)                                                                       \
	FAKE(psa_mac_sign_finish)                                                                  \
	FAKE(psa_mac_compute)                                                                      \
	FAKE(psa_cipher_abort)                                                                     \
	FAKE(psa_cipher_encrypt_setup)                                                             \
	FAKE(psa_cipher_decrypt_setup)                                                             \
	FAKE(psa_cipher_set_iv)                                                                    \
	FAKE(psa_cipher_update)                                                                    \
	FAKE(psa_cipher_finish)                                                                    \
	FAKE(psa_verify_message)                                                                   \
	FAKE(psa_sign_message)                                                                     \
	FAKE(psa_generate_key)                                                                     \
	FAKE(psa_export_key)                                                                       \
	FAKE(psa_export_public_key)                                                                \
	FAKE(psa_aead_decrypt_setup)                                                               \
	FAKE(psa_aead_encrypt_setup)                                                               \
	FAKE(psa_aead_verify)                                                                      \
	FAKE(psa_aead_finish)                                                                      \
	FAKE(psa_aead_update)                                                                      \
	FAKE(psa_aead_update_ad)                                                                   \
	FAKE(psa_aead_set_nonce)                                                                   \
	FAKE(psa_aead_set_lengths)                                                                 \
	FAKE(psa_aead_abort)                                                                       \
	FAKE(psa_raw_key_agreement)

#define RNG_BUFF_MAX_SIZE (128)

#define SHA256_LEN (32)
#define SHA512_LEN (64)
#define SHA_MAX_DIGEST_LEN (SHA512_LEN)

#define HMAC_MAX_BLOCK_SIZE (32)

#define HASH_TEST_DATA_BLOCK_SIZE (128)
#define AES_MAX_BLOCK_SIZE (16)
#define AES_TEST_DATA_BLOCK_SIZE (128)
#define HMAC_TEST_DATA_BLOCK_SIZE (128)

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

#define ALGO_DATA_CHUNK (32)

/**
 * @brief create pointer to variable
 *
 */
#define ALIAS(variable, alias_name) __typeof__(&variable) alias_name = &variable

typedef psa_status_t (*custom_psa_aead_update_t)(psa_aead_operation_t *, const uint8_t *, size_t,
						 uint8_t *, size_t, size_t *);
struct mock_psa_aead_update_values_t {
	uint32_t call_count;
	struct {
		psa_aead_operation_t *in_operation;
		const uint8_t *in_input;
		size_t in_input_length;
		uint8_t *in_output;
		size_t in_output_size;
		size_t *in_output_length;

		psa_status_t out_ret_val;
		uint8_t *out_output;
		size_t out_output_size;
		size_t out_output_length;
	} parameters[FFF_ARG_HISTORY_LEN];
};
static struct mock_psa_aead_update_values_t mock_psa_aead_update_values;

static psa_status_t mock_psa_aead_update(psa_aead_operation_t *operation, const uint8_t *input,
					 size_t input_length, uint8_t *output, size_t output_size,
					 size_t *output_length)
{
	if (FFF_ARG_HISTORY_LEN <= mock_psa_aead_update_values.call_count) {
		TEST_FAIL_MESSAGE("Function psa_aead_update called too many times!");
	}
	ALIAS(mock_psa_aead_update_values.parameters[mock_psa_aead_update_values.call_count],
	      call_parameters);
	mock_psa_aead_update_values.call_count++;

	call_parameters->in_operation = operation;
	call_parameters->in_input = input;
	call_parameters->in_input_length = input_length;
	call_parameters->in_output = output;
	call_parameters->in_output_size = output_size;
	call_parameters->in_output_length = output_length;

	if (output && call_parameters->out_output && call_parameters->out_output_size) {
		memcpy(output, call_parameters->out_output, call_parameters->out_output_size);
		*output = *call_parameters->out_output;
	}

	if (output_length) {
		*output_length = call_parameters->out_output_length;
	}

	return call_parameters->out_ret_val;
}

struct mock_psa_aead_update_output {
	uint8_t *output;
	size_t output_size;
};

#define MOCK_PSA_AEAD_UPDATE_SET_RETURN_internal(id, value)                                        \
	mock_psa_aead_update_values.parameters[id].out_ret_val = value
#define MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH_internal(id, value)                             \
	mock_psa_aead_update_values.parameters[id].out_output_length = value
#define MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_internal(int, value)                                   \
	mock_psa_aead_update_values.parameters[id].out_output = value.output;                      \
	mock_psa_aead_update_values.parameters[id].out_output_size = output_size

#define MOCK_PSA_AEAD_UPDATE_SET_RETURN(...)                                                       \
	FOR_EACH_IDX(MOCK_PSA_AEAD_UPDATE_SET_RETURN_internal, (;), __VA_ARGS__)
#define MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH(...)                                            \
	FOR_EACH_IDX(MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH_internal, (;), __VA_ARGS__)
#define MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT(...)                                                   \
	FOR_EACH_IDX(MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_internal, (;), __VA_ARGS__)

/*************************************************************************
* setUp & tearDown
* ***********************************************************************/
void setUp(void)
{
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}

void tearDown(void)
{
}

/*************************************************************************
* INIT & DEINIT
* ***********************************************************************/
void test_sid_pal_crypto_no_init_function(void)
{
	// Initialize crypto module
	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	psa_crypto_init_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());
}

void test_sid_pal_crypto_no_init_modules(void)
{
	// For any arguments functions shall return the same error code
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_deinit());
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_key_gen(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_ecdh(NULL));

	psa_crypto_init_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_key_gen(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_ecdh(NULL));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// And now we expect that error code will be different like SID_ERROR_UNINITIALIZED
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_key_gen(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_ecdh(NULL));
}

void test_sid_pal_crypto_deinit(void)
{
	// And now we expect that error code will be different like SID_ERROR_UNINITIALIZED
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_deinit());
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_key_gen(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_ecdh(NULL));
}

/*************************************************************************
* END INIT & DEINIT
* ***********************************************************************/

/*************************************************************************
* RNG
* ***********************************************************************/
void test_sid_pal_crypto_rng_null_ptr(void)
{
	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_rand(NULL, 5));
}

void test_sid_pal_crypto_rng_pass(void)
{
	uint8_t rng_buff[RNG_BUFF_MAX_SIZE];

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	psa_generate_random_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_rand(rng_buff, 5));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_rand(rng_buff, 128));
}

void test_sid_pal_crypto_rng_error(void)
{
	uint8_t rng_buff[RNG_BUFF_MAX_SIZE];

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	psa_generate_random_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, sid_pal_crypto_rand(rng_buff, 16));
}

void test_sid_pal_crypto_rng_invalid_args(void)
{
	uint8_t rng_buff[RNG_BUFF_MAX_SIZE];

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_rand(rng_buff, 0));
}

/*************************************************************************
* END RNG
* ***********************************************************************/

/*************************************************************************
* HASH
* ***********************************************************************/
void test_sid_pal_crypto_hash_null_ptr(void)
{
	sid_pal_hash_params_t params;
	uint8_t data[HASH_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hash(&params));

	params.data = data;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hash(&params));

	params.data = NULL;
	params.digest = digest;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hash(&params));
}

void test_sid_pal_crypto_hash_invalid_args(void)
{
	sid_pal_hash_params_t params;
	uint8_t data[HASH_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_HASH_SHA256;
	params.data = data;
	params.digest = digest;
	params.digest_size = SHA256_LEN;

	// Incorrect size
	params.data_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hash(&params));

	params.data_size = sizeof(data);
	params.digest_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hash(&params));
}

void test_sid_pal_crypto_hash_incorrect_algo(void)
{
	sid_pal_hash_params_t params;
	uint8_t data[HASH_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.data = data;
	params.digest = digest;
	params.digest_size = SHA256_LEN;
	params.data_size = sizeof(data);

	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_hash(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_hash(&params));
}

void test_sid_pal_crypto_hash_pass(void)
{
	sid_pal_hash_params_t params;
	uint8_t data[HASH_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_HASH_SHA256;
	params.data = data;
	params.digest = digest;
	params.digest_size = SHA256_LEN;
	params.data_size = sizeof(data);

	psa_hash_compute_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hash(&params));

	params.algo = SID_PAL_HASH_SHA512;
	psa_hash_compute_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hash(&params));

	// Check errors as well.
	psa_hash_compute_fake.return_val = PSA_ERROR_NOT_SUPPORTED;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_hash(&params));

	psa_hash_compute_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, sid_pal_crypto_hash(&params));
}

/*************************************************************************
* END HASH
* ***********************************************************************/

/*************************************************************************
* HMAC
* ***********************************************************************/
void test_sid_pal_crypto_hmac_null_ptr(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];
	uint8_t hmac_test_key[HMAC_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(&params));

	params.algo = SID_PAL_HASH_SHA512;

	params.key = NULL;
	params.data = data;
	params.digest = digest;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(&params));

	params.key = hmac_test_key;
	params.data = NULL;
	params.digest = digest;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(&params));

	params.key = hmac_test_key;
	params.data = data;
	params.digest = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(&params));

	params.key = hmac_test_key;
	params.data = NULL;
	params.digest = digest;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(&params));

	params.key = NULL;
	params.data = NULL;
	params.digest = digest;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(&params));

	params.key = hmac_test_key;
	params.data = NULL;
	params.digest = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(&params));

	params.key = NULL;
	params.data = data;
	params.digest = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hmac(&params));
}

void test_sid_pal_crypto_hmac_invalid_args(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];
	uint8_t hmac_test_key[HMAC_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_HASH_SHA256;
	params.key = hmac_test_key;
	params.digest = digest;
	params.data = data;

	params.key_size = 0;
	params.data_size = sizeof(data);
	params.digest_size = sizeof(digest);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hmac(&params));

	params.key_size = sizeof(hmac_test_key);
	params.data_size = 0;
	params.digest_size = sizeof(digest);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hmac(&params));

	params.key_size = sizeof(hmac_test_key);
	params.data_size = sizeof(data);
	params.digest_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hmac(&params));

	params.key_size = 0;
	params.data_size = sizeof(data);
	params.digest_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hmac(&params));

	params.key_size = 0;
	params.data_size = 0;
	params.digest_size = sizeof(digest);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hmac(&params));

	params.key_size = sizeof(hmac_test_key);
	params.data_size = 0;
	params.digest_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hmac(&params));
}

void test_sid_pal_crypto_hmac_incorrect_algo(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];
	uint8_t hmac_test_key[HMAC_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data;
	params.data_size = sizeof(data);
	params.digest = digest;
	params.digest_size = sizeof(digest);

	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_hmac(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_hmac(&params));
}

void test_sid_pal_crypto_hmac_false_positives(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];
	uint8_t hmac_test_key[HMAC_MAX_BLOCK_SIZE];
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED, PSA_SUCCESS, PSA_SUCCESS,
					      PSA_SUCCESS };
	psa_status_t psa_mac_sign_setup_ret[] = { PSA_ERROR_NOT_SUPPORTED, PSA_SUCCESS, PSA_SUCCESS,
						  PSA_SUCCESS };
	psa_status_t psa_mac_update_ret[] = { PSA_ERROR_BAD_STATE, PSA_SUCCESS, PSA_SUCCESS,
					      PSA_SUCCESS };

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_mac_sign_setup, psa_mac_sign_setup_ret, sizeof(psa_mac_sign_setup_ret));
	SET_RETURN_SEQ(psa_mac_update, psa_mac_update_ret, sizeof(psa_mac_update_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_HASH_SHA256;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data;
	params.data_size = sizeof(data) / 2;
	params.digest = digest;
	params.digest_size = sizeof(digest);

	psa_mac_sign_finish_fake.return_val = PSA_ERROR_BUFFER_TOO_SMALL;
	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_hmac(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_hmac(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_hmac(&params));

	psa_destroy_key_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_hmac(&params));
}

void test_sid_pal_crypto_hmac_pass(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];
	uint8_t hmac_test_key[HMAC_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_HASH_SHA256;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data;
	params.data_size = sizeof(data);
	params.digest = digest;
	params.digest_size = sizeof(digest);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_mac_sign_setup_fake.return_val = PSA_SUCCESS;
	psa_mac_update_fake.return_val = PSA_SUCCESS;
	psa_mac_sign_finish_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));

	params.algo = SID_PAL_HASH_SHA512;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
}

void test_sid_pal_crypto_hmac_variable_data_len(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];
	uint8_t hmac_test_key[HMAC_MAX_BLOCK_SIZE];
	size_t test_vector[] = { 5, 16, 32, 48, 64, 122, 128, 256, 511 };

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_HASH_SHA256;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data;
	params.digest = digest;
	params.digest_size = sizeof(digest);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_mac_sign_setup_fake.return_val = PSA_SUCCESS;
	psa_mac_update_fake.return_val = PSA_SUCCESS;
	psa_mac_sign_finish_fake.return_val = PSA_SUCCESS;

	for (int it = 0; it < (int)(sizeof(test_vector) / sizeof(test_vector[0])); it++) {
		params.data_size = test_vector[it];
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
		TEST_ASSERT_EQUAL(
			ceilf((double)((double)params.data_size / (double)ALGO_DATA_CHUNK)),
			psa_mac_update_fake.call_count);
		RESET_FAKE(psa_mac_update);
	}
}

/*************************************************************************
* END HMAC
* ***********************************************************************/

/*************************************************************************
* AES & CMAC
* ***********************************************************************/
void test_sid_pal_crypto_aes_null_ptr(void)
{
	sid_pal_aes_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = NULL;
	params.out = encrypted_data;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = NULL;
	params.out = NULL;
	params.key = aes_128_test_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = NULL;
	params.out = encrypted_data;
	params.key = aes_128_test_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data;
	params.out = NULL;
	params.key = aes_128_test_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data;
	params.out = encrypted_data;
	params.key = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data;
	params.out = encrypted_data;
	params.key = aes_128_test_key;
	params.algo = SID_PAL_AES_CTR_128;
	params.iv = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = NULL;
	params.out = encrypted_data;
	params.key = aes_128_test_key;

	params.algo = SID_PAL_AES_CTR_128;
	params.iv = iv;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data;
	params.out = NULL;
	params.key = aes_128_test_key;

	params.algo = SID_PAL_AES_CTR_128;
	params.iv = iv;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data;
	params.out = encrypted_data;
	params.key = NULL;

	params.algo = SID_PAL_AES_CTR_128;
	params.iv = iv;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data;
	params.out = encrypted_data;
	params.key = aes_128_test_key;
	params.algo = SID_PAL_AES_CMAC_128;
	params.iv = NULL;
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));
}

void test_sid_pal_crypto_aes_invalid_args(void)
{
	sid_pal_aes_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.in = data;
	params.out = encrypted_data;
	params.key = aes_128_test_key;
	params.algo = SID_PAL_AES_CTR_128;
	params.iv = iv;

	params.iv_size = 0;
	params.key_size = 0;
	params.in_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.iv_size = sizeof(iv);
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.in_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.in_size = sizeof(data);
	params.iv_size = 0;
	params.key_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.iv_size = sizeof(iv);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.key_size = 3;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.key_size = 3 * (sizeof(aes_128_test_key) * 8);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.algo = SID_PAL_AES_CMAC_128;
	params.key_size = 3;
	params.iv_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.iv_size = sizeof(iv);
	params.key_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.iv_size = sizeof(iv);
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.in_size = sizeof(data);
	params.mode = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));
}

void test_sid_pal_crypto_aes_incorrect_algo(void)
{
	sid_pal_aes_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_MAC_CALCULATE;
	params.in = data;
	params.out = encrypted_data;
	params.key = aes_128_test_key;
	params.iv = iv;

	params.iv_size = sizeof(iv);
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.in_size = sizeof(data);

	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_aes_crypt(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_aes_crypt(&params));
}

void test_sid_pal_crypto_aes_cmac_pass(void)
{
	sid_pal_aes_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t mac[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_MAC_CALCULATE;
	params.algo = SID_PAL_AES_CMAC_128;
	params.in = data;
	params.in_size = sizeof(data);
	params.out = mac;
	params.out_size = sizeof(mac);
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_mac_compute_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));
}

void test_sid_pal_crypto_aes_cmac_false_positives(void)
{
	sid_pal_aes_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED, PSA_SUCCESS, PSA_SUCCESS };
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t mac[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_MAC_CALCULATE;
	params.algo = SID_PAL_AES_CMAC_128;
	params.in = data;
	params.in_size = sizeof(data);
	params.out = mac;
	params.out_size = 0;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);

	psa_mac_compute_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_aes_crypt(&params));

	psa_mac_compute_fake.return_val = PSA_ERROR_BUFFER_TOO_SMALL;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_aes_crypt(&params));

	psa_destroy_key_fake.return_val = PSA_ERROR_BAD_STATE;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_aes_crypt(&params));
}

void test_sid_pal_crypto_aes_encrypt_pass(void)
{
	sid_pal_aes_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.algo = SID_PAL_AES_CTR_128;
	params.in = data;
	params.in_size = sizeof(data);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);

	psa_import_key_fake.return_val = PSA_SUCCESS;

	psa_cipher_encrypt_setup_fake.return_val = PSA_SUCCESS;
	psa_cipher_set_iv_fake.return_val = PSA_SUCCESS;
	psa_cipher_update_fake.return_val = PSA_SUCCESS;
	psa_cipher_finish_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));
}

void test_sid_pal_crypto_aes_encrypt_false_positives(void)
{
	sid_pal_aes_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS };
	psa_status_t psa_cipher_encrypt_setup_ret[] = { PSA_ERROR_INVALID_ARGUMENT,
							PSA_SUCCESS,
							PSA_SUCCESS,
							PSA_SUCCESS,
							PSA_SUCCESS,
							PSA_SUCCESS };
	psa_status_t psa_cipher_set_iv_ret[] = { PSA_ERROR_INSUFFICIENT_MEMORY,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS };
	psa_status_t psa_cipher_update_ret[] = { PSA_ERROR_INSUFFICIENT_MEMORY,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS };
	psa_status_t psa_cipher_finish_ret[] = { PSA_ERROR_BAD_STATE, PSA_ERROR_BAD_STATE,
						 PSA_SUCCESS,	      PSA_SUCCESS,
						 PSA_SUCCESS,	      PSA_SUCCESS };
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_cipher_encrypt_setup, psa_cipher_encrypt_setup_ret,
		       sizeof(psa_cipher_encrypt_setup_ret));
	SET_RETURN_SEQ(psa_cipher_set_iv, psa_cipher_set_iv_ret, sizeof(psa_cipher_set_iv_ret));
	SET_RETURN_SEQ(psa_cipher_update, psa_cipher_update_ret, sizeof(psa_cipher_update_ret));
	SET_RETURN_SEQ(psa_cipher_finish, psa_cipher_finish_ret, sizeof(psa_cipher_finish_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.algo = SID_PAL_AES_CTR_128;
	params.in = data;
	params.in_size = sizeof(data);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);

	psa_cipher_abort_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_aes_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_OOM, sid_pal_crypto_aes_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_OOM, sid_pal_crypto_aes_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aes_crypt(&params));

	psa_cipher_abort_fake.return_val = PSA_ERROR_BAD_STATE;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aes_crypt(&params));
}

void test_sid_pal_crypto_aes_decrypt_pass(void)
{
	sid_pal_aes_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.algo = SID_PAL_AES_CTR_128;
	params.in = data;
	params.in_size = sizeof(data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);

	psa_import_key_fake.return_val = PSA_SUCCESS;

	psa_cipher_decrypt_setup_fake.return_val = PSA_SUCCESS;
	psa_cipher_set_iv_fake.return_val = PSA_SUCCESS;
	psa_cipher_update_fake.return_val = PSA_SUCCESS;
	psa_cipher_finish_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));
}

void test_sid_pal_crypto_aes_decrypt_false_positives(void)
{
	sid_pal_aes_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS };
	psa_status_t psa_cipher_decrypt_setup_ret[] = { PSA_ERROR_INVALID_ARGUMENT,
							PSA_SUCCESS,
							PSA_SUCCESS,
							PSA_SUCCESS,
							PSA_SUCCESS,
							PSA_SUCCESS };
	psa_status_t psa_cipher_set_iv_ret[] = { PSA_ERROR_INSUFFICIENT_MEMORY,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS };
	psa_status_t psa_cipher_update_ret[] = { PSA_ERROR_INSUFFICIENT_MEMORY,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS,
						 PSA_SUCCESS };
	psa_status_t psa_cipher_finish_ret[] = { PSA_ERROR_BAD_STATE, PSA_ERROR_BAD_STATE,
						 PSA_SUCCESS,	      PSA_SUCCESS,
						 PSA_SUCCESS,	      PSA_SUCCESS };
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_cipher_decrypt_setup, psa_cipher_decrypt_setup_ret,
		       sizeof(psa_cipher_decrypt_setup_ret));
	SET_RETURN_SEQ(psa_cipher_set_iv, psa_cipher_set_iv_ret, sizeof(psa_cipher_set_iv_ret));
	SET_RETURN_SEQ(psa_cipher_update, psa_cipher_update_ret, sizeof(psa_cipher_update_ret));
	SET_RETURN_SEQ(psa_cipher_finish, psa_cipher_finish_ret, sizeof(psa_cipher_finish_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.algo = SID_PAL_AES_CTR_128;
	params.in = data;
	params.in_size = sizeof(data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);

	psa_cipher_abort_fake.return_val = PSA_SUCCESS;

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_aes_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_OOM, sid_pal_crypto_aes_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_OOM, sid_pal_crypto_aes_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aes_crypt(&params));

	psa_cipher_abort_fake.return_val = PSA_ERROR_BAD_STATE;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aes_crypt(&params));
}

/*************************************************************************
* END AES & CMAC
* ***********************************************************************/

/*************************************************************************
* ECDSA
* ***********************************************************************/
void test_sid_pal_crypto_ecc_dsa_null_ptr(void)
{
	sid_pal_dsa_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = NULL;
	params.in = data;
	params.signature = signature;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = public_SECP256R1;
	params.in = NULL;
	params.signature = signature;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = public_SECP256R1;
	params.in = data;
	params.signature = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = NULL;
	params.in = NULL;
	params.signature = signature;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = NULL;
	params.in = data;
	params.signature = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = public_SECP256R1;
	params.in = NULL;
	params.signature = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_invalid_args(void)
{
	sid_pal_dsa_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_SECP256R1;
	params.key_size = sizeof(public_SECP256R1);
	params.in = data;
	params.signature = signature;
	params.sig_size = sizeof(signature);

	params.in_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_dsa(&params));

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.key = public_SECP256R1;
	params.key_size = sizeof(public_SECP256R1);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	params.mode = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_incorrect_algo(void)
{
	sid_pal_dsa_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_SECP256R1;
	params.key_size = sizeof(public_SECP256R1);
	params.in = data;
	params.signature = signature;
	params.sig_size = sizeof(signature);
	params.in_size = sizeof(data);

	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_dsa(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_EDDSA_verify_pass(void)
{
	sid_pal_dsa_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t public_ED[EC_ED25519_PUB_KEY_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_EDDSA_ED25519;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_ED;
	params.key_size = sizeof(public_ED);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_verify_message_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_EDDSA_sign_pass(void)
{
	sid_pal_dsa_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t private_ED[EC_ED25519_PRIV_KEY_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_EDDSA_ED25519;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = private_ED;
	params.key_size = sizeof(private_ED);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_verify_message_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_EDDSA_verify_false_positives(void)
{
	sid_pal_dsa_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED, PSA_SUCCESS };
	psa_status_t psa_verify_message_ret[] = { PSA_ERROR_DATA_INVALID, PSA_ERROR_DATA_INVALID };
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t public_ED[EC_ED25519_PUB_KEY_LEN];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_verify_message, psa_verify_message_ret, sizeof(psa_verify_message_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_EDDSA_ED25519;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_ED;
	params.key_size = sizeof(public_ED);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_ecc_dsa(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_EDDSA_sign_false_positives(void)
{
	sid_pal_dsa_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED, PSA_SUCCESS };
	psa_status_t psa_sign_message_ret[] = { PSA_ERROR_DATA_INVALID, PSA_ERROR_DATA_INVALID };
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t private_ED[EC_ED25519_PRIV_KEY_LEN];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_sign_message, psa_sign_message_ret, sizeof(psa_sign_message_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_EDDSA_ED25519;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = private_ED;
	params.key_size = sizeof(private_ED);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_ecc_dsa(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_SECP256R1_sign_pass(void)
{
	sid_pal_dsa_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t private_SECP256R1[EC_SECP256R1_PRIV_KEY_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = private_SECP256R1;
	params.key_size = sizeof(private_SECP256R1);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_verify_message_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_SECP256R1_sign_false_positives(void)
{
	sid_pal_dsa_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED, PSA_SUCCESS };
	psa_status_t psa_sign_message_ret[] = { PSA_ERROR_DATA_INVALID, PSA_ERROR_DATA_INVALID };
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t private_SECP256R1[EC_SECP256R1_PRIV_KEY_LEN];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_sign_message, psa_sign_message_ret, sizeof(psa_sign_message_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key = private_SECP256R1;
	params.key_size = sizeof(private_SECP256R1);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_ecc_dsa(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_SECP256R1_verify_pass(void)
{
	sid_pal_dsa_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_SECP256R1;
	params.key_size = sizeof(public_SECP256R1);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_verify_message_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_SECP256R1_verify_false_positives(void)
{
	sid_pal_dsa_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED, PSA_SUCCESS };
	psa_status_t psa_verify_message_ret[] = { PSA_ERROR_DATA_INVALID, PSA_ERROR_DATA_INVALID };
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_verify_message, psa_verify_message_ret, sizeof(psa_verify_message_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_SECP256R1;
	params.key_size = sizeof(public_SECP256R1);
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_ecc_dsa(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));
}

void test_sid_pal_crypto_ecc_dsa_SECP256R1_buffer_overflow(void)
{
	sid_pal_dsa_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN];

	memset(&params, 0x00, sizeof(params));
	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_SECP256R1;
	params.in = data;
	params.in_size = sizeof(data);
	params.signature = signature;
	params.sig_size = sizeof(signature);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_verify_message_fake.return_val = PSA_SUCCESS;

	params.key_size = sizeof(public_SECP256R1) + 1;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

	params.key_size = sizeof(public_SECP256R1) * 2;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

	psa_destroy_key_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
}

/*************************************************************************
* ECDSA
* ***********************************************************************/

/*************************************************************************
* EC KEY GENERATION
* ***********************************************************************/
void test_sid_pal_crypto_ecc_key_gen_null_ptr(void)
{
	sid_pal_ecc_key_gen_params_t params;
	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_key_gen(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_key_gen(&params));

	params.prk = private_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_key_gen(&params));

	params.prk = NULL;
	params.puk = public_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_key_gen(&params));
}

void test_sid_pal_crypto_ecc_key_gen_invalid_args(void)
{
	sid_pal_ecc_key_gen_params_t params;
	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_EDDSA_ED25519;
	params.prk = private_key;
	params.puk = public_key;
	params.prk_size = sizeof(private_key);
	params.puk_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_key_gen(&params));

	params.prk = private_key;
	params.puk = public_key;
	params.prk_size = 0;
	params.puk_size = sizeof(public_key);
	psa_generate_key_fake.return_val = PSA_ERROR_INVALID_ARGUMENT;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_key_gen(&params));
}

void test_sid_pal_crypto_ecc_key_gen_incorrect_algo(void)
{
	sid_pal_ecc_key_gen_params_t params;
	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.prk = private_key;
	params.prk_size = sizeof(private_key);
	params.puk = public_key;
	params.puk_size = sizeof(public_key);

	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_key_gen(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_key_gen(&params));
}

void test_sid_pal_crypto_ecc_key_gen_ED25519_pass(void)
{
	sid_pal_ecc_key_gen_params_t params;
	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_EDDSA_ED25519;
	params.prk = private_key;
	params.prk_size = EC_ED25519_PRIV_KEY_LEN;
	params.puk = public_key;
	params.puk_size = EC_ED25519_PUB_KEY_LEN;

	psa_generate_key_fake.return_val = PSA_SUCCESS;
	psa_export_key_fake.return_val = PSA_SUCCESS;
	psa_export_public_key_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_key_gen(&params));
}

void test_sid_pal_crypto_ecc_key_gen_CURVE25519_pass(void)
{
	sid_pal_ecc_key_gen_params_t params;
	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDH_CURVE25519;
	params.prk = private_key;
	params.prk_size = EC_CURVE25519_PRIV_KEY_LEN;
	params.puk = public_key;
	params.puk_size = EC_CURVE25519_PUB_KEY_LEN;

	psa_generate_key_fake.return_val = PSA_SUCCESS;
	psa_export_key_fake.return_val = PSA_SUCCESS;
	psa_export_public_key_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_key_gen(&params));
}

void test_sid_pal_crypto_ecc_key_gen_SECP256R1_pass(void)
{
	sid_pal_ecc_key_gen_params_t params;
	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.prk = private_key;
	params.prk_size = EC_SECP256R1_PRIV_KEY_LEN;
	params.puk = public_key;
	params.puk_size = EC_SECP256R1_PUB_KEY_LEN;

	psa_generate_key_fake.return_val = PSA_SUCCESS;
	psa_export_key_fake.return_val = PSA_SUCCESS;
	psa_export_public_key_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_key_gen(&params));
}

void test_sid_pal_crypto_ecc_key_gen_false_positives(void)
{
	sid_pal_ecc_key_gen_params_t params;
	psa_status_t psa_generate_key_ret[] = { PSA_ERROR_NOT_PERMITTED, PSA_SUCCESS, PSA_SUCCESS };
	psa_status_t psa_export_key_ret[] = { PSA_ERROR_BUFFER_TOO_SMALL, PSA_SUCCESS,
					      PSA_SUCCESS };
	psa_status_t psa_export_public_key_ret[] = { PSA_ERROR_BUFFER_TOO_SMALL, PSA_SUCCESS,
						     PSA_SUCCESS };
	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_generate_key, psa_generate_key_ret, sizeof(psa_generate_key_ret));
	SET_RETURN_SEQ(psa_export_key, psa_export_key_ret, sizeof(psa_export_key_ret));
	SET_RETURN_SEQ(psa_export_public_key, psa_export_public_key_ret,
		       sizeof(psa_export_public_key_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.prk = private_key;
	params.prk_size = EC_SECP256R1_PRIV_KEY_LEN;
	params.puk = public_key;
	params.puk_size = EC_SECP256R1_PUB_KEY_LEN;

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_ecc_key_gen(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_ecc_key_gen(&params));
	psa_destroy_key_fake.return_val = PSA_ERROR_BAD_STATE;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_ecc_key_gen(&params));
}

/*************************************************************************
* END EC KEY GENERATION
* ***********************************************************************/

/*************************************************************************
* AEAD
* ***********************************************************************/
void test_sid_pal_crypto_aead_null_ptr(void)
{
	sid_pal_aead_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.algo = SID_PAL_AEAD_GCM_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.aad = additional_data;
	params.in = data;
	params.out = encrypted_data;
	params.mac = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.aad = additional_data;
	params.in = data;
	params.out = NULL;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.aad = additional_data;
	params.in = NULL;
	params.out = encrypted_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.aad = NULL;
	params.in = data;
	params.out = encrypted_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = NULL;
	params.aad = additional_data;
	params.in = data;
	params.out = encrypted_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = NULL;
	params.aad = additional_data;
	params.in = NULL;
	params.out = encrypted_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.aad = NULL;
	params.in = data;
	params.out = NULL;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.aad = additional_data;
	params.in = NULL;
	params.out = encrypted_data;
	params.mac = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = NULL;
	params.aad = additional_data;
	params.in = data;
	params.out = encrypted_data;
	params.mac = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));
}

void test_sid_pal_crypto_aead_invalid_args(void)
{
	sid_pal_aead_params_t params;
	uint8_t data_copy[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_AEAD_GCM_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.aad = additional_data;
	params.in = data_copy;
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.mac = mac;

	params.in_size = 0;
	params.aad_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.in_size = 0;
	params.aad_size = sizeof(additional_data);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.in_size = sizeof(data_copy);
	params.aad_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.in_size = sizeof(data_copy);
	params.aad_size = sizeof(additional_data);
	params.key_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.key_size = sizeof(aes_128_test_key);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.key_size = 512;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.key_size = sizeof(aes_128_test_key) * 8;
	psa_import_key_fake.return_val = PSA_SUCCESS;
	params.mode = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.mode = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));
}

void test_sid_pal_crypto_aead_incorrect_algo(void)
{
	sid_pal_aead_params_t params;
	uint8_t data_copy[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.mac = mac;

	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_aead_crypt(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_aead_crypt(&params));
}

void test_sid_pal_crypto_aead_gcm_128_encrypt_pass(void)
{
	sid_pal_aead_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t iv[AES_GCM_IV_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_AEAD_GCM_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_GCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data;
	params.in_size = sizeof(data);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_aead_encrypt_setup_fake.return_val = PSA_SUCCESS;
	psa_aead_set_lengths_fake.return_val = PSA_SUCCESS;
	psa_aead_set_nonce_fake.return_val = PSA_SUCCESS;
	psa_aead_update_ad_fake.return_val = PSA_SUCCESS;

	memset(&mock_psa_aead_update_values, 0, sizeof(mock_psa_aead_update_values));
	MOCK_PSA_AEAD_UPDATE_SET_RETURN(PSA_SUCCESS);
	MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH(params.out_size - 1);

	custom_psa_aead_update_t custom_psa_aead_update[] = { mock_psa_aead_update };

	SET_CUSTOM_FAKE_SEQ(psa_aead_update, custom_psa_aead_update,
			    ARRAY_SIZE(custom_psa_aead_update));

	psa_aead_finish_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
}

void test_sid_pal_crypto_aead_gcm_128_decrypt_pass(void)
{
	sid_pal_aead_params_t params;
	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t iv[AES_GCM_IV_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_AEAD_GCM_128;
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_GCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data;
	params.in_size = sizeof(data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_aead_decrypt_setup_fake.return_val = PSA_SUCCESS;
	psa_aead_set_lengths_fake.return_val = PSA_SUCCESS;
	psa_aead_set_nonce_fake.return_val = PSA_SUCCESS;
	psa_aead_update_ad_fake.return_val = PSA_SUCCESS;

	memset(&mock_psa_aead_update_values, 0, sizeof(mock_psa_aead_update_values));
	MOCK_PSA_AEAD_UPDATE_SET_RETURN(PSA_SUCCESS);
	MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH(params.out_size - 1);

	custom_psa_aead_update_t custom_psa_aead_update[] = { mock_psa_aead_update };

	SET_CUSTOM_FAKE_SEQ(psa_aead_update, custom_psa_aead_update,
			    ARRAY_SIZE(custom_psa_aead_update));

	psa_aead_verify_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
}

void test_sid_pal_crypto_aead_ccm_128_encrypt_pass(void)
{
	sid_pal_aead_params_t params;

	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t iv[AES_CCM_IV_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_AEAD_CCM_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_CCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data;
	params.in_size = sizeof(data);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_aead_encrypt_setup_fake.return_val = PSA_SUCCESS;
	psa_aead_set_lengths_fake.return_val = PSA_SUCCESS;
	psa_aead_set_nonce_fake.return_val = PSA_SUCCESS;
	psa_aead_update_ad_fake.return_val = PSA_SUCCESS;

	memset(&mock_psa_aead_update_values, 0, sizeof(mock_psa_aead_update_values));
	MOCK_PSA_AEAD_UPDATE_SET_RETURN(PSA_SUCCESS);
	MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH(params.out_size - 1);

	custom_psa_aead_update_t custom_psa_aead_update[] = { mock_psa_aead_update };

	SET_CUSTOM_FAKE_SEQ(psa_aead_update, custom_psa_aead_update,
			    ARRAY_SIZE(custom_psa_aead_update));

	psa_aead_finish_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
}

void test_sid_pal_crypto_aead_ccm_128_decrypt_pass(void)
{
	sid_pal_aead_params_t params;

	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t iv[AES_CCM_IV_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_AEAD_CCM_128;
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_CCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data;
	params.in_size = sizeof(data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_aead_decrypt_setup_fake.return_val = PSA_SUCCESS;
	psa_aead_set_lengths_fake.return_val = PSA_SUCCESS;
	psa_aead_set_nonce_fake.return_val = PSA_SUCCESS;
	psa_aead_update_ad_fake.return_val = PSA_SUCCESS;

	memset(&mock_psa_aead_update_values, 0, sizeof(mock_psa_aead_update_values));
	MOCK_PSA_AEAD_UPDATE_SET_RETURN(PSA_SUCCESS);
	MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH(params.out_size - 1);

	custom_psa_aead_update_t custom_psa_aead_update[] = { mock_psa_aead_update };

	SET_CUSTOM_FAKE_SEQ(psa_aead_update, custom_psa_aead_update,
			    ARRAY_SIZE(custom_psa_aead_update));

	psa_aead_verify_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
}

void test_sid_pal_crypto_aead_no_iv(void)
{
	sid_pal_aead_params_t params;

	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_AEAD_GCM_128;
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = NULL;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data;
	params.in_size = sizeof(data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_aead_decrypt_setup_fake.return_val = PSA_SUCCESS;
	psa_aead_set_lengths_fake.return_val = PSA_SUCCESS;
	psa_aead_update_ad_fake.return_val = PSA_SUCCESS;

	memset(&mock_psa_aead_update_values, 0, sizeof(mock_psa_aead_update_values));
	MOCK_PSA_AEAD_UPDATE_SET_RETURN(PSA_SUCCESS);
	MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH(params.out_size - 1);

	custom_psa_aead_update_t custom_psa_aead_update[] = { mock_psa_aead_update };

	SET_CUSTOM_FAKE_SEQ(psa_aead_update, custom_psa_aead_update,
			    ARRAY_SIZE(custom_psa_aead_update));

	psa_aead_verify_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(0, psa_aead_set_nonce_fake.call_count);
}

void test_sid_pal_crypto_aead_decrypt_false_positives(void)
{
	sid_pal_aead_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS };
	psa_status_t psa_aead_decrypt_setup_ret[] = { PSA_ERROR_INVALID_ARGUMENT,
						      PSA_SUCCESS,
						      PSA_SUCCESS,
						      PSA_SUCCESS,
						      PSA_SUCCESS,
						      PSA_SUCCESS,
						      PSA_SUCCESS };
	psa_status_t psa_aead_set_lengths_ret[] = { PSA_ERROR_INVALID_ARGUMENT,
						    PSA_SUCCESS,
						    PSA_SUCCESS,
						    PSA_SUCCESS,
						    PSA_SUCCESS,
						    PSA_SUCCESS,
						    PSA_SUCCESS };
	psa_status_t psa_aead_set_nonce_ret[] = { PSA_ERROR_INVALID_ARGUMENT,
						  PSA_SUCCESS,
						  PSA_SUCCESS,
						  PSA_SUCCESS,
						  PSA_SUCCESS,
						  PSA_SUCCESS,
						  PSA_SUCCESS };
	psa_status_t psa_aead_update_ad_ret[] = { PSA_ERROR_BAD_STATE, PSA_SUCCESS, PSA_SUCCESS,
						  PSA_SUCCESS,	       PSA_SUCCESS, PSA_SUCCESS,
						  PSA_SUCCESS };
	psa_status_t psa_aead_verify_ret[] = { PSA_ERROR_BAD_STATE, PSA_SUCCESS, PSA_SUCCESS,
					       PSA_SUCCESS,	    PSA_SUCCESS, PSA_SUCCESS,
					       PSA_SUCCESS };

	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t iv[AES_GCM_IV_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_aead_decrypt_setup, psa_aead_decrypt_setup_ret,
		       sizeof(psa_aead_decrypt_setup_ret));
	SET_RETURN_SEQ(psa_aead_set_lengths, psa_aead_set_lengths_ret,
		       sizeof(psa_aead_set_lengths_ret));
	SET_RETURN_SEQ(psa_aead_set_nonce, psa_aead_set_nonce_ret, sizeof(psa_aead_set_nonce_ret));
	SET_RETURN_SEQ(psa_aead_update_ad, psa_aead_update_ad_ret, sizeof(psa_aead_update_ad_ret));
	SET_RETURN_SEQ(psa_aead_verify, psa_aead_verify_ret, sizeof(psa_aead_verify_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_AEAD_GCM_128;
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_GCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data;
	params.in_size = sizeof(data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	memset(&mock_psa_aead_update_values, 0, sizeof(mock_psa_aead_update_values));
	MOCK_PSA_AEAD_UPDATE_SET_RETURN(PSA_ERROR_BAD_STATE, PSA_SUCCESS, PSA_SUCCESS, PSA_SUCCESS,
					PSA_SUCCESS, PSA_SUCCESS, PSA_SUCCESS);
	MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH(0, 0, 1, params.out_size - 1,
						   params.out_size - 2, params.out_size - 1,
						   params.out_size - 1);

	custom_psa_aead_update_t custom_psa_aead_update[] = { mock_psa_aead_update };

	SET_CUSTOM_FAKE_SEQ(psa_aead_update, custom_psa_aead_update,
			    ARRAY_SIZE(custom_psa_aead_update));

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(0, psa_destroy_key_fake.call_count);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aead_crypt(&params));
	psa_aead_abort_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aead_crypt(&params));
}

void test_sid_pal_crypto_aead_encrypt_false_positives(void)
{
	sid_pal_aead_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS,
					      PSA_SUCCESS };
	psa_status_t psa_aead_encrypt_setup_ret[] = { PSA_ERROR_INVALID_ARGUMENT,
						      PSA_SUCCESS,
						      PSA_SUCCESS,
						      PSA_SUCCESS,
						      PSA_SUCCESS,
						      PSA_SUCCESS,
						      PSA_SUCCESS };
	psa_status_t psa_aead_set_lengths_ret[] = { PSA_ERROR_INVALID_ARGUMENT,
						    PSA_SUCCESS,
						    PSA_SUCCESS,
						    PSA_SUCCESS,
						    PSA_SUCCESS,
						    PSA_SUCCESS,
						    PSA_SUCCESS };
	psa_status_t psa_aead_set_nonce_ret[] = { PSA_ERROR_INVALID_ARGUMENT,
						  PSA_SUCCESS,
						  PSA_SUCCESS,
						  PSA_SUCCESS,
						  PSA_SUCCESS,
						  PSA_SUCCESS,
						  PSA_SUCCESS };
	psa_status_t psa_aead_update_ad_ret[] = { PSA_ERROR_BAD_STATE, PSA_SUCCESS, PSA_SUCCESS,
						  PSA_SUCCESS,	       PSA_SUCCESS, PSA_SUCCESS,
						  PSA_SUCCESS };
	psa_status_t psa_aead_finish_ret[] = { PSA_ERROR_BAD_STATE, PSA_SUCCESS, PSA_SUCCESS,
					       PSA_SUCCESS,	    PSA_SUCCESS, PSA_SUCCESS,
					       PSA_SUCCESS };

	uint8_t data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t iv[AES_GCM_IV_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE];

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_aead_encrypt_setup, psa_aead_encrypt_setup_ret,
		       sizeof(psa_aead_encrypt_setup_ret));
	SET_RETURN_SEQ(psa_aead_set_lengths, psa_aead_set_lengths_ret,
		       sizeof(psa_aead_set_lengths_ret));
	SET_RETURN_SEQ(psa_aead_set_nonce, psa_aead_set_nonce_ret, sizeof(psa_aead_set_nonce_ret));
	SET_RETURN_SEQ(psa_aead_update_ad, psa_aead_update_ad_ret, sizeof(psa_aead_update_ad_ret));
	SET_RETURN_SEQ(psa_aead_finish, psa_aead_finish_ret, sizeof(psa_aead_finish_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_AEAD_GCM_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_GCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data;
	params.in_size = sizeof(data);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	memset(&mock_psa_aead_update_values, 0, sizeof(mock_psa_aead_update_values));
	MOCK_PSA_AEAD_UPDATE_SET_RETURN(PSA_ERROR_BAD_STATE, PSA_SUCCESS, PSA_SUCCESS, PSA_SUCCESS,
					PSA_SUCCESS, PSA_SUCCESS, PSA_SUCCESS);
	MOCK_PSA_AEAD_UPDATE_SET_OUT_OUTPUT_LENGTH(0, 0, 1, params.out_size - 1,
						   params.out_size - 2, params.out_size - 1,
						   params.out_size - 1);

	custom_psa_aead_update_t custom_psa_aead_update[] = { mock_psa_aead_update };

	SET_CUSTOM_FAKE_SEQ(psa_aead_update, custom_psa_aead_update,
			    ARRAY_SIZE(custom_psa_aead_update));

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(0, psa_destroy_key_fake.call_count);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aead_crypt(&params));
	psa_aead_abort_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	psa_destroy_key_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_STATE, sid_pal_crypto_aead_crypt(&params));
}

/*************************************************************************
* END AEAD
* ***********************************************************************/

/*************************************************************************
* ECDH
* ***********************************************************************/
void test_sid_pal_crypto_ecc_ecdh_null_ptr(void)
{
	sid_pal_ecdh_params_t params;
	uint8_t private_key[EC_SECP256R1_PRIV_KEY_LEN];
	uint8_t public_key[EC_SECP256R1_PUB_KEY_LEN];
	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_ecdh(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_ecdh(&params));

	params.algo = SID_PAL_ECDH_SECP256R1;
	params.prk = NULL;
	params.puk = public_key;
	params.shared_secret = secret_result;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_ecdh(&params));

	params.prk = private_key;
	params.puk = NULL;
	params.shared_secret = secret_result;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_ecdh(&params));

	params.prk = private_key;
	params.puk = public_key;
	params.shared_secret = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_ecdh(&params));

	params.prk = NULL;
	params.puk = public_key;
	params.shared_secret = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_ecdh(&params));

	params.prk = private_key;
	params.puk = NULL;
	params.shared_secret = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_ecdh(&params));

	params.prk = NULL;
	params.puk = NULL;
	params.shared_secret = secret_result;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_ecdh(&params));
}

void test_sid_pal_crypto_ecc_ecdh_incorrect_algo(void)
{
	sid_pal_ecdh_params_t params;
	uint8_t private_key[EC_SECP256R1_PRIV_KEY_LEN];
	uint8_t public_key[EC_SECP256R1_PUB_KEY_LEN];
	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.prk = private_key;
	params.prk_size = sizeof(private_key);
	params.puk = public_key;
	params.puk_size = sizeof(public_key);
	params.shared_secret = secret_result;
	params.shared_secret_sz = sizeof(secret_result);

	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_ecdh(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_ecdh(&params));
}

void test_sid_pal_crypto_ecc_ecdh_SECP256R1_pass(void)
{
	sid_pal_ecdh_params_t params;
	uint8_t private_key[EC_SECP256R1_PRIV_KEY_LEN];
	uint8_t public_key[EC_SECP256R1_PUB_KEY_LEN];
	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDH_SECP256R1;
	params.prk = private_key;
	params.prk_size = sizeof(private_key);
	params.puk = public_key;
	params.puk_size = sizeof(public_key);
	params.shared_secret = secret_result;
	params.shared_secret_sz = sizeof(secret_result);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_raw_key_agreement_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&params));
}

void test_sid_pal_crypto_ecc_ecdh_CURVE25519_pass(void)
{
	sid_pal_ecdh_params_t params;
	uint8_t private_key[EC_CURVE25519_PRIV_KEY_LEN];
	uint8_t public_key[EC_CURVE25519_PUB_KEY_LEN];
	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDH_CURVE25519;
	params.prk = private_key;
	params.prk_size = sizeof(private_key);
	params.puk = public_key;
	params.puk_size = sizeof(public_key);
	params.shared_secret = secret_result;
	params.shared_secret_sz = sizeof(secret_result);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_raw_key_agreement_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&params));
}

void test_sid_pal_crypto_ecc_ecdh_false_positives(void)
{
	sid_pal_ecdh_params_t params;
	psa_status_t psa_import_key_ret[] = { PSA_ERROR_NOT_PERMITTED, PSA_SUCCESS };
	psa_status_t psa_raw_key_agreement_ret[] = { PSA_ERROR_GENERIC_ERROR, PSA_SUCCESS };

	uint8_t private_key[EC_SECP256R1_PRIV_KEY_LEN];
	uint8_t public_key[EC_SECP256R1_PUB_KEY_LEN];
	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	memset(&params, 0x00, sizeof(params));
	SET_RETURN_SEQ(psa_import_key, psa_import_key_ret, sizeof(psa_import_key_ret));
	SET_RETURN_SEQ(psa_raw_key_agreement, psa_raw_key_agreement_ret,
		       sizeof(psa_raw_key_agreement_ret));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDH_SECP256R1;
	params.prk = private_key;
	params.prk_size = sizeof(private_key);
	params.puk = public_key;
	params.puk_size = sizeof(public_key);
	params.shared_secret = secret_result;
	params.shared_secret_sz = sizeof(secret_result);

	TEST_ASSERT_EQUAL(SID_ERROR_NO_PERMISSION, sid_pal_crypto_ecc_ecdh(&params));
	psa_destroy_key_fake.return_val = PSA_ERROR_GENERIC_ERROR;
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, sid_pal_crypto_ecc_ecdh(&params));
}

void test_sid_pal_crypto_ecc_ecdh_SECP256R1_buffer_overflow(void)
{
	sid_pal_ecdh_params_t params;
	uint8_t private_key[EC_SECP256R1_PRIV_KEY_LEN];
	uint8_t public_key[EC_SECP256R1_PUB_KEY_LEN];
	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	memset(&params, 0x00, sizeof(params));

	psa_crypto_init_fake.return_val = PSA_SUCCESS;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	params.algo = SID_PAL_ECDH_SECP256R1;
	params.prk = private_key;
	params.prk_size = sizeof(private_key);
	params.puk = public_key;
	params.shared_secret = secret_result;
	params.shared_secret_sz = sizeof(secret_result);

	psa_import_key_fake.return_val = PSA_SUCCESS;
	psa_raw_key_agreement_fake.return_val = PSA_SUCCESS;

	params.puk_size = sizeof(public_key) + 1;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&params));

	params.puk_size = sizeof(public_key) * 2;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&params));
}

/*************************************************************************
* END ECDH
* ***********************************************************************/

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

int main(void)
{
	return unity_main();
}
