/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <mock_nvs.h>
#include <mock_flash.h>
#include <string.h>

#define GROUP_ID_TEST_OK        0
#define GROUP_ID_TEST_NOK       9

static uint8_t test_data_buffer[512];

void setUp(void)
{
}

/******************************************************************
* sid_pal_storage_kv_ifc
* ****************************************************************/
void test_sid_pal_storage_kv_init(void)
{
	__wrap_nvs_init_ExpectAnyArgsAndReturn(-EINVAL);
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, sid_pal_storage_kv_init());
	__wrap_nvs_init_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_init());
}

void test_sid_pal_storage_kv_record_get(void)
{
	uint8_t test_buff[5];

	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_NOK, 1, NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, 1, NULL, 0));

	__wrap_nvs_read_ExpectAnyArgsAndReturn(-ENOENT);
	TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, 1, test_buff, sizeof(test_buff)));

	__wrap_nvs_read_ExpectAnyArgsAndReturn(-EINVAL);
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_READ_FAIL, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, 1, test_buff, sizeof(test_buff)));

	__wrap_nvs_read_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, 1, test_buff, sizeof(test_buff)));
}

void test_sid_pal_storage_kv_record_get_len(void)
{
	uint32_t test_len = 0;
	uint32_t test_expected_len = 16;

	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_NOK, 1, &test_len));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, 1, NULL));

	__wrap_nvs_read_ExpectAnyArgsAndReturn(-ENOENT);
	TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, 1, &test_len));

	__wrap_nvs_read_ExpectAnyArgsAndReturn(-EINVAL);
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_READ_FAIL, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, 1, &test_len));

	__wrap_nvs_read_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_READ_FAIL, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, 1, &test_len));

	__wrap_nvs_read_ExpectAnyArgsAndReturn(test_expected_len);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, 1, &test_len));
	TEST_ASSERT_EQUAL(test_expected_len, test_len);
}

void test_sid_pal_storage_kv_record_set(void)
{
	uint8_t test_buff[16];

	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_record_set(GROUP_ID_TEST_NOK, 1, test_buff, sizeof(test_buff)));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, 1, NULL, sizeof(test_buff)));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, 1, test_buff, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, 1, test_buff, 9999));

	__wrap_nvs_write_ExpectAnyArgsAndReturn(-ENOSPC);
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_FULL, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, 1, test_buff, sizeof(test_buff)));

	__wrap_nvs_write_ExpectAnyArgsAndReturn(-EINVAL);
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_WRITE_FAIL, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, 1, test_buff, sizeof(test_buff)));

	__wrap_nvs_write_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, 1, test_buff, sizeof(test_buff)));

	__wrap_nvs_write_ExpectAnyArgsAndReturn(sizeof(test_buff));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, 1, test_buff, sizeof(test_buff)));
}

void test_sid_pal_storage_kv_record_delete(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_record_delete(GROUP_ID_TEST_NOK, 1));

	__wrap_nvs_delete_ExpectAnyArgsAndReturn(-ENOENT);
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_ERASE_FAIL, sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, 1));

	__wrap_nvs_delete_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, 1));
}

void test_sid_pal_storage_kv_group_delete(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_NOK));

	__wrap_nvs_clear_ExpectAnyArgsAndReturn(-EINVAL);
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_ERASE_FAIL, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));

	__wrap_nvs_clear_ExpectAnyArgsAndReturn(0);
	__wrap_nvs_init_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
}

/******************************************************************
* sid_pal_assert_ifc
* ****************************************************************/
static bool should_assert;
void assert_post_action(const char *file, unsigned int line)
{
	ARG_UNUSED(file);
	ARG_UNUSED(line);

	if (should_assert) {
		should_assert = false;
		TEST_PASS_MESSAGE("Asserted.");
	}
	TEST_FAIL_MESSAGE("Asserted, but should not.");
}

void test_sid_pal_assert(void)
{
	should_assert = false;
	SID_PAL_ASSERT(true);

	should_assert = true;
	SID_PAL_ASSERT(false);
	TEST_ASSERT_FALSE_MESSAGE(should_assert, "No assert when it should be.");
}

/******************************************************************
* sid_pal_mfg_store_ifc
* ****************************************************************/
void test_sid_pal_mfg_storage_no_init(void)
{
	uint8_t read_buffer[SID_PAL_MFG_STORE_VERSION_SIZE];

	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, read_buffer, SID_PAL_MFG_STORE_VERSION_SIZE));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_mfg_store_erase());
	TEST_ASSERT_FALSE(sid_pal_mfg_store_is_empty());
	memset(read_buffer, 0xAA, SID_PAL_MFG_STORE_VERSION_SIZE);
	memset(test_data_buffer, 0xAA, SID_PAL_MFG_STORE_VERSION_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_VERSION, read_buffer, SID_PAL_MFG_STORE_VERSION_SIZE);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(read_buffer, test_data_buffer, SID_PAL_MFG_STORE_VERSION_SIZE);
}

void test_sid_pal_mfg_storage_init(void)
{
	static const sid_pal_mfg_store_region_t mfg_store_region = {
		.addr_start = 0,
		.addr_end = 0x1000,
	};

	sid_pal_mfg_store_init(mfg_store_region);
	TEST_ASSERT_EQUAL(0, sid_pal_mfg_store_erase());
}

void test_sid_pal_mfg_storage_write(void)
{
	uint8_t write_buff[SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN];

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff, 128));
	TEST_ASSERT_EQUAL(SID_ERROR_INCOMPATIBLE_PARAMS, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff, 3));
	TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_mfg_store_write(999, write_buff, SID_PAL_MFG_STORE_VERSION_SIZE));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, NULL, SID_PAL_MFG_STORE_VERSION_SIZE));

	__wrap_flash_write_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(0, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff, SID_PAL_MFG_STORE_VERSION_SIZE));
}

void test_sid_pal_mfg_storage_dev_id_get(void)
{
	uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE];

	memset(dev_id, 0x00, sizeof(dev_id));

	TEST_ASSERT_FALSE(sid_pal_mfg_store_dev_id_get(dev_id));
	TEST_ASSERT_EQUAL(0xBF, dev_id[0]);
	// DEV_ID_REG=0x33AABB99
	TEST_ASSERT_EQUAL(0x33, dev_id[1]);
	TEST_ASSERT_EQUAL(0xAA, dev_id[2]);
	TEST_ASSERT_EQUAL(0xBB, dev_id[3]);
	TEST_ASSERT_EQUAL(0x99, dev_id[4]);
}

void test_sid_pal_mfg_storage_sn_get(void)
{
	uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE];

	// No serial number.
	TEST_ASSERT_FALSE(sid_pal_mfg_store_serial_num_get(serial_num));
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
