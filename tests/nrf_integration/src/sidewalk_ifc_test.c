/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <storage/flash_map.h>
#include <device.h>
#include <string.h>
#include <sys/byteorder.h>
#include <sid_pal_timer_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <kernel.h>

/* number of nanoseconds per microseconds */
#define NSEC_PER_MSEC ((NSEC_PER_USEC) *(USEC_PER_MSEC))

#define GROUP_ID_TEST_OK        0
#define GROUP_ID_TEST_NOK       9

#define SID_PAL_TIMER_PRIO_CLASS_TOO_BIG        (9)
#define SID_PAL_TIMER_PRIO_CLASS_TOO_SMALL      (-9)

#define MFG_VERSION_1_VAL   0x01000000

/* Flash block size in bytes */
#define MFG_STORAGE_SIZE  (DT_PROP(DT_CHOSEN(zephyr_flash), erase_block_size))
/**
 * Last block in the flash storage will be used for manufacturing storage.
 * MFG_START_OFFSET = 0x0FF000
 * MFG_END_OFFSET   = 0x100000
 */
#define MFG_END_OFFSET (FLASH_AREA_OFFSET(storage) + FLASH_AREA_SIZE(storage))
#define MFG_START_OFFSET (MFG_END_OFFSET - MFG_STORAGE_SIZE)

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

static sid_pal_timer_t *p_null_timer = NULL;
static sid_pal_timer_t test_timer;
static int test_timer_arg;
static int callback_arg;
static int timer_callback_cnt;

void setUp(void)
{
}

/******************************************************************
* sid_pal_storage_kv_ifc
* NOTE: DO NOT CHANGE THE ORDER OF THE TESTS!
* ****************************************************************/
void test_sid_pal_storage_no_init(void)
{
	uint32_t len;

	// NVM is not initialized.
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_ERASE_FAIL, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_WRITE_FAIL, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1, &test_data[0], sizeof(test_data[0])));
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_READ_FAIL, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, TEST_KEY_1, &test_read_data, sizeof(test_read_data)));
	TEST_ASSERT_EQUAL(SID_ERROR_STORAGE_READ_FAIL, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, TEST_KEY_1, &len));
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
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_record_set(GROUP_ID_TEST_NOK, TEST_KEY_1, &test_data[TEST_KEY_1], sizeof(test_data[TEST_KEY_1])));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_NOK, TEST_KEY_1, &test_read_data, sizeof(test_read_data)));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_NOK, TEST_KEY_1, &len));
	TEST_ASSERT_EQUAL(SID_ERROR_PARAM_OUT_OF_RANGE, sid_pal_storage_kv_record_delete(GROUP_ID_TEST_NOK, TEST_KEY_1));
}

void test_sid_pal_storage_null_ptr(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1, NULL, sizeof(test_data[TEST_KEY_1])));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, TEST_KEY_1, NULL, sizeof(test_read_data)));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, TEST_KEY_1, NULL));
}

void test_sid_pal_storage_erase_all_1(void)
{
	test_read_data = 0;
	uint32_t len;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
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
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_1, &test_data[TEST_KEY_1], 0));

	// Write many data
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, tk, &test_data[tk], sizeof(test_data[tk])));
	}
	// Check data consistency
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
		TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
	}

	// Now, try to write more data.
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, test_key_x, test_data_buffer, sizeof(test_data_buffer)));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
	TEST_ASSERT_EQUAL(sizeof(test_data_buffer), len);

	// Check if we do not out of the memory
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, test_key_x, test_data_buffer, sector_size));

	// Check again data consistency
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
		TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
	}

	// Update one key
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, TEST_KEY_5, update_val, sizeof(update_val)));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		if (TEST_KEY_5 == tk) {
			TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
			TEST_ASSERT_EQUAL(sizeof(update_val), len);
			TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, update_val_read, sizeof(update_val_read)));
			TEST_ASSERT_EQUAL_HEX8_ARRAY(update_val_read, update_val, sizeof(update_val_read));
		} else {
			TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
			TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
			TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
			TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
		}
	}

	// Restore data
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_set(GROUP_ID_TEST_OK, tk, &test_data[tk], sizeof(test_data[tk])));
	}
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
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
			TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
		} else {
			TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
			TEST_ASSERT_EQUAL(sizeof(test_data[tk]), len);
			TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
			TEST_ASSERT_EQUAL(test_read_data, test_data[tk]);
		}
	}

	// another keys
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_delete(GROUP_ID_TEST_OK, tk));
	}
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
	}
}

void test_sid_pal_storage_erase_all_2(void)
{
	test_read_data = 0;
	uint32_t len;

	// Check if 'test_key_x' key still exist
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
	TEST_ASSERT_EQUAL(sizeof(test_data_buffer), len);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, test_key_x, &test_read_data, sizeof(test_read_data)));

	// Erase whole NVM
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(GROUP_ID_TEST_OK));
	for (test_keys_t tk = 0; tk < TEST_KEY_END; tk++) {
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get(GROUP_ID_TEST_OK, tk, &test_read_data, sizeof(test_read_data)));
		TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, tk, &len));
	}
	TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get_len(GROUP_ID_TEST_OK, test_key_x, &len));
}

/******************************************************************
* sid_pal_mfg_store_ifc
* NOTE: DO NOT CHANGE THE ORDER OF THE TESTS!
* ****************************************************************/
static void hton_buff(uint8_t *buffer, size_t buff_len)
{
	uint32_t val_l = 0;
	size_t i = 0;
	uint8_t mod_len = buff_len % sizeof(uint32_t);

	if (sizeof(uint32_t) <= buff_len) {
		for (i = 0; i < (buff_len - mod_len); i += sizeof(uint32_t)) {
			memcpy(&val_l, &buffer[i], sizeof(val_l));
			val_l = sys_cpu_to_be32(val_l);
			memcpy(&buffer[i], &val_l, sizeof(val_l));
		}
	}
	// if 1 == mod_len then last byte shall be copied directly do destination buffer.
	if (2 == mod_len) {
		uint16_t val_s = 0;
		memcpy(&val_s, &buffer[i], sizeof(val_s));
		val_s = sys_cpu_to_be16(val_s);
		memcpy(&buffer[i], &val_s, sizeof(val_s));
	} else if (3 == mod_len) {
		val_l = 0;
		memcpy(&val_l, &buffer[i], 3 * sizeof(uint8_t));
		val_l = sys_cpu_to_be24(val_l);
		memcpy(&buffer[i], &val_l, 3 * sizeof(uint8_t));
	}
}

static void mfg_set_version(uint32_t version)
{
	uint8_t write_buff[SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN];

	memcpy(write_buff, &version, SID_PAL_MFG_STORE_VERSION_SIZE);
	TEST_ASSERT_EQUAL(0, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff, SID_PAL_MFG_STORE_VERSION_SIZE));
	version = sys_cpu_to_be32(version);
	TEST_ASSERT_EQUAL(version, sid_pal_mfg_store_get_version());
}

static void mfg_clr_memory(void)
{
	TEST_ASSERT_EQUAL(0, sid_pal_mfg_store_erase());
	TEST_ASSERT_TRUE(sid_pal_mfg_store_is_empty());
}

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
		.addr_start = MFG_START_OFFSET,
		.addr_end = MFG_END_OFFSET,
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

	mfg_set_version(MFG_VERSION_1_VAL);
}

void test_sid_pal_mfg_storage_read(void)
{
	uint32_t version = MFG_VERSION_1_VAL;
	uint8_t read_buff[64];

	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_VERSION, read_buff, SID_PAL_MFG_STORE_VERSION_SIZE);
	TEST_ASSERT_EQUAL_MEMORY(&version, read_buff, SID_PAL_MFG_STORE_VERSION_SIZE);
}

void test_sid_pal_mfg_storage_erase(void)
{
	TEST_ASSERT_FALSE(sid_pal_mfg_store_is_empty());
	mfg_clr_memory();
}

void test_sid_pal_mfg_storage_dev_id_get(void)
{
	uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE];
	uint8_t fake_dev_id[8] = { 0xAC, 0xBC, 0xCC, 0xDC,
				   0x11, 0x12, 0x13, 0x14 };

	memset(dev_id, 0x00, sizeof(dev_id));
	mfg_clr_memory();

	TEST_ASSERT_FALSE(sid_pal_mfg_store_dev_id_get(dev_id));
	TEST_ASSERT_EQUAL(0xBF, dev_id[0]);
	// dev id is unique for every chip, so we cannot verify it in another way:
	TEST_ASSERT_NOT_EQUAL(0x00, dev_id[1]);
	TEST_ASSERT_NOT_EQUAL(0x00, dev_id[2]);
	TEST_ASSERT_NOT_EQUAL(0x00, dev_id[3]);
	TEST_ASSERT_NOT_EQUAL(0x00, dev_id[4]);

	memset(dev_id, 0x00, sizeof(dev_id));
	// Set fake dev_id.
	TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_DEVID, fake_dev_id, sizeof(fake_dev_id)));
}

void test_sid_pal_mfg_storage_sn_get(void)
{
	uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE];
	uint8_t fake_serial_num[20];

	for (int cnt = 0; cnt < sizeof(fake_serial_num); cnt++) {
		fake_serial_num[cnt] = cnt;
	}

	mfg_clr_memory();

	// No serial number.
	TEST_ASSERT_FALSE(sid_pal_mfg_store_serial_num_get(serial_num));

	// Set fake serial number.
	TEST_ASSERT_EQUAL(0, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_SERIAL_NUM, fake_serial_num, sizeof(fake_serial_num)));
	TEST_ASSERT_TRUE(sid_pal_mfg_store_serial_num_get(serial_num));
	TEST_ASSERT_EQUAL_HEX8_ARRAY(fake_serial_num, serial_num, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

	// Set version to MFG_VERSION_1_VAL
	mfg_set_version(MFG_VERSION_1_VAL);
	TEST_ASSERT_TRUE(sid_pal_mfg_store_serial_num_get(serial_num));
	hton_buff(serial_num, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(fake_serial_num, serial_num, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
}

/******************************************************************
* sid_pal_timer_ifc
* ****************************************************************/
static void timer_callback(void *arg, sid_pal_timer_t *originator)
{
	callback_arg = *((int *)arg);
	++timer_callback_cnt;
}

static void timer_init(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_callback, &test_timer_arg));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(&test_timer_arg, test_timer.callback_arg);
	TEST_ASSERT_EQUAL(timer_callback, test_timer.callback);
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
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_init(p_null_timer, timer_callback, NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_timer_deinit(p_null_timer));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_init(&test_timer, timer_callback, NULL));
	TEST_ASSERT_TRUE(test_timer.is_initialized);
	TEST_ASSERT_NULL(test_timer.callback_arg);
	TEST_ASSERT_FALSE(test_timer.is_periodic);
	TEST_ASSERT_EQUAL(0, test_timer.is_armed);
	TEST_ASSERT_EQUAL(timer_callback, test_timer.callback);
	TEST_ASSERT_EQUAL(SID_ERROR_ALREADY_INITIALIZED, sid_pal_timer_init(&test_timer, timer_callback, NULL));

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
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

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

void test_sid_pal_timer_one_shot_50usec(void)
{
	/**
	 * Scenario 1:
	 * 	In this case, we expect that callback will be executed just once after 50 usec.
	 */
	struct sid_timespec when = { .tv_nsec = 50 * NSEC_PER_USEC };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	// It should be enough time
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_periodically_execute_callback(void)
{
	/**
	 * Scenario 2:
	 * 	In this case, we expect that callback will be executed many times and still working.
	 */
	struct sid_timespec when = { .tv_nsec = 50 * NSEC_PER_USEC };
	struct sid_timespec period = { .tv_nsec = 500 };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	// It should be enough time
	k_sleep(K_MSEC(100));
	TEST_ASSERT_TRUE(10 < timer_callback_cnt);
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_cancel_before_it_expire(void)
{
	/**
	 * Scenario 3:
	 * 	Cancel timer before it expires.
	 */
	struct sid_timespec when = { .tv_nsec = (150 * NSEC_PER_MSEC) };
	struct sid_timespec period = { .tv_nsec = 500 };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	k_sleep(K_MSEC(100));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	// Arm the timer again, but now with short period
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	k_sleep(K_MSEC(100));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_cancel_after_it_expired(void)
{
	/**
	 * Scenario 4:
	 * 	Cancel timer after first expired.
	 */
	struct sid_timespec when = { .tv_nsec = (10 * NSEC_PER_MSEC) };
	struct sid_timespec period = { .tv_nsec = (40 * NSEC_PER_MSEC)  };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	k_sleep(K_MSEC(15));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_cancel(&test_timer));
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));

	timer_deinit();
}

void test_sid_pal_timer_deinit_after_it_expired(void)
{
	/**
	 * Scenario 5:
	 * 	Deinit timer after first expired.
	 */
	struct sid_timespec when = { .tv_nsec = (10 * NSEC_PER_MSEC) };
	struct sid_timespec period = { .tv_nsec = (40 * NSEC_PER_MSEC)  };

	timer_callback_cnt = 0;

	timer_init();

	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, &period));
	TEST_ASSERT_EQUAL(0, timer_callback_cnt);
	k_sleep(K_MSEC(15));
	TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_deinit(&test_timer));
	k_sleep(K_MSEC(100));
	TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
}

void test_sid_pal_timer_one_shot_few_times(void)
{
	/**
	 * Scenario 6:
	 * 	One-shot timer executed few times with argument;
	 */
	struct sid_timespec when = { .tv_nsec = (10 * NSEC_PER_MSEC) };

	timer_callback_cnt = 0;
	test_timer_arg = 55;

	timer_init();

	for (int cnt = 0; cnt < 8; cnt++) {
		timer_callback_cnt = 0;
		test_timer_arg += cnt;
		TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_timer_arm(&test_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &when, NULL));
		TEST_ASSERT_EQUAL(0, timer_callback_cnt);
		TEST_ASSERT_TRUE(sid_pal_timer_is_armed(&test_timer));
		k_sleep(K_MSEC(50));
		TEST_ASSERT_EQUAL(test_timer_arg, callback_arg);
		TEST_ASSERT_FALSE(sid_pal_timer_is_armed(&test_timer));
		TEST_ASSERT_EQUAL(1, timer_callback_cnt);
	}

	timer_deinit();
}

/******************************************************************
* sid_pal_uptime_ifc
* ****************************************************************/
void test_sid_pal_uptime_get(void)
{
	struct sid_timespec sid_time = { 0 };
	int64_t uptime_msec = k_uptime_get();
	uint32_t uptime_sec = (uint32_t)(uptime_msec / MSEC_PER_SEC);
	uint32_t uptime_nsec = (uint32_t)((uptime_msec * NSEC_PER_MSEC) % NSEC_PER_SEC);

	TEST_ASSERT_NOT_EQUAL_MESSAGE(0, uptime_msec, "Test data preparation failed.");

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_uptime_now(&sid_time));

	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(uptime_sec, sid_time.tv_sec);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(uptime_nsec, sid_time.tv_nsec);
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

void main(void)
{
	test_sid_pal_storage_init();
	(void)unity_main();
}
