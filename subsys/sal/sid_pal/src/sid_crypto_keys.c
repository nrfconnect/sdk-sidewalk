/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_error.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_crypto_keys.h>
#include <errno.h>
#include <zephyr/logging/log.h>
#include <json_printer/sidTypes2str.h>

LOG_MODULE_REGISTER(sid_crypto_key, CONFIG_SIDEWALK_CRYPTO_LOG_LEVEL);
#define ESUCCESS (0)
#define MAX_PUBLIC_KEY_LENGTH (65)

int sid_crypto_keys_init(void)
{
	static bool initialized = false;
	if (!initialized) {
		sid_error_t e = sid_pal_crypto_init();
		if (e != SID_ERROR_NONE) {
			LOG_ERR("Failed to initialize sid_pal_crypto with error %d (%s)", e,
				SID_ERROR_T_STR(e));
			return -EINVAL;
		}
		initialized = true;
	}
	/* Nothing to do, left for stable api for future features */
	return ESUCCESS;
}

static void sid_crypto_keys_attributes_set(sid_crypto_key_id_t sid_key_id,
					   psa_key_attributes_t *attr)
{
	size_t key_bits = 0;
	psa_key_usage_t usage_flags = 0;
	psa_algorithm_t alg = PSA_ALG_NONE;
	psa_key_type_t type = PSA_KEY_TYPE_NONE;

	switch (sid_key_id) {
	case SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID:
		usage_flags = PSA_KEY_USAGE_SIGN_HASH;
		alg = PSA_ALG_PURE_EDDSA;
		type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_TWISTED_EDWARDS);
		key_bits = 255;
		break;
	case SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID:
		usage_flags = PSA_KEY_USAGE_SIGN_HASH;
		alg = PSA_ALG_ECDSA(PSA_ALG_SHA_256);
		type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
		key_bits = 256;
		break;
	case SID_CRYPTO_KV_WAN_MASTER_KEY_ID:
	case SID_CRYPTO_KV_APP_KEY_KEY_ID:
	case SID_CRYPTO_KV_D2D_KEY_ID:
		usage_flags = PSA_KEY_USAGE_SIGN_MESSAGE;
		alg = PSA_ALG_CMAC;
		type = PSA_KEY_TYPE_AES;
		key_bits = 128;
		break;
	case SID_CRYPTO_KEY_ID_LAST:
		LOG_ERR("Unsupported key id %d", sid_key_id);
	}

	psa_reset_key_attributes(attr);
	psa_set_key_usage_flags(attr, usage_flags);
	psa_set_key_algorithm(attr, alg);
	psa_set_key_type(attr, type);
	psa_set_key_bits(attr, key_bits);

	psa_set_key_lifetime(attr, PSA_KEY_LIFETIME_PERSISTENT);
	psa_set_key_id(attr, sid_key_id);
}

int sid_crypto_keys_new_import(psa_key_id_t id, uint8_t *data, size_t size)
{
	/* Check arguments */
	if (PSA_KEY_ID_NULL == id || !data || !size) {
		return -EINVAL;
	}

	/* Fix for 1.18 RC4 */
	psa_key_id_t temp;
	int ret = sid_crypto_keys_buffer_get(&temp, data, size);
	if (ret == ESUCCESS) {
		LOG_HEXDUMP_DBG(data, size, "Key data is key id, aborting");
		return -EBADR;
	}

	/* Remove the key if any exists */
	psa_status_t status = PSA_ERROR_GENERIC_ERROR;
	status = psa_destroy_key(id);
	if (PSA_SUCCESS != status && PSA_ERROR_INVALID_HANDLE != status) {
		LOG_WRN("psa_destroy_key failed! (err %d id %d)", status, id);
	}

	/* Configure the key attributes */
	psa_key_id_t out_id = PSA_KEY_ID_NULL;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	sid_crypto_keys_attributes_set(id, &attributes);

	/* Import key to secure storage */
	status = psa_import_key(&attributes, data, size, &out_id);
	if (PSA_SUCCESS == status && out_id == id) {
		LOG_DBG("psa_import_key success");
	} else {
		LOG_ERR("psa_import_key failed! (err %d id %d)", status, id);
		return -EACCES;
	}

	/* Clear key data */
	status = psa_purge_key(id);
	if (status != PSA_SUCCESS) {
		LOG_ERR("psa_purge_key failed! (err %d id %d)", status, id);
		return -EFAULT;
	}

	psa_reset_key_attributes(&attributes);

	return ESUCCESS;
}

int sid_crypto_keys_new_generate(psa_key_id_t id, uint8_t *puk, size_t puk_size)
{
	/* Check arguments */
	if (PSA_KEY_ID_NULL == id || !puk || !puk_size) {
		return -EINVAL;
	}

	/* Remove the key if any exists */
	psa_status_t status = PSA_ERROR_GENERIC_ERROR;
	status = psa_destroy_key(id);
	if (PSA_SUCCESS != status && PSA_ERROR_INVALID_HANDLE != status) {
		LOG_WRN("psa_destroy_key failed! (err %d id %d)", status, id);
	}

	/* Configure the key attributes */
	psa_key_id_t out_id = PSA_KEY_ID_NULL;
	size_t out_size = 0;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	sid_crypto_keys_attributes_set(id, &attributes);

	/* Generate key in secure storage */
	status = psa_generate_key(&attributes, &out_id);
	if (PSA_SUCCESS == status && out_id == id) {
		LOG_DBG("key generation success");
	} else {
		LOG_ERR("psa_generate_key failed! (err %d id %d)", status, id);
		return -EACCES;
	}

	/* Export public key */
	uint8_t public_key[MAX_PUBLIC_KEY_LENGTH] = { 0 };
	size_t pub_key_offset = (SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID == id) ? 1 : 0;

	status = psa_export_public_key(id, public_key, puk_size + pub_key_offset, &out_size);
	memcpy(puk, &public_key[pub_key_offset], puk_size);
	memset(public_key, 0, sizeof(public_key));

	if (PSA_SUCCESS == status && out_size == puk_size + pub_key_offset) {
		LOG_DBG("export public key success");
	} else {
		LOG_ERR("psa_export_public_key failed! (err %d id %d)", status, id);
		LOG_DBG("puk size expected %d was %d", puk_size, out_size);
		return -EBADF;
	}

	/* Clear key data */
	status = psa_purge_key(id);
	if (status != PSA_SUCCESS) {
		LOG_ERR("psa_purge_key failed! (err %d id %d)", status, id);
		return -EFAULT;
	}

	psa_reset_key_attributes(&attributes);

	return ESUCCESS;
}

int sid_crypto_keys_buffer_set(psa_key_id_t id, uint8_t *data, size_t size)
{
	psa_status_t status = PSA_ERROR_GENERIC_ERROR;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t *data_id = (psa_key_id_t *)data;

	/* Check arguments */
	if (PSA_KEY_ID_NULL == id || !data || size < sizeof(psa_key_id_t)) {
		return -EINVAL;
	}

	status = psa_get_key_attributes(id, &attributes);
	psa_reset_key_attributes(&attributes);
	if (status != PSA_SUCCESS) {
		return -EACCES;
	}

	/* Save key id to buffer */
	memset(data, 0, size);
	*data_id = id;
	LOG_DBG("key buffer set %d", id);

	return ESUCCESS;
}

int sid_crypto_keys_buffer_get(psa_key_id_t *id, uint8_t *data, size_t size)
{
	if (!id || !data || !size) {
		return -EINVAL;
	}

	/* if key not found, assign null id */
	*id = PSA_KEY_ID_NULL;

	/* Check if a key data consists only of key id and zeros */
	psa_key_id_t *data_id = (psa_key_id_t *)data;
	if (!SID_CRYPTO_KEYS_ID_IS_SIDEWALK_KEY(*data_id)) {
		return -ENOENT;
	}

	for (size_t i = sizeof(psa_key_id_t); i < size; i++) {
		if (0x00 != data[i]) {
			return -ENOENT;
		}
	}

	memcpy(id, data_id, sizeof(psa_key_id_t));
	LOG_DBG("found persistent key: %d", *id);
	return ESUCCESS;
}

int sid_crypto_keys_delete(psa_key_id_t id)
{
	if (PSA_KEY_ID_NULL == id) {
		return -EINVAL;
	}

	if (!SID_CRYPTO_KEYS_ID_IS_SIDEWALK_KEY(id)) {
		return -ENOENT;
	}

	psa_status_t status = psa_destroy_key(id);
	if (status == PSA_ERROR_INVALID_HANDLE) {
		LOG_WRN("psa_destroy_key invalid id %d", id);
		return -EINVAL;
	} else if (status != PSA_SUCCESS) {
		LOG_ERR("psa_destroy_key failed! (err %d id %d)", status, id);
		return -EFAULT;
	}

	return ESUCCESS;
}

int sid_crypto_keys_deinit(void)
{
	/* Nothing to do, left for stable api for future features */
	return ESUCCESS;
}
