/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_storage_kv_internal_group_ids.h>
#include <mock_nvs.h>

#define GROUP_ID_TEST_OK        (SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID)
#define GROUP_ID_TEST_NOK       (0xffff)

void setUp(void)
{
}

/******************************************************************
* sid_pal_storage_kv_ifc
* ****************************************************************/
void test_sid_pal_storage_kv_init(void)
{
	__wrap_nvs_mount_ExpectAnyArgsAndReturn(-EINVAL);
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, sid_pal_storage_kv_init());
	__wrap_nvs_mount_ExpectAnyArgsAndReturn(0);
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
	__wrap_nvs_mount_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
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
