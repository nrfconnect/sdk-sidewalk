/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <sid_pal_storage_kv_ifc.h>

#include <zephyr/storage/flash_map.h>
#include <zephyr/device.h>
#include <string.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <sid_pal_assert_ifc.h>

#define GROUP_ID_TEST_OK 0

typedef enum {
	TEST_KEY_1,
	TEST_KEY_2,
	TEST_KEY_3,
	TEST_KEY_4,
	TEST_KEY_5,
	TEST_KEY_6,
	TEST_KEY_7,
	TEST_KEY_8,
	TEST_KEY_9,
	TEST_KEY_10,
	TEST_KEY_END
} test_keys_t;

static uint32_t test_data[TEST_KEY_END];
static uint8_t test_data_buffer[512];
static uint32_t test_read_data;
static uint8_t test_key_x = 0xCC;

static void *sid_pal_storage_test_init(void)
{
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		memset(&test_data[tk], 0x05 + tk, sizeof(uint32_t));
	}
	return NULL;
}

/******************************************************************
* NOTE: DO NOT CHANGE THE ORDER OF THE TESTS!
* ****************************************************************/
// ZTEST(functional_storage, test_1sid_pal_storage_no_init)
// {
// 	uint32_t len;

// 	// NVM is not initialized.
// 	zassert_equal(SID_ERROR_STORAGE_ERASE_FAIL,
// 		      sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
// 	zassert_equal(SID_ERROR_STORAGE_WRITE_FAIL,
// 		      sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1, &test_data[0],
// 						    sizeof(test_data[0])));
// 	zassert_equal(SID_ERROR_STORAGE_READ_FAIL,
// 		      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, TEST_KEY_1, &test_read_data,
// 						    sizeof(test_read_data)));
// 	zassert_equal(SID_ERROR_STORAGE_READ_FAIL,
// 		      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, TEST_KEY_1, &len));
// 	zassert_equal(SID_ERROR_STORAGE_ERASE_FAIL,
// 		      sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, TEST_KEY_1));
// }

ZTEST(functional_storage, test_2sid_pal_storage_kv_init)
{
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_init());
}

ZTEST(functional_storage, test_4sid_pal_storage_null_ptr)
{
	zassert_equal(SID_ERROR_NULL_POINTER,
		      sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1, NULL,
						    sizeof(test_data[TEST_KEY_1])));
	zassert_equal(SID_ERROR_NULL_POINTER,
		      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, TEST_KEY_1, NULL,
						    sizeof(test_read_data)));
	zassert_equal(SID_ERROR_NULL_POINTER,
		      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, TEST_KEY_1, NULL));
}

ZTEST(functional_storage, test_5sid_pal_storage_erase_all_1)
{
	test_read_data = 0;
	uint32_t len;

	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NOT_FOUND,
			      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
							    sizeof(test_read_data)));
		zassert_equal(SID_ERROR_NOT_FOUND,
			      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
	}
}

ZTEST(functional_storage, test_6sid_pal_storage_write_get)
{
	test_read_data = 0;
	uint32_t sector_size = 0x1000;
	uint8_t update_val[8];
	uint8_t update_val_read[8];
	uint32_t len;

	memset(test_data_buffer, 0xAA, sizeof(test_data_buffer));
	memset(update_val, 0x12, sizeof(update_val));

	// Incorrect length
	zassert_equal(SID_ERROR_INVALID_ARGS,
		      sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1,
						    &test_data[TEST_KEY_1], 0));

	// Write many data
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, tk, &test_data[tk],
							    sizeof(test_data[tk])));
	}
	// Check data consistency
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		zassert_equal(sizeof(test_data[tk]), len);
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
							    sizeof(test_read_data)));
		zassert_equal(test_read_data, test_data[tk]);
	}

	// Now, try to write more data.
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, test_key_x, test_data_buffer,
						    sizeof(test_data_buffer)));
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
	zassert_equal(sizeof(test_data_buffer), len);

	// Check if we do not out of the memory
	zassert_equal(SID_ERROR_STORAGE_WRITE_FAIL,
		      sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, test_key_x, test_data_buffer,
						    sector_size));

	// Check again data consistency
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		zassert_equal(sizeof(test_data[tk]), len);
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
							    sizeof(test_read_data)));
		zassert_equal(test_read_data, test_data[tk]);
	}

	// Update one key
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_5, update_val,
						    sizeof(update_val)));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		if (TEST_KEY_5 == tk) {
			zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(
							      GROUP_ID_TEST_OK, tk, &len));
			zassert_equal(sizeof(update_val), len);
			zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_get(
							      GROUP_ID_TEST_OK, tk, update_val_read,
							      sizeof(update_val_read)));
			zassert_mem_equal(update_val_read, update_val, sizeof(update_val_read));
		} else {
			zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(
							      GROUP_ID_TEST_OK, tk, &len));
			zassert_equal(sizeof(test_data[tk]), len);
			zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_get(
							      GROUP_ID_TEST_OK, tk, &test_read_data,
							      sizeof(test_read_data)));
			zassert_equal(test_read_data, test_data[tk]);
		}
	}

	// Restore data
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, tk, &test_data[tk],
							    sizeof(test_data[tk])));
	}
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		zassert_equal(sizeof(test_data[tk]), len);
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
							    sizeof(test_read_data)));
		zassert_equal(test_read_data, test_data[tk]);
	}
}

ZTEST(functional_storage, test_7sid_pal_storage_delete_records)
{
	test_read_data = 0;
	uint32_t len;

	zassert_equal(SID_ERROR_NONE,
		      sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, TEST_KEY_5));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		if (TEST_KEY_5 == tk) {
			zassert_equal(SID_ERROR_NOT_FOUND,
				      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk,
								    &test_read_data,
								    sizeof(test_read_data)));
		} else {
			zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(
							      GROUP_ID_TEST_OK, tk, &len));
			zassert_equal(sizeof(test_data[tk]), len);
			zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_get(
							      GROUP_ID_TEST_OK, tk, &test_read_data,
							      sizeof(test_read_data)));
			zassert_equal(test_read_data, test_data[tk]);
		}
	}

	// another keys
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NONE,
			      sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, tk));
	}
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NOT_FOUND,
			      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		zassert_equal(SID_ERROR_NOT_FOUND,
			      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
							    sizeof(test_read_data)));
	}
}

ZTEST(functional_storage, test_8sid_pal_storage_erase_all_2)
{
	test_read_data = 0;
	uint32_t len;

	// Check if 'test_key_x' key still exist
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
	zassert_equal(sizeof(test_data_buffer), len);
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, test_key_x, &test_read_data,
						    sizeof(test_read_data)));

	// Erase whole NVM
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		zassert_equal(SID_ERROR_NOT_FOUND,
			      sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
							    sizeof(test_read_data)));
		zassert_equal(SID_ERROR_NOT_FOUND,
			      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
	}
	zassert_equal(SID_ERROR_NOT_FOUND,
		      sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
}

ZTEST(functional_storage, test_sanity)
{
	zassert_equal(true, true);
}

ZTEST_SUITE(functional_storage, NULL, sid_pal_storage_test_init, NULL, NULL, NULL);
