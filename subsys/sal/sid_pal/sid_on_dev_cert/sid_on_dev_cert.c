/*
 * Copyright 2023-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <stdint.h>
#include <sid_on_dev_cert.h>
#include <sid_endian.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_rng.h>

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#include <sid_crypto_keys.h>
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <sid_mfg_hex_parsers.h>
#include <sid_mfg_storage.h>

LOG_MODULE_REGISTER(sid_dev_cert, CONFIG_SIDEWALK_LOG_LEVEL);

// Internal flags for tracking library state
// clang-format off
#define ODC_STATE_FLG_SMSN              0x01
#define ODC_STATE_FLG_APP_KEY           0x02
#define ODC_STATE_FLG_ED25519_CSR       0x04
#define ODC_STATE_FLG_P256R1_CSR        0x08
#define ODC_STATE_FLG_ED25519_CHAIN     0x10
#define ODC_STATE_FLG_P256R1_CHAIN      0x20
#define ODC_STATE_MSK_FULL_INIT         (ODC_STATE_FLG_SMSN | \
                                        ODC_STATE_FLG_ED25519_CSR | \
                                        ODC_STATE_FLG_P256R1_CSR | \
                                        ODC_STATE_FLG_ED25519_CHAIN | \
                                        ODC_STATE_FLG_P256R1_CHAIN)
// clang-format on

#define IS_STATE_FLG_SET(_FLAGS_) ((context->state & (_FLAGS_)) == (_FLAGS_))
#define IS_STATE_FLG_UNSET(_FLAGS_) ((context->state & (_FLAGS_)) == 0)

K_HEAP_DEFINE(cert_heap, KB(8));

enum sid_on_dev_cert_ca_id {
	CERT_DAK = 0,
	CERT_PRODUCT = 1,
	CERT_MAN = 2,
	CERT_SW = 3,
	CERT_AMZN = 4,
	CERT_MAX_NUM = 5,
};

// Sidewalk Certificate Authority (CA) for ED25519 curve
struct sid_on_dev_cert_ca_ed25519 {
	uint8_t pub_key[SID_ODC_ED25519_PUK_SIZE];
	uint8_t signature[SID_ODC_SIGNATURE_SIZE];
	uint16_t serial_length;
	uint8_t *serial;
};

// Sidewalk Certificate Authority (CA) for P256r1 curve
struct sid_on_dev_cert_ca_p256r1 {
	uint8_t pub_key[SID_ODC_P256R1_PUK_SIZE];
	uint8_t signature[SID_ODC_SIGNATURE_SIZE];
	uint16_t serial_length;
	uint8_t *serial;
};

struct sid_on_dev_cert_context {
	uint8_t state;
	uint8_t smsn[SID_ODC_SMSN_SIZE];
	uint8_t apid[SID_PAL_MFG_STORE_APID_SIZE];
	uint8_t dtid[SID_PAL_MFG_STORE_DTID_SIZE];
	uint8_t dtid_len;
	uint8_t app_key[SID_ODC_ED25519_PUK_SIZE];
	uint8_t device_ed25519_prk[SID_ODC_ED25519_PRK_SIZE];
	uint8_t device_ed25519_puk[SID_ODC_ED25519_PUK_SIZE];
	uint8_t device_ed25519_sig[SID_ODC_SIGNATURE_SIZE];
	uint8_t device_p256r1_prk[SID_ODC_P256R1_PRK_SIZE];
	uint8_t device_p256r1_puk[SID_ODC_P256R1_PUK_SIZE];
	uint8_t device_p256r1_sig[SID_ODC_SIGNATURE_SIZE];
	struct sid_on_dev_cert_ca_ed25519 cert_ed25519[CERT_MAX_NUM];
	struct sid_on_dev_cert_ca_p256r1 cert_p256r1[CERT_MAX_NUM];
};

static struct sid_on_dev_cert_context *context = NULL;

static bool write_to_mfg_store(uint16_t value, const uint8_t *const buffer, uint16_t length)
{
	uint8_t *read_buf = NULL;
	int32_t result = sid_pal_mfg_store_write(value, buffer, length);
	if (result) {
		goto err_out;
	}

	read_buf = k_heap_alloc(&cert_heap, length, K_NO_WAIT);
	if (!read_buf) {
		result = -1;
		goto err_out;
	}

	sid_pal_mfg_store_read(value, read_buf, length);
	if (memcmp(buffer, read_buf, length)) {
		result = -1;
		goto err_out;
	}

out:
	if (read_buf) {
		k_heap_free(&cert_heap, read_buf);
	}
	return result == 0;

err_out:
	LOG_ERR("Field not written to flash [%d][%d] res:%d", value, length, result);
	goto out;
}

static uint16_t sid_on_dev_cert_get_ca_serial_length(const uint8_t *serial, size_t available)
{
	/*
     * Certificate Serial Number format (standard 4-byte):
     *
     *   31 : 30         29 : 28             27 : 26             25 : 23             22 : 0
     *   2b              2b                  2b                  3b                  23b
     *   Type of         Certificate         Certificate's       Certificate's       Serial #
     *   certificate     intermediate        Intermediate        Signing rights
     *                   chain size          level
     *
     * Extended Certificate Serial Number format (2-bytes header + N-Bytes serial):
     *
     *   15 : 14         13 : 12             11 : 10             9 : 7               6 : 0
     *   2b              2b                  2b                  3b                  7b              8*X b
     *   Type of         Certificate         Certificate's       Certificate's       Length          Serial #
     *   certificate     intermediate        Intermediate        Signing rights
     *                   chain size          level
     *
     *   CLOUD - 02      0b11 = extended                                             Number of bytes
     *                   certificate                                                 to read for
     *                   chain                                                       the serial.
     *
     * Note: Standard 4-byte Certificate Serial Number is transmitted as little-endian byte sequence.
     * So, the Extended Certificate Serial Number must also have the first 4 bytes as little-endian.
     */
	if (available < SID_ODC_CA_SERIAL_MIN_SIZE) {
		// Not enough data to even read the serial header
		LOG_ERR("CA Serial truncated [%d]", (int)available);
		return 0;
	}

	if ((serial[3] & 0xF0) == 0xB0) {
		// Type of certificate = 2 (Cloud), Certificate intermediate chain = 0b11 (extended certificate chain)
		uint16_t length = serial[2] & 0x7F;
		length += 2; // get full serial length
		if (length < SID_ODC_CA_SERIAL_MIN_SIZE || length > SID_ODC_CA_SERIAL_MAX_SIZE) {
			LOG_ERR("CA Serial format error [%d][%d][%d][%d]", serial[0],
					  serial[1], serial[2], serial[3]);
			return 0;
		} else {
			return length;
		}
	}
	return SID_ODC_CA_SERIAL_MIN_SIZE;
}

static bool sid_on_dev_cert_ed25519_verify(const struct sid_on_dev_cert_ca_ed25519 *cert,
					   const uint8_t *puk)
{
	uint8_t hash[SID_ODC_ED25519_PUK_SIZE + SID_ODC_CA_SERIAL_MAX_SIZE];
	memcpy(hash, cert->pub_key, SID_ODC_ED25519_PUK_SIZE);
	memcpy(hash + SID_ODC_ED25519_PUK_SIZE, cert->serial, cert->serial_length);

	sid_pal_dsa_params_t params = {
		.algo = SID_PAL_EDDSA_ED25519,
		.mode = SID_PAL_CRYPTO_VERIFY,
		.key = puk,
		.key_size = SID_ODC_ED25519_PUK_SIZE,
		.in = hash,
		.in_size = cert->serial_length + SID_ODC_ED25519_PUK_SIZE,
		.signature = (uint8_t *)&cert->signature,
		.sig_size = SID_ODC_SIGNATURE_SIZE,
	};
	return (sid_pal_crypto_ecc_dsa(&params) == SID_ERROR_NONE);
}

static bool sid_on_dev_cert_p256r1_verify(const struct sid_on_dev_cert_ca_p256r1 *cert,
					  const uint8_t *puk)
{
	uint8_t hash[SID_ODC_P256R1_PUK_SIZE + SID_ODC_CA_SERIAL_MAX_SIZE];
	memcpy(hash, cert->pub_key, SID_ODC_P256R1_PUK_SIZE);
	memcpy(hash + SID_ODC_P256R1_PUK_SIZE, cert->serial, cert->serial_length);

	sid_pal_dsa_params_t params = {
		.algo = SID_PAL_ECDSA_SECP256R1,
		.mode = SID_PAL_CRYPTO_VERIFY,
		.key = puk,
		.key_size = SID_ODC_P256R1_PUK_SIZE,
		.in = hash,
		.in_size = cert->serial_length + SID_ODC_P256R1_PUK_SIZE,
		.signature = (uint8_t *)&cert->signature,
		.sig_size = SID_ODC_SIGNATURE_SIZE,
	};
	return (sid_pal_crypto_ecc_dsa(&params) == SID_ERROR_NONE);
}

/**
 * @brief  verify certs stored in context
 *
 * @param last_verify_cert argument defines max cert id to check.
 *          Note:
 *          In case certs is read from flash, AMZN cert contains only pub_key,
 *          that is why 'last_verify_cert' should be set to CERT_AMZN.
 *          In case writing Certificate Chain to the Device,
 *          all the fields contain valid data and therefore should be verified.
 *          That is why 'last_verify_cert' should be set to CERT_MAX_NUM.
 * @return sid_error_t
 */
static sid_error_t sid_on_dev_cert_verify(enum sid_on_dev_cert_ca_id last_verify_cert)
{
	sid_error_t ret = SID_ERROR_NONE;
	/*
     * Certificate chain:  Device <-- DAK <-- PRODUCT <-- Manufacturer <-- Sidewalk <-- AMZN
     * Each certificate signs and validates the next certificate in the chain.
     * In this AMZN public key would verify Sidewalk signature. Sidewalk public key would verify Manufacturer signature.
     * Manufacturer public key would verify PRODUCT and so on down to device
     */

	// Verify ED25519 and P256R1 Sidewalk Device Certificates
	uint8_t hash[SID_ODC_SMSN_SIZE + SID_ODC_P256R1_PUK_SIZE];
	sid_pal_dsa_params_t params = {
		.mode = SID_PAL_CRYPTO_VERIFY,
		.sig_size = SID_ODC_SIGNATURE_SIZE,
	};

	memcpy(hash, context->device_ed25519_puk, SID_ODC_ED25519_PUK_SIZE);
	memcpy(hash + SID_ODC_ED25519_PUK_SIZE, context->smsn, SID_ODC_SMSN_SIZE);

	params.algo = SID_PAL_EDDSA_ED25519;
	params.key = context->cert_ed25519[CERT_DAK].pub_key;
	params.key_size = SID_ODC_ED25519_PUK_SIZE;
	params.in = hash;
	params.in_size = SID_ODC_ED25519_PUK_SIZE + SID_ODC_SMSN_SIZE;
	params.signature = context->device_ed25519_sig;

	if (sid_pal_crypto_ecc_dsa(&params) != SID_ERROR_NONE) {
		LOG_ERR("Verify ED25519 Sidewalk Device Certificate failed");
		ret = SID_ERROR_GENERIC;
		goto exit;
	}

	memcpy(hash, context->device_p256r1_puk, SID_ODC_P256R1_PUK_SIZE);
	memcpy(hash + SID_ODC_P256R1_PUK_SIZE, context->smsn, SID_ODC_SMSN_SIZE);

	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.key = context->cert_p256r1[CERT_DAK].pub_key;
	params.key_size = SID_ODC_P256R1_PUK_SIZE;
	params.in = hash;
	params.in_size = SID_ODC_P256R1_PUK_SIZE + SID_ODC_SMSN_SIZE;
	params.signature = context->device_p256r1_sig;

	if (sid_pal_crypto_ecc_dsa(&params) != SID_ERROR_NONE) {
		LOG_ERR("Verify P256R1 Sidewalk Device Certificate failed");
		ret = SID_ERROR_GENERIC;
		goto exit;
	}
	// Verify ED25519 and P256R1 Sidewalk CAs
	for (int i = 0; i < last_verify_cert; i++) {
		// Note: Each certificate is signed with the key of the next certificate, except of self-signed Amazon Root CA
		if (!sid_on_dev_cert_ed25519_verify(
			    &context->cert_ed25519[i],
			    context->cert_ed25519[(i == CERT_AMZN) ? i : (i + 1)].pub_key)) {
			LOG_ERR("Verify ED25519 Sidewalk CA failed [%d]", i);
			ret = SID_ERROR_GENERIC;
			goto exit;
		}
		if (!sid_on_dev_cert_p256r1_verify(
			    &context->cert_p256r1[i],
			    context->cert_p256r1[(i == CERT_AMZN) ? i : (i + 1)].pub_key)) {
			LOG_ERR("Verify P256R1 Sidewalk CA failed [%d]", i);
			ret = SID_ERROR_GENERIC;
			goto exit;
		}
	}
exit:
	return ret;
}

static bool sid_on_dev_cert_read_ed2219_certs_data(sid_pal_mfg_store_value_t *value,
						   struct sid_on_dev_cert_ca_ed25519 *buffer)
{
	uint16_t mfg_value = (uint16_t)*value;
	sid_pal_mfg_store_read(mfg_value++, buffer->pub_key, SID_ODC_ED25519_PUK_SIZE);
	sid_pal_mfg_store_read(mfg_value++, buffer->signature, SID_ODC_SIGNATURE_SIZE);
	buffer->serial_length = sid_pal_mfg_store_get_length_for_value(mfg_value);
	if (!buffer->serial_length) {
		goto ret_error;
	}
	if (buffer->serial != NULL) {
		k_heap_free(&cert_heap, buffer->serial);
	}
	buffer->serial = k_heap_alloc(&cert_heap, buffer->serial_length, K_NO_WAIT);
	if (buffer->serial == NULL) {
		goto ret_error;
	}
	sid_pal_mfg_store_read(mfg_value++, buffer->serial, buffer->serial_length);
	*value = (sid_pal_mfg_store_value_t)mfg_value;
	return false;
ret_error:
	return true;
}

static bool sid_on_dev_cert_read_p256r1_certs_data(sid_pal_mfg_store_value_t *value,
						   struct sid_on_dev_cert_ca_p256r1 *buffer)
{
	uint16_t mfg_value = (uint16_t)*value;
	sid_pal_mfg_store_read(mfg_value++, buffer->pub_key, SID_ODC_P256R1_PUK_SIZE);
	sid_pal_mfg_store_read(mfg_value++, buffer->signature, SID_ODC_SIGNATURE_SIZE);
	buffer->serial_length = sid_pal_mfg_store_get_length_for_value(mfg_value);
	if (!buffer->serial_length) {
		goto ret_error;
	}
	if (buffer->serial != NULL) {
		k_heap_free(&cert_heap, buffer->serial);
	}
	buffer->serial = k_heap_alloc(&cert_heap, buffer->serial_length, K_NO_WAIT);
	if (buffer->serial == NULL) {
		goto ret_error;
	}
	sid_pal_mfg_store_read(mfg_value++, buffer->serial, buffer->serial_length);
	*value = (sid_pal_mfg_store_value_t)mfg_value;
	return false;
ret_error:
	return true;
}

static void sid_on_dev_cert_free_allocated_mem_for_serials(void)
{
	for (enum sid_on_dev_cert_ca_id id = CERT_DAK; id < CERT_MAX_NUM; id++) {
		if (context->cert_ed25519[id].serial != NULL) {
			k_heap_free(&cert_heap, context->cert_ed25519[id].serial);
			context->cert_ed25519[id].serial = NULL;
		}
		if (context->cert_p256r1[id].serial != NULL) {
			k_heap_free(&cert_heap, context->cert_p256r1[id].serial);
			context->cert_p256r1[id].serial = NULL;
		}
	}
}

// check each byte of the array is != 0xff && != 0x00
static bool array_is_empty(const uint8_t *array, size_t arr_size)
{
	bool res_val = false;
	size_t orig_arr_size = arr_size, zeros = 0, ffs = 0;

	while (arr_size--) {
		zeros += (*array == 0x00) ? 1 : 0;
		ffs += (*array == UINT8_MAX) ? 1 : 0;
		++array;
	}
	if ((zeros == orig_arr_size) || (ffs == orig_arr_size)) {
		res_val = true;
	}

	return res_val;
}

static sid_error_t sign_verify_user_string(sid_pal_dsa_mode_t mode, sid_pal_ecc_algo_t algo,
					   uint8_t *in_data, size_t data_size, uint8_t *signature)
{
	sid_pal_dsa_params_t dsa_params = {
		.algo = algo,
		.mode = mode,
		.in = in_data,
		.in_size = data_size,
		.signature = signature,
		.sig_size = SID_ODC_SIGNATURE_SIZE,
	};

	switch (algo) {
	case SID_PAL_ECDSA_SECP256R1:
		dsa_params.key = (mode == SID_PAL_CRYPTO_SIGN) ? context->device_p256r1_prk :
								 context->device_p256r1_puk;
		dsa_params.key_size = (mode == SID_PAL_CRYPTO_SIGN) ? SID_ODC_P256R1_PRK_SIZE :
								      SID_ODC_P256R1_PUK_SIZE;
		break;
	case SID_PAL_EDDSA_ED25519:
		dsa_params.key = (mode == SID_PAL_CRYPTO_SIGN) ? context->device_ed25519_prk :
								 context->device_ed25519_puk;
		dsa_params.key_size = (mode == SID_PAL_CRYPTO_SIGN) ? SID_ODC_ED25519_PRK_SIZE :
								      SID_ODC_ED25519_PUK_SIZE;
		break;
	default:
		return SID_ERROR_INVALID_ARGS;
	}

	return sid_pal_crypto_ecc_dsa(&dsa_params);
}

static sid_error_t sign_verify_key_pair(sid_pal_ecc_algo_t algo, uint8_t *in_data, size_t data_size,
					uint8_t *signature)
{
	sid_error_t ret_val =
		sign_verify_user_string(SID_PAL_CRYPTO_SIGN, algo, in_data, data_size, signature);

	if (ret_val != SID_ERROR_NONE) {
		LOG_ERR("Error user signature generation! Algo: %d", algo);
		goto exit;
	}

	ret_val =
		sign_verify_user_string(SID_PAL_CRYPTO_VERIFY, algo, in_data, data_size, signature);

	if (ret_val != SID_ERROR_NONE) {
		LOG_ERR("Error verification of user signature! Algo: %d", algo);
		goto exit;
	}
exit:
	return ret_val;
}

#define USER_STRING_SIZE UINT8_MAX
static sid_error_t check_key_pairs(void)
{
	uint8_t random_string[USER_STRING_SIZE];
	uint8_t sign_arr[SID_ODC_SIGNATURE_SIZE];
	sid_error_t ret_val = SID_ERROR_NONE;

	if ((ret_val = sid_rng_init()) != SID_ERROR_NONE) {
		goto exit;
	}

	for (uint8_t i = 0; i < USER_STRING_SIZE; ++i) {
		random_string[i] = (uint8_t)sid_rng_get_range_crypto_safe(UINT8_MAX + 1);
	}

	if ((ret_val = sign_verify_key_pair(SID_PAL_ECDSA_SECP256R1, random_string,
					    USER_STRING_SIZE, sign_arr)) != SID_ERROR_NONE) {
		goto exit;
	}

	if ((ret_val = sign_verify_key_pair(SID_PAL_EDDSA_ED25519, random_string, USER_STRING_SIZE,
					    sign_arr)) != SID_ERROR_NONE) {
		goto exit;
	}
	LOG_INF("Key pair was verified successfully.");

exit:
	return ret_val;
}

sid_error_t sid_on_dev_cert_status_check(void)
{
	sid_error_t ret_val = SID_ERROR_NONE;

	if (!context) {
		return SID_ERROR_UNINITIALIZED;
	}
	// read whole data from flash space and place it to context
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SMSN, context->smsn, SID_ODC_SMSN_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_APID, context->apid, SID_PAL_MFG_STORE_APID_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_APP_PUB_ED25519, context->app_key,
			       SID_ODC_ED25519_PUK_SIZE);

	if (array_is_empty(context->apid, SID_PAL_MFG_STORE_APID_SIZE)) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}

	size_t dtid_len = sid_pal_mfg_store_get_length_for_value(SID_PAL_MFG_STORE_DTID);
	if (dtid_len == 0 || dtid_len > SID_PAL_MFG_STORE_DTID_SIZE) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DTID, context->dtid, dtid_len);

	// GW does not support app key, skip the check
#if !defined(GW_SUPPORT) || !GW_SUPPORT
	if (array_is_empty(context->app_key, SID_ODC_ED25519_PUK_SIZE)) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}
#endif

	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519, context->device_ed25519_prk,
			       SID_ODC_ED25519_PRK_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVICE_PUB_ED25519, context->device_ed25519_puk,
			       SID_ODC_ED25519_PUK_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,
			       context->device_ed25519_sig, SID_ODC_SIGNATURE_SIZE);

	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1, context->device_p256r1_prk,
			       SID_ODC_P256R1_PRK_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVICE_PUB_P256R1, context->device_p256r1_puk,
			       SID_ODC_P256R1_PUK_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,
			       context->device_p256r1_sig, SID_ODC_SIGNATURE_SIZE);

	sid_pal_mfg_store_value_t mfg_storage_value = SID_PAL_MFG_STORE_DAK_PUB_ED25519;
	for (enum sid_on_dev_cert_ca_id id = CERT_DAK; id < CERT_AMZN; id++) {
		if (sid_on_dev_cert_read_ed2219_certs_data(&mfg_storage_value,
							   &context->cert_ed25519[id]) ||
		    sid_on_dev_cert_read_p256r1_certs_data(&mfg_storage_value,
							   &context->cert_p256r1[id])) {
			ret_val = SID_ERROR_STORAGE_READ_FAIL;
			goto exit;
		}
	}
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_AMZN_PUB_ED25519,
			       context->cert_ed25519[CERT_AMZN].pub_key, SID_ODC_ED25519_PUK_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_AMZN_PUB_P256R1,
			       context->cert_p256r1[CERT_AMZN].pub_key, SID_ODC_P256R1_PUK_SIZE);

	ret_val = sid_on_dev_cert_verify(CERT_AMZN);
	if (ret_val != SID_ERROR_NONE) {
		goto exit;
	}

	if ((ret_val = check_key_pairs()) != SID_ERROR_NONE) {
		goto exit;
	}

exit:
	// despite errors clean-up context
	sid_on_dev_cert_free_allocated_mem_for_serials();
	memset(context, 0, sizeof(struct sid_on_dev_cert_context));
	return ret_val;
}

sid_error_t sid_on_dev_cert_read_cert_values(enum sid_on_dev_cert_print_fileds info_num,
					     uint8_t *data, size_t *data_size)
{
	uint16_t read_data_field = 0;

	if (!data || !data_size) {
		return SID_ERROR_NULL_POINTER;
	}

	switch (info_num) {
	case SID_ODC_PRINT_APID:
		read_data_field = SID_PAL_MFG_STORE_APID;
		*data_size = SID_PAL_MFG_STORE_APID_SIZE;
		break;
	case SID_ODC_PRINT_APP_KEY:
		read_data_field = SID_PAL_MFG_STORE_APP_PUB_ED25519;
		*data_size = SID_ODC_ED25519_PUK_SIZE;
		break;
	case SID_ODC_PRINT_DTID:
		read_data_field = SID_PAL_MFG_STORE_DTID;
		*data_size = sid_pal_mfg_store_get_length_for_value(SID_PAL_MFG_STORE_DTID);
		if (*data_size == 0 || *data_size > SID_PAL_MFG_STORE_DTID_SIZE) {
			return SID_ERROR_PARAM_OUT_OF_RANGE;
		}
		break;
	default:
		// extra "data read fail" indicator;
		*data_size = 0;
		return SID_ERROR_INVALID_ARGS;
	}
	sid_pal_mfg_store_read(read_data_field, data, *data_size);
	return SID_ERROR_NONE;
}

sid_error_t sid_on_dev_cert_init(void)
{
	if (!context) {
		context =
			k_heap_alloc(&cert_heap, sizeof(struct sid_on_dev_cert_context), K_NO_WAIT);
		if (!context) {
			return SID_ERROR_OOM;
		}
	} else {
		sid_on_dev_cert_free_allocated_mem_for_serials();
	}

	memset(context, 0, sizeof(struct sid_on_dev_cert_context));

	return SID_ERROR_NONE;
}

void sid_on_dev_cert_deinit(void)
{
	if (context) {
		sid_on_dev_cert_free_allocated_mem_for_serials();
		k_heap_free(&cert_heap, context);
		context = NULL;
	}

	return;
}

sid_error_t sid_on_dev_cert_generate_smsn(const struct sid_on_dev_cert_info *dev_info,
					  uint8_t *smsn)
{
	if (!dev_info || !smsn || !dev_info->dev_type || !dev_info->dsn || !dev_info->apid) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!context) {
		return SID_ERROR_UNINITIALIZED;
	}

	// We must reset all flags when generating SMSN
	context->state = 0;

	size_t dev_type_len = strlen(dev_info->dev_type);
	size_t dsn_len = strlen(dev_info->dsn);
	size_t apid_len = strlen(dev_info->apid);

	// Check if suffix is present and calculate actual device type length
	const char *suffix = "-PRODUCTION";
	size_t suffix_len = strlen(suffix);
	size_t actual_dev_type_len = dev_type_len;

	if (dev_type_len >= suffix_len &&
	    strcmp(dev_info->dev_type + dev_type_len - suffix_len, suffix) == 0) {
		actual_dev_type_len = dev_type_len - suffix_len;
	}

	// Check input string length. Device Serial Number should be stored as a null terminated string.
	if (((dsn_len + 1) > SID_PAL_MFG_STORE_SERIAL_NUM_SIZE) ||
	    (apid_len != SID_PAL_MFG_STORE_APID_SIZE) ||
	    actual_dev_type_len > SID_PAL_MFG_STORE_DTID_SIZE) {
		return SID_ERROR_INVALID_ARGS;
	}

	uint8_t *m_buf = NULL;
	m_buf = k_heap_alloc(&cert_heap, dev_type_len + dsn_len + apid_len, K_NO_WAIT);
	if (!m_buf) {
		return SID_ERROR_OOM;
	}

	/*
     * Generate SMSN:
     *   SMSN = SHA256(DMS-DeviceType, [DSN|unique serial], Advertised-Product-ID)
     */
	size_t idx = 0;
	memcpy(m_buf, dev_info->dev_type, dev_type_len);
	idx += dev_type_len;
	memcpy(&m_buf[idx], dev_info->dsn, dsn_len);
	idx += dsn_len;
	memcpy(&m_buf[idx], dev_info->apid, apid_len);
	idx += apid_len;

	sid_pal_hash_params_t hash_params = { .algo = SID_PAL_HASH_SHA256,
					      .data = m_buf,
					      .data_size = idx,
					      .digest = smsn,
					      .digest_size = SID_ODC_SMSN_SIZE };

	sid_error_t ret = sid_pal_crypto_hash(&hash_params);
	if (ret == SID_ERROR_NONE) {
		memcpy(context->apid, dev_info->apid, sizeof(context->apid));
		// The DeviceType is stored without the '-PRODUCTION' suffix.
		memcpy(context->dtid, dev_info->dev_type, actual_dev_type_len);
		context->dtid_len = (uint8_t)actual_dev_type_len;
		memcpy(context->smsn, smsn, SID_ODC_SMSN_SIZE);
		context->state |= ODC_STATE_FLG_SMSN;
	}

	k_heap_free(&cert_heap, m_buf);

	return ret;
}

sid_error_t sid_on_dev_cert_generate_csr(enum sid_on_dev_cert_algo_type algo, uint8_t *csr,
					 size_t *csr_size)
{
	if (!csr || !csr_size) {
		return SID_ERROR_NULL_POINTER;
	}

	sid_error_t ret = SID_ERROR_NONE;
	size_t csr_in_size = *csr_size;
	*csr_size = 0;

	if (!context) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (IS_STATE_FLG_UNSET(ODC_STATE_FLG_SMSN)) {
		return SID_ERROR_INVALID_STATE;
	}

	sid_pal_ecc_key_gen_params_t key_params = {};

	if (algo == SID_ODC_CRYPT_ALGO_ED25519) {
		// If the user regenerates the CSR, then we must clear the corresponding flags
		context->state &= ~ODC_STATE_FLG_ED25519_CSR;
		context->state &= ~ODC_STATE_FLG_ED25519_CHAIN;
		key_params.algo = SID_PAL_EDDSA_ED25519;
		key_params.puk = context->device_ed25519_puk;
		key_params.puk_size = sizeof(context->device_ed25519_puk);
		key_params.prk = context->device_ed25519_prk;
		key_params.prk_size = sizeof(context->device_ed25519_prk);

	} else if (algo == SID_ODC_CRYPT_ALGO_P256R1) {
		// If the user regenerates the CSR, then we must clear the corresponding flags
		context->state &= ~ODC_STATE_FLG_P256R1_CSR;
		context->state &= ~ODC_STATE_FLG_P256R1_CHAIN;
		key_params.algo = SID_PAL_ECDSA_SECP256R1;
		key_params.puk = context->device_p256r1_puk;
		key_params.puk_size = sizeof(context->device_p256r1_puk);
		key_params.prk = context->device_p256r1_prk;
		key_params.prk_size = sizeof(context->device_p256r1_prk);
	} else {
		return SID_ERROR_INCOMPATIBLE_PARAMS;
	}

	// Generate key pair
#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	ret = SID_ERROR_GENERIC;
	psa_key_id_t key_id = (algo == SID_ODC_CRYPT_ALGO_ED25519) ?
				      SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID :
				      SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID;
	int err = sid_crypto_keys_new_generate(key_id, key_params.puk, key_params.puk_size);
	if (!err) {
		err = sid_crypto_keys_buffer_set(key_id, key_params.prk, key_params.prk_size);
		if (!err) {
			ret = SID_ERROR_NONE;
		}
	}
#else
	ret = sid_pal_crypto_ecc_key_gen(&key_params);
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */
	if (ret == SID_ERROR_NONE) {
		/*
         * Signing the certificate CSR
         * Message_to_sign = Public key || SMSN
         * Then signing using the generates private key
         * Signature = sign(Private Key, Message_to_sign)
         * The returned CSR:
         * CSR = Public Key || SMSN || Signature
         */
		size_t csr_out_size =
			key_params.puk_size + SID_ODC_SMSN_SIZE + SID_ODC_SIGNATURE_SIZE;
		if (csr_in_size < csr_out_size) {
			return SID_ERROR_INVALID_ARGS;
		}

		size_t idx = 0;
		memcpy(&csr[idx], key_params.puk, key_params.puk_size);
		idx += key_params.puk_size;
		memcpy(&csr[idx], context->smsn, SID_ODC_SMSN_SIZE);
		idx += SID_ODC_SMSN_SIZE;

		sid_pal_dsa_params_t params = {
			.algo = (algo == SID_ODC_CRYPT_ALGO_ED25519) ? SID_PAL_EDDSA_ED25519 :
								       SID_PAL_ECDSA_SECP256R1,
			.mode = SID_PAL_CRYPTO_SIGN,
			.key = key_params.prk,
			.key_size = key_params.prk_size,
			.in = csr,
			.in_size = idx,
			.signature = csr + idx,
			.sig_size = SID_ODC_SIGNATURE_SIZE,
		};

		if ((ret = sid_pal_crypto_ecc_dsa(&params)) == SID_ERROR_NONE) {
			// pass to the caller actual CSR size
			*csr_size = csr_out_size;
		}
	}

	if (ret == SID_ERROR_NONE) {
		if (algo == SID_ODC_CRYPT_ALGO_ED25519) {
			context->state |= ODC_STATE_FLG_ED25519_CSR;
		} else {
			context->state |= ODC_STATE_FLG_P256R1_CSR;
		}
	}

	return ret;
}

sid_error_t
sid_on_dev_cert_write_cert_chain(const struct sid_on_dev_cert_chain_params *chain_params)
{
	if (!chain_params || !chain_params->cert_chain) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!context) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (chain_params->algo == SID_ODC_CRYPT_ALGO_ED25519) {
		// We must clear flag if the user rewrites chain
		context->state &= ~ODC_STATE_FLG_ED25519_CHAIN;
		if (IS_STATE_FLG_UNSET(ODC_STATE_FLG_ED25519_CSR)) {
			return SID_ERROR_INVALID_STATE;
		}
		if (chain_params->cert_chain_size < SID_ODC_ED25519_SCC_MIN_SIZE ||
		    chain_params->cert_chain_size > SID_ODC_ED25519_SCC_MAX_SIZE) {
			return SID_ERROR_INVALID_ARGS;
		}
	} else if (chain_params->algo == SID_ODC_CRYPT_ALGO_P256R1) {
		// We must clear flag if the user rewrites chain
		context->state &= ~ODC_STATE_FLG_P256R1_CHAIN;
		if (IS_STATE_FLG_UNSET(ODC_STATE_FLG_P256R1_CSR)) {
			return SID_ERROR_INVALID_STATE;
		}
		if (chain_params->cert_chain_size < SID_ODC_P256R1_SCC_MIN_SIZE ||
		    chain_params->cert_chain_size > SID_ODC_P256R1_SCC_MAX_SIZE) {
			return SID_ERROR_INVALID_ARGS;
		}
	} else {
		return SID_ERROR_INCOMPATIBLE_PARAMS;
	}

	sid_error_t ret = SID_ERROR_NONE;

	uint8_t *pointer = chain_params->cert_chain;
	uint8_t *pointer_end = pointer + chain_params->cert_chain_size;

	// Check SMSN in certificate chain. It must match the SMSN generated earlier.
	if (memcmp(context->smsn, pointer, SID_ODC_SMSN_SIZE) != 0) {
		ret = SID_ERROR_INVALID_ARGS;
		goto exit;
	}

	pointer += SID_ODC_SMSN_SIZE;

	if (chain_params->algo == SID_ODC_CRYPT_ALGO_ED25519) {
		// Check public key in certificate chain. It must match the key generated earlier.
		if (memcmp(context->device_ed25519_puk, pointer,
			   sizeof(context->device_ed25519_puk)) != 0) {
			ret = SID_ERROR_INVALID_ARGS;
			goto exit;
		}

		pointer += SID_ODC_ED25519_PUK_SIZE;
		memcpy(context->device_ed25519_sig, pointer, sizeof(context->device_ed25519_sig));
		pointer += SID_ODC_SIGNATURE_SIZE;

		// Store ED25519 Sidewalk CAs in memory
		for (int i = 0; i < CERT_MAX_NUM; i++) {
			context->cert_ed25519[i].serial_length =
				sid_on_dev_cert_get_ca_serial_length(pointer,
								     pointer_end - pointer);
			if (context->cert_ed25519[i].serial_length == 0) {
				ret = SID_ERROR_INVALID_ARGS;
				goto exit;
			}
			if (context->cert_ed25519[i].serial != NULL) {
				k_heap_free(&cert_heap, context->cert_ed25519[i].serial);
			}
			context->cert_ed25519[i].serial = k_heap_alloc(
				&cert_heap, context->cert_ed25519[i].serial_length, K_NO_WAIT);
			if (context->cert_ed25519[i].serial == NULL) {
				ret = SID_ERROR_OOM;
				goto exit;
			}
			// check for memory overlap
			if ((pointer + context->cert_ed25519[i].serial_length +
			     SID_ODC_ED25519_PUK_SIZE + SID_ODC_SIGNATURE_SIZE) > pointer_end) {
				ret = SID_ERROR_INVALID_ARGS;
				goto exit;
			}
			memcpy(context->cert_ed25519[i].serial, pointer,
			       context->cert_ed25519[i].serial_length);
			pointer += context->cert_ed25519[i].serial_length;
			memcpy(context->cert_ed25519[i].pub_key, pointer, SID_ODC_ED25519_PUK_SIZE);
			pointer += SID_ODC_ED25519_PUK_SIZE;
			memcpy(context->cert_ed25519[i].signature, pointer, SID_ODC_SIGNATURE_SIZE);
			pointer += SID_ODC_SIGNATURE_SIZE;
		}

		context->state |= ODC_STATE_FLG_ED25519_CHAIN;
	} else {
		// Check public key in certificate chain. It must match the key generated earlier.
		if (memcmp(context->device_p256r1_puk, pointer,
			   sizeof(context->device_p256r1_puk)) != 0) {
			ret = SID_ERROR_INVALID_ARGS;
			goto exit;
		}

		pointer += SID_ODC_P256R1_PUK_SIZE;
		memcpy(context->device_p256r1_sig, pointer, sizeof(context->device_p256r1_sig));
		pointer += SID_ODC_SIGNATURE_SIZE;

		// Store P256R1 Sidewalk CAs in memory
		for (int i = 0; i < CERT_MAX_NUM; i++) {
			context->cert_p256r1[i].serial_length =
				sid_on_dev_cert_get_ca_serial_length(pointer,
								     pointer_end - pointer);
			if (context->cert_p256r1[i].serial_length == 0) {
				ret = SID_ERROR_INVALID_ARGS;
				goto exit;
			}
			if (context->cert_p256r1[i].serial != NULL) {
				k_heap_free(&cert_heap, context->cert_p256r1[i].serial);
			}
			context->cert_p256r1[i].serial = k_heap_alloc(
				&cert_heap, context->cert_p256r1[i].serial_length, K_NO_WAIT);
			if (context->cert_p256r1[i].serial == NULL) {
				ret = SID_ERROR_OOM;
				goto exit;
			}
			// check for memory overlap
			if ((pointer + context->cert_p256r1[i].serial_length +
			     SID_ODC_P256R1_PUK_SIZE + SID_ODC_SIGNATURE_SIZE) > pointer_end) {
				ret = SID_ERROR_INVALID_ARGS;
				goto exit;
			}
			memcpy(context->cert_p256r1[i].serial, pointer,
			       context->cert_p256r1[i].serial_length);
			pointer += context->cert_p256r1[i].serial_length;
			memcpy(context->cert_p256r1[i].pub_key, pointer, SID_ODC_P256R1_PUK_SIZE);
			pointer += SID_ODC_P256R1_PUK_SIZE;
			memcpy(context->cert_p256r1[i].signature, pointer, SID_ODC_SIGNATURE_SIZE);
			pointer += SID_ODC_SIGNATURE_SIZE;
		}

		context->state |= ODC_STATE_FLG_P256R1_CHAIN;
	}

exit:
	return ret;
}

sid_error_t sid_on_dev_cert_write_app_server_key(const uint8_t *app_key)
{
	if (!app_key) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!context) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (IS_STATE_FLG_SET(ODC_STATE_FLG_APP_KEY)) {
		return SID_ERROR_INVALID_STATE;
	}

	memcpy(context->app_key, app_key, sizeof(context->app_key));

	context->state |= ODC_STATE_FLG_APP_KEY;

	return SID_ERROR_NONE;
}

sid_error_t sid_on_dev_cert_verify_and_store(void)
{
	if (!context) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (!IS_STATE_FLG_SET(ODC_STATE_MSK_FULL_INIT)) {
		return SID_ERROR_INVALID_STATE;
	}

	sid_error_t ret = sid_on_dev_cert_verify(CERT_MAX_NUM);
	if (ret != SID_ERROR_NONE) {
		goto exit;
	}

	int32_t result = sid_pal_mfg_store_erase();
	if (result) {
		LOG_ERR("MFG erase failed [%d]", result);
		ret = SID_ERROR_STORAGE_ERASE_FAIL;
		goto exit;
	}

	/*
     * First of all, we need to write the MFG version.
     * This will determine the storage format: static offsets or TLV
     */
	uint32_t mfg_version;

	if (sid_pal_mfg_store_is_tlv_support()) {
		mfg_version = sid_htonl(SID_PAL_MFG_STORE_TLV_VERSION);
	} else {
		mfg_version = sid_htonl(SID_PAL_MFG_STORE_FIXED_OFFSETS_VERSION);
	}

	result = sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, (uint8_t *)&mfg_version,
					 SID_PAL_MFG_STORE_VERSION_SIZE);

	if (result) {
		LOG_ERR("MFG version %u is not supported", (unsigned int)sid_ntohl(mfg_version));
		ret = SID_ERROR_NOSUPPORT;
		goto exit;
	}

	bool status =
		write_to_mfg_store(SID_PAL_MFG_STORE_SMSN, context->smsn, SID_ODC_SMSN_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_APID, context->apid,
				   SID_PAL_MFG_STORE_APID_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DTID, context->dtid, context->dtid_len)
#if !CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
		&& write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,
				      context->device_ed25519_prk, SID_ODC_ED25519_PRK_SIZE)
#endif
		&& write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PUB_ED25519,
				      context->device_ed25519_puk, SID_ODC_ED25519_PUK_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,
				   context->device_ed25519_sig, SID_ODC_SIGNATURE_SIZE)
#if !CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
		&& write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1,
				      context->device_p256r1_prk, SID_ODC_P256R1_PRK_SIZE)
#endif
		&& write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PUB_P256R1,
				      context->device_p256r1_puk, SID_ODC_P256R1_PUK_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,
				   context->device_p256r1_sig, SID_ODC_SIGNATURE_SIZE);

	if (IS_STATE_FLG_SET(ODC_STATE_FLG_APP_KEY)) {
		status &= write_to_mfg_store(SID_PAL_MFG_STORE_APP_PUB_ED25519, context->app_key,
					     SID_ODC_ED25519_PUK_SIZE);
	}
	if (!status) {
		ret = SID_ERROR_STORAGE_WRITE_FAIL;
		goto exit;
	}

	status = write_to_mfg_store(SID_PAL_MFG_STORE_DAK_PUB_ED25519,
				    context->cert_ed25519[CERT_DAK].pub_key,
				    SID_ODC_ED25519_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE,
				    context->cert_ed25519[CERT_DAK].signature,
				    SID_ODC_SIGNATURE_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_DAK_ED25519_SERIAL,
				    context->cert_ed25519[CERT_DAK].serial,
				    context->cert_ed25519[CERT_DAK].serial_length)

		 && write_to_mfg_store(SID_PAL_MFG_STORE_DAK_PUB_P256R1,
				       context->cert_p256r1[CERT_DAK].pub_key,
				       SID_ODC_P256R1_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE,
				    context->cert_p256r1[CERT_DAK].signature,
				    SID_ODC_SIGNATURE_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_DAK_P256R1_SERIAL,
				    context->cert_p256r1[CERT_DAK].serial,
				    context->cert_p256r1[CERT_DAK].serial_length)

		 && write_to_mfg_store(SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519,
				       context->cert_ed25519[CERT_PRODUCT].pub_key,
				       SID_ODC_ED25519_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE,
				    context->cert_ed25519[CERT_PRODUCT].signature,
				    SID_ODC_SIGNATURE_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL,
				    context->cert_ed25519[CERT_PRODUCT].serial,
				    context->cert_ed25519[CERT_PRODUCT].serial_length) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1,
				    context->cert_p256r1[CERT_PRODUCT].pub_key,
				    SID_ODC_P256R1_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE,
				    context->cert_p256r1[CERT_PRODUCT].signature,
				    SID_ODC_SIGNATURE_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL,
				    context->cert_p256r1[CERT_PRODUCT].serial,
				    context->cert_p256r1[CERT_PRODUCT].serial_length)

		 && write_to_mfg_store(SID_PAL_MFG_STORE_MAN_PUB_ED25519,
				       context->cert_ed25519[CERT_MAN].pub_key,
				       SID_ODC_ED25519_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE,
				    context->cert_ed25519[CERT_MAN].signature,
				    SID_ODC_SIGNATURE_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_MAN_ED25519_SERIAL,
				    context->cert_ed25519[CERT_MAN].serial,
				    context->cert_ed25519[CERT_MAN].serial_length) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_MAN_PUB_P256R1,
				    context->cert_p256r1[CERT_MAN].pub_key,
				    SID_ODC_P256R1_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE,
				    context->cert_p256r1[CERT_MAN].signature,
				    SID_ODC_SIGNATURE_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_MAN_P256R1_SERIAL,
				    context->cert_p256r1[CERT_MAN].serial,
				    context->cert_p256r1[CERT_MAN].serial_length)

		 && write_to_mfg_store(SID_PAL_MFG_STORE_SW_PUB_ED25519,
				       context->cert_ed25519[CERT_SW].pub_key,
				       SID_ODC_ED25519_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE,
				    context->cert_ed25519[CERT_SW].signature,
				    SID_ODC_SIGNATURE_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_SW_ED25519_SERIAL,
				    context->cert_ed25519[CERT_SW].serial,
				    context->cert_ed25519[CERT_SW].serial_length) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_SW_PUB_P256R1,
				    context->cert_p256r1[CERT_SW].pub_key,
				    SID_ODC_P256R1_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE,
				    context->cert_p256r1[CERT_SW].signature,
				    SID_ODC_SIGNATURE_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_SW_P256R1_SERIAL,
				    context->cert_p256r1[CERT_SW].serial,
				    context->cert_p256r1[CERT_SW].serial_length)

		 && write_to_mfg_store(SID_PAL_MFG_STORE_AMZN_PUB_ED25519,
				       context->cert_ed25519[CERT_AMZN].pub_key,
				       SID_ODC_ED25519_PUK_SIZE) &&
		 write_to_mfg_store(SID_PAL_MFG_STORE_AMZN_PUB_P256R1,
				    context->cert_p256r1[CERT_AMZN].pub_key,
				    SID_ODC_P256R1_PUK_SIZE);

	if (!status) {
		ret = SID_ERROR_STORAGE_WRITE_FAIL;
		goto exit;
	}

	struct mfg_flags flags = {
		.initialized = 1,
#if CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
		.keys_in_psa = 1,
#endif
	};
	status = write_to_mfg_store(MFG_FLAGS_TYPE_ID, (uint8_t *)&flags, sizeof(flags));
	if (!status) {
		ret = SID_ERROR_STORAGE_WRITE_FAIL;
		goto exit;
	}

	result = sid_mfg_storage_flush();
	if (result) {
		LOG_ERR("MFG flush failed [%d]", result);
		ret = SID_ERROR_STORAGE_WRITE_FAIL;
		goto exit;
	}

exit:
	return ret;
}
