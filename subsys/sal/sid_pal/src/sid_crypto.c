/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_crypto.c
 *  @brief Sidewalk cryptography interface implementation.
 */

#include <sid_pal_crypto_ifc.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <psa/crypto.h>
#include <psa/crypto_extra.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

LOG_MODULE_REGISTER(sid_crypto, CONFIG_SIDEWALK_CRYPTO_LOG_LEVEL);

#define BYTE_TO_BITS(_byte)    (_byte << 3)
#define BITS_TO_BYTE(_bits)    (_bits >> 3)

/* AES key length in bytes. */
#define AES_128_KEY_LENGTH       (16)

/* EC key length in bits. */
#define CURVE25519_KEY_LEN_BITS  (255)
#define SECP256R1_KEY_LEN_BITS   (256)
#define ED25519_KEY_LEN_BITS     (255)

/* Data chunk used in cryptographic algorithm. */
#define ALGO_DATA_CHUNK (32)

/* Max. EC key buffer length in bytes. */
#define EC_MAX_KEY_LENGTH        (65)
#define EC_MAX_PUBLIC_KEY_LENGTH        (EC_MAX_KEY_LENGTH)

/* Public key prefix length */
#define SECPxxx_KEY_PREFIX_LEN  (0x01)

/* Public key prefix offset */
#define SECPxxx_PREFIX_OFFSET   (0)

/* Public key x coordinate offset in buffer. */
#define SECPxxx_KEY_OFFSET      (SECPxxx_KEY_PREFIX_LEN)

/* Sid AES mode to PSA key usage policy. */
#define AES_MODE_TO_USAGE(_mode) ((SID_PAL_CRYPTO_ENCRYPT == _mode) ? PSA_KEY_USAGE_ENCRYPT :  \
				  ((SID_PAL_CRYPTO_DECRYPT == _mode) ? PSA_KEY_USAGE_DECRYPT : \
				   PSA_KEY_USAGE_SIGN_MESSAGE))

/* Sid ECDSA mode to PSA key usage policy. */
#define ECDSA_MODE_TO_USAGE(_mode) ((SID_PAL_CRYPTO_VERIFY == _mode) ? PSA_KEY_USAGE_VERIFY_MESSAGE : \
				    PSA_KEY_USAGE_SIGN_MESSAGE)

/* Sid ECDSA mode to PSA key key family type. */
#define ECC_FAMILY_TYPE(_mode, _type) ((SID_PAL_CRYPTO_VERIFY == _mode) ? \
				       PSA_KEY_TYPE_ECC_PUBLIC_KEY(_type) : PSA_KEY_TYPE_ECC_KEY_PAIR(_type))

/* Crypto initialization global flag. */
static bool is_initialized = false;

/* Prefix for uncompressed public key */
static const uint8_t secpxxx_key_prefix[SECPxxx_KEY_PREFIX_LEN] = { 0x04 };

static sid_error_t get_error(psa_status_t psa_erc);
static psa_status_t prepare_key(const uint8_t *key, size_t key_length, size_t key_bits,
				psa_key_usage_t usage_flags, psa_algorithm_t alg, psa_key_type_t type,
				psa_key_handle_t *key_handle);
static psa_status_t aes_execute(psa_cipher_operation_t *operation,
				sid_pal_aes_params_t *params);
static psa_status_t aes_encrypt(psa_key_handle_t key_handle,
				sid_pal_aes_params_t *params);
static psa_status_t aes_decrypt(psa_key_handle_t key_handle,
				sid_pal_aes_params_t *params);
static psa_status_t aead_execute(psa_aead_operation_t *op,
				 sid_pal_aead_params_t *params);
static psa_status_t aead_encrypt(psa_key_handle_t key_handle,
				 sid_pal_aead_params_t *params, psa_algorithm_t alg);
static psa_status_t aead_decrypt(psa_key_handle_t key_handle,
				 sid_pal_aead_params_t *params, psa_algorithm_t alg);

/**
 * @brief Get the psa error object and return sidewalk error code.
 *
 * @param psa_erc - psa error code.
 * @return sidewalk error code.
 */
static sid_error_t get_error(psa_status_t psa_erc)
{
	sid_error_t sid_erc = SID_ERROR_NONE;

	if (PSA_SUCCESS != psa_erc) {
		LOG_ERR("PSA Error code: %d", psa_erc);
	}

	switch (psa_erc) {
	case PSA_SUCCESS:
		sid_erc = SID_ERROR_NONE;
		break;
	case PSA_ERROR_NOT_SUPPORTED:
		sid_erc = SID_ERROR_NOSUPPORT;
		break;
	case PSA_ERROR_INSUFFICIENT_MEMORY:
		sid_erc = SID_ERROR_OOM;
		break;
	case PSA_ERROR_INVALID_ARGUMENT:
		sid_erc = SID_ERROR_INVALID_ARGS;
		break;
	case PSA_ERROR_BUFFER_TOO_SMALL:
		sid_erc = SID_ERROR_OUT_OF_RESOURCES;
		break;
	case PSA_ERROR_BAD_STATE:
		sid_erc = SID_ERROR_INVALID_STATE;
		break;
	case PSA_ERROR_NOT_PERMITTED:
		sid_erc = SID_ERROR_NO_PERMISSION;
		break;
	case PSA_ERROR_DATA_INVALID:
		sid_erc = SID_ERROR_NULL_POINTER;
		break;
	default:
		sid_erc = SID_ERROR_GENERIC;
	}
	return sid_erc;
}

/**
 * @brief The function prepares binaries key for use in cryptographic algorithms.
 *
 * @param key - binary key buffer.
 * @param key_length - key length in bytes.
 * @param key_bits - key length in bits.
 * @param usage_flags - define which opeartions are permitted with te key.
 * @param alg - key permitted-algorithm policy.
 * @param type - key type.
 * @param key_handle - handle to key (null when key cannot be set).
 *
 * @return PSA_SUCCESS when success, otherwise error code.
 */
static psa_status_t prepare_key(const uint8_t *key, size_t key_length, size_t key_bits,
				psa_key_usage_t usage_flags, psa_algorithm_t alg, psa_key_type_t type,
				psa_key_handle_t *key_handle)
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_status_t status;

	if (!key_handle) {
		return PSA_ERROR_DATA_INVALID;
	}

	psa_set_key_usage_flags(&attributes, usage_flags);
	psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_algorithm(&attributes, alg);
	psa_set_key_type(&attributes, type);
	psa_set_key_bits(&attributes, key_bits);

	status = psa_import_key(&attributes, key, key_length, key_handle);
	if (PSA_SUCCESS == status) {
		psa_reset_key_attributes(&attributes);
	}

	return status;
}

/**
 * @brief Perform the AES algorithm.
 * NOTE: The algorithm must be set before calling this function.
 *
 * @param operation - active cipher operation.
 * @param params - AES parameters.
 *
 * @return PSA_SUCCESS when success, otherwise error code.
 */
static psa_status_t aes_execute(psa_cipher_operation_t *operation,
				sid_pal_aes_params_t *params)
{
	psa_status_t status;
	size_t out_len;

	status = psa_cipher_set_iv(operation, params->iv, params->iv_size);

	if (PSA_SUCCESS == status) {
		LOG_DBG("psa_cipher_set_iv success.");
		status = psa_cipher_update(operation,
					   params->in, params->in_size,
					   params->out, params->out_size,
					   &out_len);

		if (PSA_SUCCESS == status) {
			LOG_DBG("psa_cipher_update success.");
			status = psa_cipher_finish(operation,
						   params->out + out_len,
						   params->out_size - out_len,
						   &out_len);
		}
	}
	return status;
}

/**
 * @brief Encrypt a raw data using the AES_CTR.
 *
 * @param key_handle - key to use for encryption operation.
 * @param params - AES parameters.
 *
 * @return PSA_SUCCESS when success, otherwise error code.
 */
static psa_status_t aes_encrypt(psa_key_handle_t key_handle,
				sid_pal_aes_params_t *params)
{
	psa_status_t status;
	psa_cipher_operation_t operation = PSA_CIPHER_OPERATION_INIT;

	status = psa_cipher_encrypt_setup(&operation, key_handle, PSA_ALG_CTR);

	if (PSA_SUCCESS == status) {
		LOG_DBG("psa_cipher_encrypt_setup success.");
		status = aes_execute(&operation, params);
	}

	if (PSA_SUCCESS != status) {
		if (PSA_SUCCESS != psa_cipher_abort(&operation)) {
			LOG_WRN("Abort failed!");
		}
	}

	return status;
}

/**
 * @brief Decrypt a encrypted data using the AES_CTR.
 *
 * @param key_handle - key to use for decryption operation.
 * @param params - AES parameters.
 *
 * @return PSA_SUCCESS when success, otherwise error code.
 */
static psa_status_t aes_decrypt(psa_key_handle_t key_handle,
				sid_pal_aes_params_t *params)
{
	psa_status_t status;
	psa_cipher_operation_t operation = PSA_CIPHER_OPERATION_INIT;

	status = psa_cipher_decrypt_setup(&operation, key_handle, PSA_ALG_CTR);

	if (PSA_SUCCESS == status) {
		LOG_DBG("psa_cipher_decrypt_setup success.");
		status = aes_execute(&operation, params);
	}

	if (PSA_SUCCESS != status) {
		if (PSA_SUCCESS != psa_cipher_abort(&operation)) {
			LOG_WRN("Abort failed!");
		}
	}

	return status;
}

/**
 * @brief Perform the AEAD algorithm.
 * NOTE: The algorithm must be set before calling this function.
 *
 * @param op - active cipher operation.
 * @param params - AES parameters.
 * @return PSA_SUCCESS when success, otherwise error code.
 */
static psa_status_t aead_execute(psa_aead_operation_t *op,
				 sid_pal_aead_params_t *params)
{
	size_t out_len, mac_len;
	psa_status_t status = psa_aead_set_lengths(op, params->aad_size, params->in_size);

	if (PSA_SUCCESS == status) {
		LOG_DBG("psa_aead_set_lengths success.");
		if (NULL != params->iv) {
			status = psa_aead_set_nonce(op, params->iv, params->iv_size);
		}

		if (PSA_SUCCESS == status) {
			LOG_DBG("psa_aead_set_nonce success.");
			status = psa_aead_update_ad(op, params->aad, params->aad_size);

			if (PSA_SUCCESS == status) {
				LOG_DBG("psa_aead_update_ad success.");
				status = psa_aead_update(op, params->in, params->in_size,
							 params->out, params->out_size, &out_len);

				if (PSA_SUCCESS == status) {
					LOG_DBG("psa_aead_update_ad success (out_len=%d).", out_len);
					if (SID_PAL_CRYPTO_ENCRYPT == params->mode) {
						status = psa_aead_finish(op,
									 params->out + out_len,
									 params->out_size - out_len, &out_len,
									 params->mac, params->mac_size, &mac_len);
						LOG_DBG("psa_aead_finish %s (out_len=%d, mac_len=%d)",
							(PSA_SUCCESS == status) ? "success." : "failed!",
							out_len, mac_len);
					} else {
						status = psa_aead_verify(op,
									 params->out + out_len,
									 params->out_size - out_len, &out_len,
									 params->mac, params->mac_size);
						LOG_DBG("psa_aead_verify %s (out_len=%d)",
							(PSA_SUCCESS == status) ? "success." : "failed!",
							out_len);
					}
				}
			}
		}
	}
	return status;
}

/**
 * @brief The function processes authenticated encryption operation.
 *
 * @param key_handle - key to use for decryption operation.
 * @param params - AEAD parameters.
 * @param alg - AEAD algorithm to compute.
 *
 * @return PSA_SUCCESS when success, otherwise error code.
 */
static psa_status_t aead_encrypt(psa_key_handle_t key_handle,
				 sid_pal_aead_params_t *params, psa_algorithm_t alg)
{
	psa_aead_operation_t op = PSA_AEAD_OPERATION_INIT;
	psa_status_t status = psa_aead_encrypt_setup(&op, key_handle, alg);

	if (PSA_SUCCESS == status) {
		status = aead_execute(&op, params);
	}

	if (PSA_SUCCESS != psa_aead_abort(&op)) {
		LOG_WRN("Abort failed!");
	}

	return status;
}

/**
 * @brief The function processes authenticated decryption operation.
 *
 * @param key_handle - key to use for decryption operation.
 * @param params - AEAD parameters.
 * @param alg - AEAD algorithm to compute.
 *
 * @return PSA_SUCCESS when success, otherwise error code.
 */
static psa_status_t aead_decrypt(psa_key_handle_t key_handle,
				 sid_pal_aead_params_t *params, psa_algorithm_t alg)
{
	psa_aead_operation_t op = PSA_AEAD_OPERATION_INIT;
	psa_status_t status = psa_aead_decrypt_setup(&op, key_handle, alg);

	if (PSA_SUCCESS == status) {
		status = aead_execute(&op, params);
	}

	if (PSA_SUCCESS != psa_aead_abort(&op)) {
		LOG_WRN("Abort failed!");
	}

	return status;
}

sid_error_t sid_pal_crypto_init(void)
{
	psa_status_t status = psa_crypto_init();

	if (PSA_SUCCESS == status) {
		is_initialized = true;
		LOG_DBG("Init success!");
	} else {
		LOG_ERR("Init failed! (sts: %d)", status);
	}

	return get_error(status);
}

sid_error_t sid_pal_crypto_deinit(void)
{
	is_initialized = false;
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_crypto_rand(uint8_t *rand, size_t size)
{
	if (!is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!rand) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!size) {
		return SID_ERROR_INVALID_ARGS;
	}

	return get_error(psa_generate_random(rand, size));
}

sid_error_t sid_pal_crypto_hash(sid_pal_hash_params_t *params)
{
	psa_algorithm_t alg_sha;
	size_t hash_length;

	if (!is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!params || !params->data || !params->digest) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!params->data_size || !params->digest_size) {
		return SID_ERROR_INVALID_ARGS;
	}

	switch (params->algo) {
	case SID_PAL_HASH_SHA256:
		alg_sha = PSA_ALG_SHA_256;
		break;
	case SID_PAL_HASH_SHA512:
		alg_sha = PSA_ALG_SHA_512;
		break;
	default:
		return SID_ERROR_NOSUPPORT;
	}

	return get_error(psa_hash_compute(alg_sha,
					  params->data, params->data_size,
					  params->digest, params->digest_size,
					  &hash_length));
}

sid_error_t sid_pal_crypto_hmac(sid_pal_hmac_params_t *params)
{
	psa_status_t status;
	psa_mac_operation_t operation = PSA_MAC_OPERATION_INIT;
	psa_algorithm_t alg_sha;
	psa_key_handle_t key_handle;

	if (!is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!params || !params->key || !params->data || !params->digest) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!params->key_size || !params->data_size || !params->digest_size) {
		return SID_ERROR_INVALID_ARGS;
	}

	switch (params->algo) {
	case SID_PAL_HASH_SHA256:
		alg_sha = PSA_ALG_SHA_256;
		break;
	case SID_PAL_HASH_SHA512:
		alg_sha = PSA_ALG_SHA_512;
		break;
	default:
		return SID_ERROR_NOSUPPORT;
	}

	// NOTE: key_size is in bytes.
	status = prepare_key(params->key, params->key_size, BYTE_TO_BITS(params->key_size),
			     PSA_KEY_USAGE_SIGN_HASH, PSA_ALG_HMAC(alg_sha), PSA_KEY_TYPE_HMAC,
			     &key_handle);

	if (PSA_SUCCESS == status) {
		size_t hmac_length;
		LOG_DBG("Key load success.");
		status = psa_mac_sign_setup(&operation, key_handle, PSA_ALG_HMAC(alg_sha));

		if (PSA_SUCCESS == status) {
			size_t offset = 0;
			size_t data_chunk = ALGO_DATA_CHUNK;
			size_t s_left = params->data_size;

			LOG_DBG("psa_mac_sign_setup success.");
			do {
				data_chunk = MIN(s_left, data_chunk);

				status = psa_mac_update(&operation, &params->data[offset], data_chunk);

				offset += data_chunk;
				s_left -= data_chunk;
			} while ((PSA_SUCCESS == status) && (0 != s_left));

			if (PSA_SUCCESS == status) {
				LOG_DBG("psa_mac_update success.");
				status = psa_mac_sign_finish(&operation, params->digest,
							     params->digest_size, &hmac_length);
				LOG_DBG("psa_mac_sign_finish %s [hmac length=%d]",
					(PSA_SUCCESS == status) ? "success." : "failed!", hmac_length);
			}
		}

		if (PSA_SUCCESS != psa_destroy_key(key_handle)) {
			LOG_WRN("Destroy key failed!");
		}
	}

	return get_error(status);
}

sid_error_t sid_pal_crypto_aes_crypt(sid_pal_aes_params_t *params)
{
	psa_status_t status = PSA_ERROR_NOT_SUPPORTED;
	psa_algorithm_t alg;
	size_t key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
	psa_key_handle_t key_handle;

	if (!is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!params || !params->key || !params->in || !params->out ||
	    ((SID_PAL_AES_CTR_128 == params->algo) && !params->iv)) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!params->in_size) {
		return SID_ERROR_INVALID_ARGS;
	}

	switch (params->algo) {
	case SID_PAL_AES_CMAC_128:
		alg = PSA_ALG_CMAC;
		key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
		break;
	case SID_PAL_AES_CTR_128:
		alg = PSA_ALG_CTR;
		key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
		break;
	default:
		return SID_ERROR_NOSUPPORT;
	}

	if ((key_len != params->key_size) ||
	    ((SID_PAL_AES_CTR_128 == params->algo) &&
	     params->iv_size != PSA_CIPHER_IV_LENGTH(PSA_KEY_TYPE_AES, alg))) {
		// Log is only for debug purpose, in other case use error code.
		LOG_DBG("Incorrect %s length.",
			(key_len != params->key_size) ? "key" : "IV");
		return SID_ERROR_INVALID_ARGS;
	}

	// NOTE: key_size is in bits.
	status = prepare_key(params->key, BITS_TO_BYTE(params->key_size), params->key_size,
			     AES_MODE_TO_USAGE(params->mode), alg, PSA_KEY_TYPE_AES,
			     &key_handle);

	if (PSA_SUCCESS == status) {
		LOG_DBG("Key import success");

		switch (params->mode) {
		case SID_PAL_CRYPTO_ENCRYPT:
			status = aes_encrypt(key_handle, params);
			LOG_DBG("AES encrypt %s", (PSA_SUCCESS == status) ? "success." : "failed!");
			break;
		case SID_PAL_CRYPTO_DECRYPT:
			status = aes_decrypt(key_handle, params);
			LOG_DBG("AES decrypt %s", (PSA_SUCCESS == status) ? "success." : "failed!");
			break;
		case SID_PAL_CRYPTO_MAC_CALCULATE:
		{
			size_t out_len;

			status = psa_mac_compute(key_handle, alg, params->in, params->in_size,
						 params->out, params->out_size, &out_len);
			LOG_DBG("Mac calculate %s", (PSA_SUCCESS == status) ? "success." : "failed!");
		}
		break;
		default:
			return SID_ERROR_INVALID_ARGS;
		}

		if (PSA_SUCCESS != psa_destroy_key(key_handle)) {
			LOG_WRN("Destroy key failed!");
		}
	}

	return get_error(status);
}

sid_error_t sid_pal_crypto_aead_crypt(sid_pal_aead_params_t *params)
{
	psa_status_t status = PSA_ERROR_NOT_SUPPORTED;
	psa_algorithm_t alg;
	psa_key_handle_t key_handle;
	size_t key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);

	if (!is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!params || !params->key || !params->in ||
	    !params->out || !params->aad || !params->mac) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!params->in_size || !params->aad_size) {
		return SID_ERROR_INVALID_ARGS;
	}

	switch (params->algo) {
	case SID_PAL_AEAD_GCM_128:
		alg = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, params->mac_size);
		key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
		break;
	case SID_PAL_AEAD_CCM_128:
		alg = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, params->mac_size);
		key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
		break;
	case SID_PAL_AEAD_CCM_STAR_128:
	default:
		return SID_ERROR_NOSUPPORT;
	}

	if (key_len != params->key_size) {
		return SID_ERROR_INVALID_ARGS;
	}

	if ((NULL != params->iv) &&
	    (params->iv_size != PSA_AEAD_NONCE_LENGTH(PSA_KEY_TYPE_AES, alg))) {
		return SID_ERROR_INVALID_ARGS;
	}

	// NOTE: key_size is in bits.
	status = prepare_key(params->key, BITS_TO_BYTE(params->key_size), params->key_size,
			     AES_MODE_TO_USAGE(params->mode), alg, PSA_KEY_TYPE_AES,
			     &key_handle);

	if (PSA_SUCCESS == status) {
		LOG_DBG("Key import success.");

		switch (params->mode) {
		case SID_PAL_CRYPTO_ENCRYPT:
			status = aead_encrypt(key_handle, params, alg);
			LOG_DBG("AEAD encrypt %s", (PSA_SUCCESS == status) ? "success." : "failed!");
			break;
		case SID_PAL_CRYPTO_DECRYPT:
			status = aead_decrypt(key_handle, params, alg);
			LOG_DBG("AEAD decrypt %s", (PSA_SUCCESS == status) ? "success." : "failed!");
			break;
		default:
			return SID_ERROR_INVALID_ARGS;
		}

		if (PSA_SUCCESS != psa_destroy_key(key_handle)) {
			LOG_WRN("Destroy key failed!");
		}
	}

	return get_error(status);
}

sid_error_t sid_pal_crypto_ecc_dsa(sid_pal_dsa_params_t *params)
{
	psa_status_t status;
	psa_key_handle_t key_handle;
	psa_algorithm_t alg;
	psa_ecc_family_t type;
	size_t key_len;
	uint8_t key[EC_MAX_KEY_LENGTH];
	uint8_t key_offset = 0;
	size_t key_size = 0;

	if (!is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!params || !params->key || !params->in || !params->signature) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!params->in_size || !params->key_size) {
		return SID_ERROR_INVALID_ARGS;
	}

	key_size = params->key_size;

	switch (params->algo) {
	case SID_PAL_EDDSA_ED25519:
		alg = PSA_ALG_PURE_EDDSA;
		type = PSA_ECC_FAMILY_TWISTED_EDWARDS;
		key_len = ED25519_KEY_LEN_BITS;
		break;
	case SID_PAL_ECDSA_SECP256R1:
		alg = PSA_ALG_ECDSA(PSA_ALG_SHA_256);
		type = PSA_ECC_FAMILY_SECP_R1;
		key_len = SECP256R1_KEY_LEN_BITS;

		// Sidewalk protocol has harcoded 64-byte public key length,
		// but PSA API required 65-byte public key length...
		if (SID_PAL_CRYPTO_VERIFY == params->mode) {
			// ... so, add prefix to SECPxxx public key.
			memcpy(&key[SECPxxx_PREFIX_OFFSET], secpxxx_key_prefix,
			       MIN(sizeof(key), SECPxxx_KEY_PREFIX_LEN));
			key_size += SECPxxx_KEY_PREFIX_LEN;
			key_offset = SECPxxx_KEY_OFFSET;
		}
		break;
	default:
		return SID_ERROR_NOSUPPORT;
	}

	key_size = MIN(sizeof(key), key_size);
	memcpy(&key[key_offset], params->key, key_size - key_offset);

	// NOTE: key_size is in bytes.
	status = prepare_key(key, key_size,
			     key_len, ECDSA_MODE_TO_USAGE(params->mode), alg,
			     ECC_FAMILY_TYPE(params->mode, type),
			     &key_handle);

	if (PSA_SUCCESS == status) {
		LOG_DBG("Key import success.");

		switch (params->mode) {
		case SID_PAL_CRYPTO_VERIFY:
			status = psa_verify_message(key_handle, alg,
						    params->in, params->in_size,
						    params->signature, params->sig_size);

			break;
		case SID_PAL_CRYPTO_SIGN:
		{
			size_t out_len;

			status = psa_sign_message(key_handle, alg,
						  params->in, params->in_size,
						  params->signature, params->sig_size,
						  &out_len);
		}
		break;
		default:
			return SID_ERROR_INVALID_ARGS;
		}

		if (PSA_SUCCESS != psa_destroy_key(key_handle)) {
			LOG_WRN("Destroy key failed!");
		}
	}

	return get_error(status);
}

sid_error_t sid_pal_crypto_ecc_ecdh(sid_pal_ecdh_params_t *params)
{
	psa_status_t status;
	psa_key_handle_t priv_key_handle;
	psa_ecc_family_t type;
	size_t key_len;
	uint8_t pub_key[EC_MAX_KEY_LENGTH];
	uint8_t pub_key_offset = 0;
	size_t pub_key_size = 0;

	if (!is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!params || !params->prk || !params->puk || !params->shared_secret) {
		return SID_ERROR_NULL_POINTER;
	}

	pub_key_size = params->puk_size;

	switch (params->algo) {
	case SID_PAL_ECDH_SECP256R1:
		type = PSA_ECC_FAMILY_SECP_R1;
		key_len = SECP256R1_KEY_LEN_BITS;

		// Sidewalk protocol has harcoded 64-byte public key length,
		// but PSA API required 65-byte public key length...
		memcpy(&pub_key[SECPxxx_PREFIX_OFFSET], secpxxx_key_prefix,
		       MIN(sizeof(pub_key), SECPxxx_KEY_PREFIX_LEN));
		pub_key_size += SECPxxx_KEY_PREFIX_LEN;
		pub_key_offset = SECPxxx_KEY_OFFSET;
		break;
	case SID_PAL_ECDH_CURVE25519:
		type = PSA_ECC_FAMILY_MONTGOMERY;
		key_len = CURVE25519_KEY_LEN_BITS;
		break;
	default:
		return SID_ERROR_NOSUPPORT;
	}

	// NOTE: params->prk_size and params->puk_size are in bytes.
	status = prepare_key(params->prk, params->prk_size,
			     key_len, PSA_KEY_USAGE_DERIVE, PSA_ALG_ECDH,
			     PSA_KEY_TYPE_ECC_KEY_PAIR(type),
			     &priv_key_handle);

	if (PSA_SUCCESS == status) {
		size_t out_len;

		LOG_DBG("Key import success.");

		pub_key_size = MIN(sizeof(pub_key), pub_key_size);
		memcpy(&pub_key[pub_key_offset], params->puk, pub_key_size - pub_key_offset);

		status = psa_raw_key_agreement(PSA_ALG_ECDH, priv_key_handle,
					       pub_key, pub_key_size,
					       params->shared_secret, params->shared_secret_sz,
					       &out_len);
	}

	if (PSA_SUCCESS != psa_destroy_key(priv_key_handle)) {
		LOG_WRN("Destroy key failed!");
	}

	return get_error(status);
}

sid_error_t sid_pal_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t *params)
{
	psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_algorithm_t alg;
	psa_key_type_t type;
	psa_status_t status;
	psa_key_handle_t keys_handle;
	size_t key_len;
	uint8_t pub_key_offset = 0;

	if (!is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!params || !params->prk || !params->puk) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!params->puk_size) {
		return SID_ERROR_INVALID_ARGS;
	}

	switch (params->algo) {
	case SID_PAL_EDDSA_ED25519:
		type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_TWISTED_EDWARDS);
		alg = PSA_ALG_ECDSA_ANY;
		key_len = ED25519_KEY_LEN_BITS;
		break;
	case SID_PAL_ECDSA_SECP256R1:
	case SID_PAL_ECDH_SECP256R1:
		type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
		alg = PSA_ALG_ECDSA_ANY;
		key_len = SECP256R1_KEY_LEN_BITS;
		/* The SECPxxx public key contains additional constatnt byte (prefix) on the buffer beginning,
		   to backward compatibility this one byte is skipped. */
		pub_key_offset = SECPxxx_KEY_OFFSET;
		break;
	case SID_PAL_ECDH_CURVE25519:
		type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_MONTGOMERY);
		alg = PSA_ALG_ECDH;
		key_len = CURVE25519_KEY_LEN_BITS;
		break;
	default:
		return SID_ERROR_NOSUPPORT;
	}

	// NOTE: params->prk_size and params->puk_size are in bytes.
	psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_EXPORT);
	psa_set_key_lifetime(&key_attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_algorithm(&key_attributes, alg);
	psa_set_key_type(&key_attributes, type);
	psa_set_key_bits(&key_attributes, key_len);

	status = psa_generate_key(&key_attributes, &keys_handle);
	if (PSA_SUCCESS == status) {
		size_t key_len;

		LOG_DBG("Key pair generated.");
		status = psa_export_key(keys_handle, params->prk, params->prk_size, &key_len);
		if (PSA_SUCCESS == status) {
			uint8_t public_key[EC_MAX_PUBLIC_KEY_LENGTH];

			LOG_DBG("Private key exported.");

			status = psa_export_public_key(keys_handle, public_key, sizeof(public_key), &key_len);
			memcpy(params->puk, &public_key[pub_key_offset], params->puk_size);

			LOG_DBG("Public key export %s", (PSA_SUCCESS == status) ? "success." : "failed!");
		}

		if (PSA_SUCCESS != psa_destroy_key(keys_handle)) {
			LOG_WRN("Destroy key failed!");
		}
	}
	psa_reset_key_attributes(&key_attributes);

	return get_error(status);
}
