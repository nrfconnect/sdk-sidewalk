/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_CRYPTO_KEYS_H
#define SID_CRYPTO_KEYS_H

#include <psa/crypto.h>

#define SID_CRYPTO_KEYS_ID_IS_SIDEWALK_KEY(_id)                                                    \
	(PSA_KEY_ID_USER_MIN <= _id && _id < SID_CRYPTO_KEY_ID_LAST)

/**
 * @brief Persistent psa key ids used in Sidewalk.
 */
typedef enum {
	SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID = PSA_KEY_ID_USER_MIN,
	SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID,
	SID_CRYPTO_KV_WAN_MASTER_KEY_ID,
	SID_CRYPTO_KV_APP_KEY_KEY_ID,
	SID_CRYPTO_KV_D2D_KEY_ID,
	SID_CRYPTO_KEY_ID_LAST
} sid_crypto_key_id_t;

/**
 * @brief Init secure key storage for Sidewalk keys.
 * 
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_init(void);

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
int sid_crypto_keys_new_import(psa_key_id_t id, uint8_t *data, size_t size);

/**
 * @brief Generate a new key value.
 * 
 * @note key value under given key id will be overwritten.
 * 
 * @param id [in] Key id to generate new.
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_new_generate(psa_key_id_t id, uint8_t *puk, size_t puk_size);

/**
 * @brief Set key id in buffer.
 * 
 * @param id [in] Key id to write to the data buffer.
 * @param buffer [out] key id fulfilled with zeros.
 * @param size [in] size of raw key data buffer.
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_buffer_set(psa_key_id_t id, uint8_t *buffer, size_t size);

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
int sid_crypto_keys_buffer_get(psa_key_id_t *id, uint8_t *buffer, size_t size);

/**
 * @brief Destroy key.
 * 
 * @note This operation is irreversible.
 * 
 * @param id [in] psa key id to be permanently removed.
 * @return 0 on success, or -errno on failure.
 */
int sid_crypto_keys_delete(psa_key_id_t id);

/**
 * @brief Deinit sidewalk key storage.
 * 
 * @return 0 on success, or -errno on failure. 
 */
int sid_crypto_keys_deinit(void);

#endif /* SID_CRYPTO_KEYS_H */
