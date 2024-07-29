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

#define TEST_KEY_SIZE (16)

static psa_key_id_t test_key_id = SID_CRYPTO_KV_APP_KEY_KEY_ID;

static void *setup(void)
{
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_init());
	zassert_equal(SID_ERROR_NONE, sid_pal_crypto_init());

	return NULL;
}

static void teardown(void *f)
{
	sid_crypto_keys_delete(test_key_id);
	sid_pal_crypto_deinit();
}

ZTEST(crypto_keys, test_sid_crypto_key_invalid_args)
{
	int err = -ENOEXEC;
	uint8_t key_data[TEST_KEY_SIZE];

	err = sid_crypto_keys_init();
	zassert_equal(0, err, "err: %d", err);

	/* Invalid data */
	psa_key_id_t new_key_id = PSA_KEY_ID_NULL;
	err = sid_crypto_keys_buffer_get(&new_key_id, NULL, TEST_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_buffer_get(&new_key_id, key_data, 0);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_buffer_set(test_key_id, NULL, TEST_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_buffer_set(test_key_id, key_data, 0);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_import(test_key_id, NULL, TEST_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_import(test_key_id, key_data, 0);
	zassert_equal(-EINVAL, err, "err: %d", err);

	/* Invalid key id */
	err = sid_crypto_keys_buffer_get(NULL, key_data, TEST_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_buffer_set(PSA_KEY_ID_NULL, key_data, TEST_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_import(PSA_KEY_ID_NULL, key_data, TEST_KEY_SIZE);
	zassert_equal(-EINVAL, err, "err: %d", err);

	err = sid_crypto_keys_new_generate(PSA_KEY_ID_NULL);
	zassert_equal(-EINVAL, err, "err: %d", err);
}

ZTEST(crypto_keys, test_sid_crypto_key_positive)
{
	psa_key_id_t new_key_id = PSA_KEY_ID_NULL;
	uint8_t test_key_data[TEST_KEY_SIZE];
	int err = -ENOEXEC;

	err = sid_crypto_keys_init();
	zassert_equal(0, err, "err: %d", err);

	err = sid_crypto_keys_new_import(test_key_id, test_key_data, TEST_KEY_SIZE);
	zassert_equal(0, err, "err: %d", err);

	err = sid_crypto_keys_buffer_set(test_key_id, test_key_data, TEST_KEY_SIZE);
	zassert_equal(0, err, "err: %d", err);

	err = sid_crypto_keys_buffer_get(&new_key_id, test_key_data, TEST_KEY_SIZE);
	zassert_equal(0, err, "err: %d", err);

	zassert_equal(new_key_id, test_key_id);

	err = sid_crypto_keys_deinit();
	zassert_equal(0, err, "err: %d", err);
}

ZTEST_SUITE(crypto_keys, NULL, setup, NULL, NULL, teardown);
