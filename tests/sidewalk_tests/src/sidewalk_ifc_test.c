/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_timer_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_pal_log_ifc.h>
#include <mock_nvs.h>
#include <mock_flash.h>
#include <mock_zephyr_time.h>
#include <string.h>
#include <limits.h>

#define GROUP_ID_TEST_OK        0
#define GROUP_ID_TEST_NOK       9

#define SID_PAL_TIMER_PRIO_CLASS_TOO_BIG        (9)
#define SID_PAL_TIMER_PRIO_CLASS_TOO_SMALL      (-9)

static uint8_t test_data_buffer[512];

static sid_pal_timer_t *p_null_timer = NULL;
static sid_pal_timer_t test_timer;
static int test_timer_arg;

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

/******************************************************************
* sid_pal_timer_ifc
* ****************************************************************/
void timer_cb(void *arg, sid_pal_timer_t *originator)
{

}

static void timer_init(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_cb, &test_timer_arg));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(&test_timer_arg, test_timer.callback_arg);
	TEST_ASSERT_EQUAL(timer_cb, test_timer.callback);
}

static void timer_deinit(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));
	TEST_ASSERT_FALSE(test_timer.is_initialized);
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_NULL(test_timer.callback);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
}

void test_sid_pal_timer_init_deinit(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_init(NULL, NULL, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_init(&test_timer, NULL, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_init(p_null_timer, timer_cb, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_deinit(p_null_timer));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_cb, NULL));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(timer_cb, test_timer.callback);
	TEST_ASSERT_EQUAL(SID_ERROR_ALREADY_INITIALIZED, sid_pal_timer_init(&test_timer, timer_cb, NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));
	TEST_ASSERT_FALSE(test_timer.is_initialized);
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_NULL(test_timer.callback);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);

	timer_init();
	timer_deinit();
}

void test_sid_pal_timer_arm(void)
{
	struct sid_timespec when = { .tv_sec = 5 };
	struct sid_timespec period = { .tv_sec = 5 };

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_arm(p_null_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_arm(p_null_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, NULL, NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_TOO_BIG, &when, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_TOO_SMALL, &when, NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_NOT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_TRUE(test_timer.is_periodic);
	TEST_ASSERT_NOT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));

	timer_deinit();
}

void test_sid_pal_timer_is_armed(void)
{
	struct sid_timespec when = { .tv_sec = 5 };

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(p_null_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_cancel(void)
{
	struct sid_timespec when = { .tv_sec = 5 };
	struct sid_timespec period = { .tv_sec = 5 };

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_cancel(p_null_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_timer_cancel(&test_timer));

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_TRUE(test_timer.is_periodic);
	TEST_ASSERT_NOT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);

	timer_deinit();
}

void test_sid_pal_timer_is_armed_deinit(void)
{
	struct sid_timespec when = { .tv_sec = 5 };

	timer_init();

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
}

/******************************************************************
* sid_pal_uptime_ifc
* ****************************************************************/
static void uptime_test_time(uint64_t uptime_nanoseconds)
{
	uint32_t seconds, nanoseconds;
	struct sid_timespec sid_time;

	seconds = (uint32_t)(uptime_nanoseconds / NSEC_PER_SEC);
	nanoseconds = (uint32_t)(uptime_nanoseconds % NSEC_PER_SEC);

	__wrap_zephyr_uptime_ns_ExpectAndReturn(uptime_nanoseconds);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_uptime_now(&sid_time));

	TEST_ASSERT_EQUAL(seconds, sid_time.tv_sec);
	TEST_ASSERT_EQUAL(nanoseconds, sid_time.tv_nsec);
}

void test_sid_pal_uptime_get_now(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_uptime_now(NULL));

	uptime_test_time(0ull);
	uptime_test_time(1ull * NSEC_PER_SEC + 10ull);
	uptime_test_time(10ull * NSEC_PER_SEC + 100ull);
	uptime_test_time(LONG_MAX);
	uptime_test_time(LLONG_MAX);
	uptime_test_time(ULLONG_MAX);
}

void test_sid_pal_uptime_accuracy(void)
{
	/* Note: This is a oversimplified test fo dummy implementation.*/
	int16_t ppm;

	ppm = sid_pal_uptime_get_xtal_ppm();
	sid_pal_uptime_set_xtal_ppm(ppm);
	TEST_ASSERT_EQUAL(ppm, sid_pal_uptime_get_xtal_ppm());
}

/******************************************************************
* sid_pal_log_ifc
* ****************************************************************/

void test_log_severity(void)
{
	sid_pal_log(SID_PAL_LOG_SEVERITY_ERROR, 1, "Sidewalk log Error");
	sid_pal_log(SID_PAL_LOG_SEVERITY_WARNING, 1, "Sidewalk log Warning");
	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 1, "Sidewalk log Info");
	sid_pal_log(SID_PAL_LOG_SEVERITY_DEBUG, 1, "Sidewalk log Debug");
	sid_pal_log(SID_PAL_LOG_SEVERITY_DEBUG + 1, 1, "Sidewalk log Unknow");
	TEST_PASS();
}

void test_log_arguments(void)
{
	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 1, "%s", "Text");
	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 1, "%d", 1);
	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 5, "%d, %d, %d, %d, %d", 1, 2, 3, 4, 5);

	sid_pal_log(SID_PAL_LOG_SEVERITY_INFO, 1, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
	TEST_PASS();
}

void test_log_flush(void)
{
	sid_pal_log_flush();
	TEST_PASS();
}

void test_log_push_string(void)
{
	char test_string[] = "test message 123";

	const char *ret_string = sid_pal_log_push_str(test_string);

	TEST_ASSERT_EQUAL_STRING(test_string, ret_string);
}

void test_log_get_buffer(void)
{
	bool ret;
	struct sid_pal_log_buffer *const test_log_buffer = NULL;

	ret = sid_pal_log_get_log_buffer(test_log_buffer);
	TEST_ASSERT_FALSE_MESSAGE(ret, "sid_pal_log_get_log_buffer not implemented, should always return false.");
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
