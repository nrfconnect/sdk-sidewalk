/*
 * Copyright 2023 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#include "zephyr/sys/byteorder.h"
#include "zephyr/sys/util.h"
#include <sid_pal_log_ifc.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_on_dev_cert.h>
#include <sid_endian.h>

#include <stdlib.h>
#include <string.h>

#include <sys/_stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_dev_cert, CONFIG_SIDEWALK_LOG_LEVEL);

// Internal flags for tracking library state
#define ODC_STATE_FLG_SMSN 0x01
#define ODC_STATE_FLG_APP_KEY 0x02
#define ODC_STATE_FLG_ED25519_CSR 0x04
#define ODC_STATE_FLG_P256R1_CSR 0x08
#define ODC_STATE_FLG_ED25519_CHAIN 0x10
#define ODC_STATE_FLG_P256R1_CHAIN 0x20
#define ODC_STATE_MSK_FULL_INIT 0x3F

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
	uint8_t serial[SID_ODC_CA_SERIAL_MAX_SIZE];
};

// Sidewalk Certificate Authority (CA) for P256r1 curve
struct sid_on_dev_cert_ca_p256r1 {
	uint8_t pub_key[SID_ODC_P256R1_PUK_SIZE];
	uint8_t signature[SID_ODC_SIGNATURE_SIZE];
	uint16_t serial_length;
	uint8_t serial[SID_ODC_CA_SERIAL_MAX_SIZE];
};

struct sid_on_dev_cert_context {
	uint8_t state;
	uint8_t smsn[SID_ODC_SMSN_SIZE];
	uint8_t apid[SID_PAL_MFG_STORE_APID_SIZE];
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

static uint16_t sid_on_dev_cert_get_ca_serial_length(const uint8_t *serial)
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
	uint16_t head = sys_get_be16(serial);
	if ((head & 0x3000) == 0x3000) {
		return (head & 0x007f) + 4;
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

sid_error_t sid_on_dev_cert_init(void)
{
	if (!context) {
		context =
			k_heap_alloc(&cert_heap, sizeof(struct sid_on_dev_cert_context), K_NO_WAIT);

		if (!context) {
			return SID_ERROR_OOM;
		}
	}

	memset(context, 0xFF, sizeof(struct sid_on_dev_cert_context));

	for (int i = 0; i < CERT_MAX_NUM; i++) {
		context->cert_ed25519[i].serial_length = 0;
		context->cert_p256r1[i].serial_length = 0;
	}

	context->state = 0;

	return SID_ERROR_NONE;
}

void sid_on_dev_cert_deinit(void)
{
	if (context) {
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
	size_t board_id_len = dev_info->board_id ? strlen(dev_info->board_id) : 0;

	// Check input string length. Device Serial Number should be stored as a null terminated string.
	if (((dsn_len + 1) > SID_PAL_MFG_STORE_SERIAL_NUM_SIZE) ||
	    (apid_len != SID_PAL_MFG_STORE_APID_SIZE)) {
		return SID_ERROR_INVALID_ARGS;
	}

	uint8_t *m_buf = NULL;
	m_buf = k_heap_alloc(&cert_heap, dev_type_len + dsn_len + apid_len + board_id_len,
			     K_NO_WAIT);

	if (!m_buf) {
		return SID_ERROR_OOM;
	}

	/*
    * Generate SMSN:
    *   SMSN = SHA256(DMS-DeviceType, [DSN|unique serial], Advertised-Product-ID, BoardID[optional])
    */
	size_t idx = 0;
	memcpy(m_buf, dev_info->dev_type, dev_type_len);
	idx += dev_type_len;
	memcpy(&m_buf[idx], dev_info->dsn, dsn_len);
	idx += dsn_len;
	memcpy(&m_buf[idx], dev_info->apid, apid_len);
	idx += apid_len;
	if (dev_info->board_id) {
		memcpy(&m_buf[idx], dev_info->board_id, board_id_len);
		idx += board_id_len;
	}

	sid_pal_hash_params_t hash_params = { .algo = SID_PAL_HASH_SHA256,
					      .data = m_buf,
					      .data_size = idx,
					      .digest = smsn,
					      .digest_size = SID_ODC_SMSN_SIZE };

	sid_error_t ret = sid_pal_crypto_hash(&hash_params);
	if (ret == SID_ERROR_NONE) {
		memcpy(context->apid, dev_info->apid, sizeof(context->apid));
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
	if ((ret = sid_pal_crypto_ecc_key_gen(&key_params)) == SID_ERROR_NONE) {
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
				sid_on_dev_cert_get_ca_serial_length(pointer);
			if (context->cert_ed25519[i].serial_length == 0) {
				ret = SID_ERROR_INVALID_ARGS;
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
				sid_on_dev_cert_get_ca_serial_length(pointer);
			if (context->cert_p256r1[i].serial_length == 0) {
				ret = SID_ERROR_INVALID_ARGS;
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

	if (context->state != ODC_STATE_MSK_FULL_INIT) {
		return SID_ERROR_INVALID_STATE;
	}

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
	for (int i = 0; i < CERT_MAX_NUM; i++) {
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
		LOG_ERR("MFG version %ld is not supported", sid_htonl(mfg_version));
		ret = SID_ERROR_NOSUPPORT;
		goto exit;
	}

	bool status =
		write_to_mfg_store(SID_PAL_MFG_STORE_SMSN, context->smsn, SID_ODC_SMSN_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_APID, context->apid,
				   SID_PAL_MFG_STORE_APID_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_APP_PUB_ED25519, context->app_key,
				   SID_ODC_ED25519_PUK_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,
				   context->device_ed25519_prk, SID_ODC_ED25519_PRK_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PUB_ED25519,
				   context->device_ed25519_puk, SID_ODC_ED25519_PUK_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,
				   context->device_ed25519_sig, SID_ODC_SIGNATURE_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1, context->device_p256r1_prk,
				   SID_ODC_P256R1_PRK_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PUB_P256R1, context->device_p256r1_puk,
				   SID_ODC_P256R1_PUK_SIZE) &&
		write_to_mfg_store(SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,
				   context->device_p256r1_sig, SID_ODC_SIGNATURE_SIZE);

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

exit:
	return ret;
}
