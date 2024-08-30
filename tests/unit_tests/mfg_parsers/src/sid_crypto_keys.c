/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sid_crypto_keys.h>
/**
 * @brief Init secure key storage for Sidewalk keys.
 * 
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_init(void)
{
	return 0;
}

/**
 * @brief Import key value form buffer.
 * 
 * @note key value under given key id will be overwritten.
 * 
 * @param id [in] Key id to import data.
 * @param data [in] raw key data on input.
 * @param size [in] size of raw key data buffer.
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_new_import(psa_key_id_t id, uint8_t *data, size_t size)
{
	return 0;
}

/**
 * @brief Generate a new key value.
 * 
 * @note key value under given key id will be overwritten.
 * 
 * @param id [in] Key id to generate new.
 * @param puk [in] Buffer with raw key value.
 * @param puk_size [in] Size of buffer with rew kay value.
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_new_generate(psa_key_id_t id, uint8_t *puk, size_t puk_size)
{
	return 0;
}

/**
 * @brief Set key id in buffer.
 * 
 * @param id [in] Key id to write to the data buffer.
 * @param buffer [out] key id fulfilled with zeros.
 * @param size [in] size of raw key data buffer.
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_buffer_set(psa_key_id_t id, uint8_t *buffer, size_t size)
{
	return 0;
}

/**
 * @brief Get key id from buffer.
 * 
 * @param id [out] psa key id from key data buffer.
 *  If key not found set to PSA_KEY_ID_NULL.
 * @param buffer [in] key data buffer.
 * @param size [in] size of key data buffer.
 * @return 0 on success, or -errno on failure.
 *  -ENOENT - if no key in buffer.
 */
int sid_crypto_keys_buffer_get(psa_key_id_t *id, uint8_t *buffer, size_t size)
{
	return 0;
}

/**
 * @brief Destroy key.
 * 
 * @note This operation is irreversible.
 * 
 * @param id [in] psa key id to be permanently removed.
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_delete(psa_key_id_t id)
{
	return 0;
}

/**
 * @brief Deinit sidewalk key storage.
 * 
 * @return 0 on success, or -errno on failure. 
 */
int sid_crypto_keys_deinit(void)
{
	return 0;
}
