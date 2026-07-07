/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_CRYPTO_KEYS_H
#define SID_CRYPTO_KEYS_H

#include <psa/crypto.h>

#if defined(CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU)
#include <cracen_psa_kmu.h>

#define SID_CRYPTO_KMU_KEY_ID(_offset)                                                             \
	PSA_KEY_HANDLE_FROM_CRACEN_KMU_SLOT(                                                       \
		CRACEN_KMU_KEY_USAGE_SCHEME_RAW,                                                   \
		CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU_SLOT_START + (_offset))
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU */

/**
 * @brief Persistent psa key ids used in Sidewalk.
 */
typedef enum {
#if defined(CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU)
	SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID = SID_CRYPTO_KMU_KEY_ID(0), /* occupies 2 KMU slots */
	SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID = SID_CRYPTO_KMU_KEY_ID(2), /* occupies 2 KMU slots */
	SID_CRYPTO_KV_WAN_MASTER_KEY_ID = SID_CRYPTO_KMU_KEY_ID(4),
	SID_CRYPTO_KV_APP_KEY_KEY_ID = SID_CRYPTO_KMU_KEY_ID(5),
	SID_CRYPTO_KV_D2D_KEY_ID = SID_CRYPTO_KMU_KEY_ID(6),
	SID_CRYPTO_KEY_ID_LAST
#else
	SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID = PSA_KEY_ID_USER_MIN,
	SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID,
	SID_CRYPTO_KV_WAN_MASTER_KEY_ID,
	SID_CRYPTO_KV_APP_KEY_KEY_ID,
	SID_CRYPTO_KV_D2D_KEY_ID,
	SID_CRYPTO_KEY_ID_LAST
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU */
} sid_crypto_key_id_t;

#define SID_CRYPTO_KEYS_ID_IS_SIDEWALK_KEY(_id)                                                    \
	((_id) == SID_CRYPTO_MFG_ED25519_PRIV_KEY_ID ||                                            \
	 (_id) == SID_CRYPTO_MFG_SECP_256R1_PRIV_KEY_ID ||                                         \
	 (_id) == SID_CRYPTO_KV_WAN_MASTER_KEY_ID || (_id) == SID_CRYPTO_KV_APP_KEY_KEY_ID ||      \
	 (_id) == SID_CRYPTO_KV_D2D_KEY_ID)

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
 * @param puk [in] Buffer with raw key value.
 * @param puk_size [in] Size of buffer with rew kay value.
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
