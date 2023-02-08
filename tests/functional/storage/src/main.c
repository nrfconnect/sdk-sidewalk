/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_storage_kv_ifc.h>

#include <zephyr/storage/flash_map.h>
#include <zephyr/device.h>
#include <string.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <sid_pal_assert_ifc.h>

#define GROUP_ID_TEST_OK        0
#define GROUP_ID_TEST_NOK       9

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

static void test_sid_pal_storage_init(void)
{
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		memset(&test_data[tk], 0x05 + tk, sizeof(uint32_t));
	}
}

/******************************************************************
* NOTE: DO NOT CHANGE THE ORDER OF THE TESTS!
* ****************************************************************/
void test_sid_pal_storage_no_init(void)
{
	uint32_t len;

	// NVM is not initialized.
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_ERASE_FAIL, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_WRITE_FAIL,
			  sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1, &test_data[0],
							sizeof(test_data[0])));
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_READ_FAIL,
			  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, TEST_KEY_1, &test_read_data,
							sizeof(test_read_data)));
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_READ_FAIL,
			  sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, TEST_KEY_1, &len));
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_ERASE_FAIL, sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, TEST_KEY_1));
}

void test_sid_pal_storage_kv_init(void)
{
	sid_pal_storage_kv_init();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_init());
}

void test_sid_pal_storage_incorrect_group_id(void)
{
	uint32_t len;

	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_NOK));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE,
			  sid_pal_storage_kv_record_set(GROUP_ID_TEST_NOK, TEST_KEY_1, &test_data[TEST_KEY_1],
							sizeof(test_data[TEST_KEY_1])));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE,
			  sid_pal_storage_kv_record_get(GROUP_ID_TEST_NOK, TEST_KEY_1, &test_read_data,
							sizeof(test_read_data)));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE,
			  sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_NOK, TEST_KEY_1, &len));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE,
			  sid_pal_storage_kv_record_delete(GROUP_ID_TEST_NOK, TEST_KEY_1));
}

void test_sid_pal_storage_null_ptr(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER,
			  sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1, NULL,
							sizeof(test_data[TEST_KEY_1])));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER,
			  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, TEST_KEY_1, NULL, sizeof(test_read_data)));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER,
			  sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, TEST_KEY_1, NULL));
}

void test_sid_pal_storage_erase_all_1(void)
{
	test_read_data = 0;
	uint32_t len;

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND,
				  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
								sizeof(test_read_data)));
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
	}
}

void test_sid_pal_storage_write_get(void)
{
	test_read_data = 0;
	uint32_t sector_size = 0x1000;
	uint8_t update_val[8];
	uint8_t update_val_read[8];
	uint32_t len;

	memset(test_data_buffer, 0xAA, sizeof(test_data_buffer));
	memset(update_val, 0x12, sizeof(update_val));

	// Incorrect length
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS,
			  sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1, &test_data[TEST_KEY_1], 0));

	// Write many data
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE,
				  sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, tk, &test_data[tk],
								sizeof(test_data[tk])));
	}
	// Check data consistency
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
		TEST_ASSERT_EQUAL(SID_ERROR_NONE,
				  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
								sizeof(test_read_data)));
		TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
	}

	// Now, try to write more data.
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, test_key_x, test_data_buffer,
							sizeof(test_data_buffer)));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
	TEST_ASSERT_EQUAL(sizeof(test_data_buffer), len);

	// Check if we do not out of the memory
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES,
			  sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, test_key_x, test_data_buffer, sector_size));

	// Check again data consistency
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
		TEST_ASSERT_EQUAL(SID_ERROR_NONE,
				  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
								sizeof(test_read_data)));
		TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
	}

	// Update one key
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_5, update_val, sizeof(update_val)));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		if (TEST_KEY_5 == tk) {
			TEST_ASSERT_EQUAL(SID_ERROR_NONE,
					  sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
			TEST_ASSERT_EQUAL(sizeof(update_val), len);
			TEST_ASSERT_EQUAL(SID_ERROR_NONE,
					  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, update_val_read,
									sizeof(update_val_read)));
			TEST_ASSERT_EQUAL_HEX8_ARRAY(update_val_read, update_val, sizeof(update_val_read));
		} else {
			TEST_ASSERT_EQUAL(SID_ERROR_NONE,
					  sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
			TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
			TEST_ASSERT_EQUAL(SID_ERROR_NONE,
					  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
									sizeof(test_read_data)));
			TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
		}
	}

	// Restore data
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE,
				  sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, tk, &test_data[tk],
								sizeof(test_data[tk])));
	}
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
		TEST_ASSERT_EQUAL(SID_ERROR_NONE,
				  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
								sizeof(test_read_data)));
		TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
	}
}

void test_sid_pal_storage_delete_records(void)
{
	test_read_data = 0;
	uint32_t len;

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, TEST_KEY_5));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		if (TEST_KEY_5 == tk) {
			TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND,
					  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
									sizeof(test_read_data)));
		} else {
			TEST_ASSERT_EQUAL(SID_ERROR_NONE,
					  sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
			TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
			TEST_ASSERT_EQUAL(SID_ERROR_NONE,
					  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
									sizeof(test_read_data)));
			TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
		}
	}

	// another keys
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, tk));
	}
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND,
				  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
								sizeof(test_read_data)));
	}
}

void test_sid_pal_storage_erase_all_2(void)
{
	test_read_data = 0;
	uint32_t len;

	// Check if 'test_key_x' key still exist
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
	TEST_ASSERT_EQUAL(sizeof(test_data_buffer), len);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, test_key_x, &test_read_data,
							sizeof(test_read_data)));

	// Erase whole NVM
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND,
				  sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data,
								sizeof(test_read_data)));
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
	}
	TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
}

extern int unity_main(void);

void main(void)
{
	test_sid_pal_storage_init();
	(void)unity_main();
}
