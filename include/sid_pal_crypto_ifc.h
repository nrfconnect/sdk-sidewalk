#ifndef SID_PAL_CRYPTO_IFC_H
#define SID_PAL_CRYPTO_IFC_H

#include <sid_error.h>

#include <stddef.h>
#include <stdint.h>

typedef enum {
    SID_PAL_HASH_SHA256 = 1,
    SID_PAL_HASH_SHA512,
} sid_pal_hash_algo_t;

typedef enum {
    SID_PAL_AES_CMAC_128 = 1,
    SID_PAL_AES_CTR_128,
} sid_pal_aes_algo_t;

typedef enum {
    SID_PAL_AEAD_GCM_128 = 1,
    SID_PAL_AEAD_CCM_128,
    SID_PAL_AEAD_CCM_STAR_128,
} sid_pal_aead_algo_t;

typedef enum {
    SID_PAL_ECDH_CURVE25519 = 1,
    SID_PAL_ECDH_SECP256R1,
    SID_PAL_EDDSA_ED25519,
    SID_PAL_ECDSA_SECP256R1,
} sid_pal_ecc_algo_t;

typedef enum {
    SID_PAL_CRYPTO_ENCRYPT = 1,
    SID_PAL_CRYPTO_DECRYPT,
    SID_PAL_CRYPTO_MAC_CALCULATE,
} sid_pal_aes_mode_t;

typedef enum {
    SID_PAL_CRYPTO_SIGN = 1,
    SID_PAL_CRYPTO_VERIFY,
} sid_pal_dsa_mode_t;

typedef struct {
    sid_pal_hash_algo_t algo;
    uint8_t const *data;
    size_t data_size;
    uint8_t *digest;
    size_t digest_size;
} sid_pal_hash_params_t;

typedef struct {
    sid_pal_hash_algo_t algo;
    uint8_t const *key;
    size_t key_size;
    uint8_t const *data;
    size_t data_size;
    uint8_t *digest;
    size_t digest_size;
} sid_pal_hmac_params_t;

typedef struct {
    sid_pal_aes_algo_t algo;
    sid_pal_aes_mode_t mode;
    uint8_t const *key;
    size_t key_size;
    uint8_t const *iv;
    size_t iv_size;
    uint8_t const *in;
    size_t in_size;
    uint8_t *out;
    size_t out_size;
} sid_pal_aes_params_t;

typedef struct {
    sid_pal_aead_algo_t algo;
    sid_pal_aes_mode_t mode;
    uint8_t const *key;
    size_t key_size;
    uint8_t const *iv;
    size_t iv_size;
    uint8_t const *aad;
    size_t aad_size;
    uint8_t const *in;
    size_t in_size;
    uint8_t *out;
    size_t out_size;
    uint8_t *mac;
    size_t mac_size;
} sid_pal_aead_params_t;

typedef struct {
    sid_pal_ecc_algo_t algo;
    sid_pal_dsa_mode_t mode;
    uint8_t const *key;
    size_t key_size;
    uint8_t const *in;
    size_t in_size;
    uint8_t *signature;
    size_t sig_size;
} sid_pal_dsa_params_t;

typedef struct {
    sid_pal_ecc_algo_t algo;
    uint8_t const *prk;
    size_t prk_size;
    uint8_t const *puk;
    size_t puk_size;
    uint8_t *shared_secret;
    size_t shared_secret_sz;
} sid_pal_ecdh_params_t;

typedef struct {
    sid_pal_ecc_algo_t algo;
    uint8_t *prk;
    size_t prk_size;
    uint8_t *puk;
    size_t puk_size;
} sid_pal_ecc_key_gen_params_t;

sid_error_t sid_pal_crypto_init(void);
sid_error_t sid_pal_crypto_deinit(void);
sid_error_t sid_pal_crypto_rand(uint8_t *rand, size_t size);
sid_error_t sid_pal_crypto_hash(sid_pal_hash_params_t *params);
sid_error_t sid_pal_crypto_hmac(sid_pal_hmac_params_t* params);
sid_error_t sid_pal_crypto_aes_crypt(sid_pal_aes_params_t *params);
sid_error_t sid_pal_crypto_aead_crypt(sid_pal_aead_params_t  *params);
sid_error_t sid_pal_crypto_ecc_dsa(sid_pal_dsa_params_t *params);
sid_error_t sid_pal_crypto_ecc_ecdh(sid_pal_ecdh_params_t *params);
sid_error_t sid_pal_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t *params);

#endif
