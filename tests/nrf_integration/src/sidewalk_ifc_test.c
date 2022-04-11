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
#include <sid_pal_crypto_ifc.h>
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

// Crypto
#define RNG_BUFF_MAX_SIZE       (128)

#define SHA256_LEN                      (32)
#define SHA512_LEN                      (64)
#define SHA_MAX_DIGEST_LEN              (SHA512_LEN)

#define HMAC_MAX_BLOCK_SIZE             (32)

#define HASH_TEST_DATA_BLOCK_SIZE       (128)
#define AES_MAX_BLOCK_SIZE              (16)
#define AES_TEST_DATA_BLOCK_SIZE        (128)
#define HMAC_TEST_DATA_BLOCK_SIZE       (128)

#define AES_IV_SIZE                     (16)
#define AES_GCM_IV_SIZE                 (12)
#define AES_CCM_IV_SIZE                 (13)

#define EC_ED25519_PRIV_KEY_LEN         (32)
#define EC_CURVE25519_PRIV_KEY_LEN      (32)
#define EC_SECP256R1_PRIV_KEY_LEN       (32)
#define ECC_PRIVATE_KEY_MAX_LEN         (EC_SECP256R1_PRIV_KEY_LEN)

#define EC_ED25519_PUB_KEY_LEN          (32)
#define EC_CURVE25519_PUB_KEY_LEN       (32)
#define EC_SECP256R1_PUB_KEY_LEN        (64)
#define ECC_PUBLIC_KEY_MAX_LEN          (EC_SECP256R1_PUB_KEY_LEN)

#define ECDH_SECRET_SIZE                (32)

#define ECDSA_SIGNATURE_SIZE            (64)

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

static const uint8_t test_string[AES_TEST_DATA_BLOCK_SIZE] = {
	"Nordic Semiconductor is a Norwegian fabless semiconductor company specializing in ... "
};

uint8_t aes_128_test_key[AES_MAX_BLOCK_SIZE] = { 0xAC, 0x1D, 0x05, 0x22, 0xAC, 0x1D, 0x05, 0x22,
						 0xFA, 0xD4, 0xCC, 0x29, 0xFA, 0xD4, 0xCC, 0x29 };
// In this key one byte is different
uint8_t aes_128_test_fake_key[AES_MAX_BLOCK_SIZE] = { 0xAC, 0x1D, 0x09, 0x22, 0xAC, 0x1D, 0x05, 0x22,
						      0xFA, 0xD4, 0xCC, 0x29, 0xFA, 0xD4, 0xCC, 0x29 };

uint8_t hmac_test_key[HMAC_MAX_BLOCK_SIZE] = { 0xAC, 0x1D, 0x05, 0x22, 0xAC, 0x1D, 0x05, 0x22,
					       0xFA, 0xD4, 0xCC, 0x29, 0xFA, 0xD4, 0xCC, 0x29,
					       0xDA, 0x3C, 0xEE, 0xA4, 0x82, 0x0D, 0xAA, 0x50,
					       0xAC, 0xFE, 0xBB, 0x34, 0x1D, 0x05, 0x22, 0xAC };

uint8_t hmac_test_fake_key[HMAC_MAX_BLOCK_SIZE] = { 0xAC, 0x1D, 0x05, 0x22, 0xbb, 0x1D, 0x05, 0x22,
						    0xFA, 0xD4, 0xCC, 0x29, 0xFA, 0xD4, 0xCC, 0x29,
						    0xaa, 0x3C, 0xEE, 0xA4, 0x82, 0x0D, 0xAA, 0x50,
						    0xAC, 0xFE, 0xBB, 0x34, 0x1D, 0x05, 0x22, 0xAC };

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

/******************************************************************
* sid_pal_crypto_ifc
* ****************************************************************/
void test_sid_pal_crypto_no_init(void)
{
	// For any arguments functions shall return the same error code
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_deinit());
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_ecdh(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_key_gen(NULL));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// And now we expect that error code will be different like SID_ERROR_UNINITIALIZED
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_ecdh(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_key_gen(NULL));
}

void test_sid_pal_crypto_deinit(void)
{
	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// We expect that error code will be different like SID_ERROR_UNINITIALIZED
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_ecdh(NULL));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_key_gen(NULL));

	// Deinitialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_deinit());

	// For any arguments functions shall return the same error code
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_hmac(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_ecdh(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_UNINITIALIZED, sid_pal_crypto_ecc_key_gen(NULL));
}

void test_sid_pal_crypto_rng(void)
{
	uint8_t tmp_buff[RNG_BUFF_MAX_SIZE];
	uint8_t rng_buff[RNG_BUFF_MAX_SIZE];
	size_t rng_test_len[] = { 8, 16, 24, 32, 64, sizeof(rng_buff) };

	memset(tmp_buff, 0x00, sizeof(tmp_buff));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_rand(NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_rand(NULL, 5));

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_rand(rng_buff, 0));

	// Check for any length from 1 to RNG_BUFF_MAX_SIZE
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_rand(rng_buff, 1));
	TEST_ASSERT_NOT_EQUAL(0, rng_buff[0]);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_rand(rng_buff, 3));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(rng_buff, tmp_buff, 3));

	for (int cnt = 0; cnt < ARRAY_SIZE(rng_test_len); cnt++) {
		memset(tmp_buff, 0x00, sizeof(tmp_buff));
		// Generate random values and check if those are different in time
		for (int test_it = 0; test_it < 5; test_it++) {
			TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_rand(rng_buff, rng_test_len[cnt]));
			TEST_ASSERT_NOT_EQUAL(0, memcmp(rng_buff, tmp_buff, rng_test_len[cnt]));
			k_sleep(K_NSEC(50)); // additional time for entropy update
			// Save last random value
			memcpy(tmp_buff, rng_buff, rng_test_len[cnt]);
		}
	}
}

void test_sid_pal_crypto_hash(void)
{
	sid_pal_hash_params_t params;

	// SHA must be calculated from RAM, in another case result can be unpredicted.
	uint8_t data_copy[HASH_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	// SHA for different lengths - calculated in openssl
	size_t test_vector_buf_len[] = { 8, 16, 32, 64, 57, 128 };

	// SHA2_256
	uint8_t test_data_sha_256_computed[][SHA256_LEN] = {
		{ 0xe3, 0xb4, 0x38, 0x27, 0x59, 0x7c, 0x6e, 0x6f, 0xaf, 0x3d, 0x7f, 0x63, 0x97, 0xa4, 0x4c, 0x1d,
		  0xc7, 0x31, 0xf8, 0x25, 0x79, 0x27, 0xf3, 0xc1, 0xa6, 0xfb, 0x5a, 0x37, 0xb2, 0x42, 0x7f, 0x7f },
		{ 0xf9, 0xca, 0x02, 0x11, 0x50, 0x30, 0x4b, 0x29, 0x7a, 0x45, 0xac, 0xb3, 0x3a, 0x3c, 0xaf, 0x4c,
		  0x1c, 0xe7, 0x13, 0xa1, 0xa1, 0xb2, 0x3d, 0x0e, 0x70, 0x3d, 0x94, 0x57, 0x74, 0x83, 0xe6, 0xd0 },
		{ 0x69, 0xd9, 0x11, 0x8f, 0x79, 0x61, 0xa3, 0x9a, 0x8a, 0x07, 0x94, 0xe0, 0xa4, 0xe4, 0x52, 0xfe,
		  0xba, 0xb2, 0x00, 0x67, 0x22, 0x5a, 0xa0, 0x7c, 0xfc, 0x1f, 0xf9, 0x27, 0x9d, 0x85, 0xb9, 0xe9 },
		{ 0xe8, 0x7b, 0x4a, 0xa1, 0x34, 0x3d, 0x70, 0x88, 0xa4, 0x64, 0x6e, 0xb8, 0xa0, 0xee, 0x2a, 0xa0,
		  0x10, 0x37, 0xbc, 0x7a, 0x29, 0x58, 0xd8, 0x86, 0x40, 0xa6, 0x22, 0xbc, 0x36, 0x32, 0x39, 0x51 },
		{ 0xe9, 0x85, 0x7a, 0x27, 0xa5, 0x96, 0x48, 0xbe, 0x7d, 0x07, 0x31, 0x5b, 0xc7, 0x4a, 0xc6, 0xf8,
		  0x0d, 0x54, 0xec, 0x47, 0xeb, 0x65, 0x13, 0x8c, 0xe8, 0xe7, 0x1f, 0x34, 0x7f, 0xeb, 0xee, 0x51 },
		{ 0x12, 0x94, 0x2a, 0xde, 0x61, 0x34, 0xc9, 0x88, 0xc3, 0xf0, 0xf5, 0x12, 0x34, 0xca, 0x83, 0xe8,
		  0x00, 0x05, 0x50, 0x9a, 0x22, 0xb5, 0x9a, 0x90, 0x93, 0x59, 0xf0, 0x03, 0xed, 0x0f, 0xe1, 0x74 }
	};
	// SHA2_512
	uint8_t test_data_sha_512_computed[][SHA512_LEN] = {
		{ 0x81, 0x59, 0x26, 0xf9, 0x3e, 0xd5, 0xa2, 0xa4, 0x3f, 0x08, 0xee, 0x95, 0xe0, 0xf2, 0x01, 0x46,
		  0xc8, 0xdc, 0xa6, 0x09, 0xa2, 0x73, 0x8b, 0x97, 0x6a, 0xa7, 0x97, 0xf3, 0x67, 0x29, 0xd2, 0x88,
		  0x6c, 0xcf, 0x70, 0x87, 0xa2, 0x67, 0x34, 0xac, 0xed, 0x8d, 0x60, 0x45, 0xeb, 0xc4, 0x5a, 0x3f,
		  0xcb, 0x18, 0xe9, 0xe0, 0xac, 0x48, 0x37, 0x03, 0xcb, 0xc0, 0xa3, 0x47, 0x5d, 0x72, 0xaa, 0x6e },
		{ 0xfe, 0xa5, 0xe2, 0x3d, 0x05, 0xe7, 0x59, 0x78, 0xf8, 0xbf, 0xa1, 0x5c, 0xdc, 0x19, 0xeb, 0x0d,
		  0xba, 0xd0, 0x8d, 0x38, 0xb0, 0x58, 0x9b, 0xbc, 0x8b, 0x13, 0x9c, 0x26, 0xf7, 0xdf, 0x63, 0x27,
		  0xe8, 0x3c, 0xef, 0xd9, 0x60, 0xa6, 0x73, 0xe7, 0x97, 0x62, 0x8f, 0x01, 0xe7, 0x93, 0x00, 0x31,
		  0x5e, 0xb0, 0xe6, 0x7b, 0xdf, 0xb1, 0xf6, 0xb2, 0xa2, 0x6f, 0x04, 0x62, 0x3d, 0xaf, 0xca, 0xef },
		{ 0x7e, 0x67, 0xab, 0x1a, 0x2d, 0xa9, 0x49, 0x28, 0x35, 0x5b, 0xa5, 0xc7, 0x39, 0x2b, 0xe3, 0x62,
		  0x64, 0xbd, 0xf0, 0x4b, 0x0d, 0x9f, 0x48, 0xa8, 0xf3, 0xf5, 0x30, 0xb6, 0x01, 0x3b, 0x01, 0x46,
		  0xad, 0x08, 0xb6, 0x9b, 0xbe, 0x95, 0x05, 0x84, 0x84, 0xb7, 0x6c, 0xff, 0xf8, 0xf0, 0x04, 0x45,
		  0xf8, 0x7d, 0x4d, 0xf5, 0x2f, 0x01, 0xed, 0x5f, 0x05, 0x99, 0x1f, 0x3c, 0xea, 0xe4, 0x80, 0x3a },
		{ 0x60, 0x07, 0x3d, 0xdc, 0x1b, 0x0e, 0xf9, 0x9b, 0xb9, 0x15, 0xca, 0xce, 0xdc, 0x27, 0x8d, 0xd6,
		  0x55, 0xc5, 0x4b, 0x4d, 0x7f, 0x56, 0x9a, 0xb5, 0x25, 0x07, 0x80, 0x47, 0x1b, 0xb1, 0x69, 0xbd,
		  0x65, 0x48, 0xe8, 0xf5, 0xf9, 0xda, 0xc3, 0xd3, 0x87, 0xfb, 0x62, 0x1d, 0x74, 0x39, 0xe5, 0x36,
		  0xec, 0x7d, 0x24, 0xa3, 0xdb, 0x65, 0x09, 0xc3, 0x5b, 0x98, 0xf5, 0x23, 0xcd, 0xb0, 0xeb, 0xc1 },
		{ 0xbb, 0x66, 0xb1, 0xb2, 0x48, 0xed, 0xad, 0xae, 0x2a, 0x11, 0x4a, 0x30, 0x8e, 0xe8, 0x06, 0xff,
		  0x50, 0x1c, 0xd6, 0x75, 0x4e, 0xc5, 0x89, 0x30, 0x64, 0xb2, 0xb3, 0x7f, 0x32, 0x01, 0xba, 0x57,
		  0x9f, 0x2b, 0x42, 0x40, 0xe5, 0x39, 0x8f, 0x9a, 0xe5, 0x9a, 0x8f, 0xe8, 0xfc, 0xaa, 0x0f, 0x1a,
		  0x5e, 0x27, 0xfb, 0xe2, 0x95, 0x59, 0x93, 0x8d, 0x6e, 0x47, 0xda, 0x19, 0x22, 0x1a, 0xd3, 0xf2 },
		{ 0x62, 0x0e, 0x29, 0x5c, 0x53, 0xf9, 0xb6, 0xa8, 0x5b, 0x2e, 0x52, 0x4b, 0xe9, 0xc5, 0x42, 0x57,
		  0x0e, 0x1e, 0xb2, 0x5b, 0x44, 0xa3, 0x00, 0x34, 0xf4, 0xc2, 0x78, 0x6a, 0x7d, 0xd4, 0x93, 0x17,
		  0xd2, 0xcc, 0xee, 0xc2, 0xcd, 0x2a, 0xb9, 0xd5, 0xa2, 0xef, 0x35, 0x20, 0x08, 0xed, 0x20, 0xf1,
		  0x66, 0x85, 0x09, 0xc3, 0x1f, 0x4d, 0xdc, 0xab, 0x26, 0x73, 0x02, 0x0a, 0x6c, 0x72, 0x7d, 0xd0 }
	};

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(digest, 0x00, sizeof(digest));
	memcpy(data_copy, test_string, sizeof(data_copy));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// NULL pointer
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hash(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hash(&params));

	params.data = data_copy;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hash(&params));

	params.data = NULL;
	params.digest = digest;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_hash(&params));

	// Invalid arguments
	params.data = data_copy;
	params.digest = digest;
	params.data_size = 0;
	params.digest_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hash(&params));

	params.data_size = sizeof(data_copy);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hash(&params));

	params.data_size = 0;
	params.digest_size = sizeof(digest);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_hash(&params));

	params.data = data_copy;
	params.data_size = sizeof(data_copy);
	params.digest = digest;
	params.digest_size = SHA256_LEN;

	// Unsupported algorithms
	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_hash(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_hash(&params));

	// Compute SHA2_256
	params.algo = SID_PAL_HASH_SHA256;
	params.data = data_copy;
	params.data_size = sizeof(data_copy);
	params.digest = digest;
	params.digest_size = SHA256_LEN;

	// Verify test condition
	TEST_ASSERT_EQUAL(ARRAY_SIZE(test_vector_buf_len), ARRAY_SIZE(test_data_sha_256_computed));
	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_buf_len); test_it++) {
		params.data_size = test_vector_buf_len[test_it];
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hash(&params));
		TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data_sha_256_computed[test_it], digest, SHA256_LEN);
	}

	// Huge (random) data package from RAM
	params.data_size = 1024;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hash(&params));

	// Too small out buffer
	params.digest_size = SHA256_LEN / 2;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_hash(&params));

	// Compute SHA2_512
	params.algo = SID_PAL_HASH_SHA512;
	params.data = data_copy;
	params.data_size = sizeof(data_copy);
	params.digest = digest;
	params.digest_size = SHA512_LEN;

	// Verify test condition
	TEST_ASSERT_EQUAL(ARRAY_SIZE(test_vector_buf_len), ARRAY_SIZE(test_data_sha_512_computed));
	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_buf_len); test_it++) {
		params.data_size = test_vector_buf_len[test_it];
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hash(&params));
		TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data_sha_512_computed[test_it], digest, SHA512_LEN);
	}

	// Big (random) data package from RAM
	params.data_size = 1024;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hash(&params));
}

/**********************************************
* HMAC
* ********************************************/
void test_sid_pal_crypto_hmac_invalid_args(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data_copy[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	// Test precondition
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(digest, 0x00, sizeof(digest));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// SHA2_256
	params.algo = SID_PAL_HASH_SHA256;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data_copy;
	params.data_size = sizeof(data_copy);
	params.digest = digest;

	// To small output buffer.
	params.digest_size = SHA256_LEN / 2;

	// PSA Crypto API returns this error code.
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_hmac(&params));
}

void test_sid_pal_crypto_hmac_sha256(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data_copy[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];
	size_t test_vector_data_in_len[] = { 8, 16, 24, 55, 64, HMAC_TEST_DATA_BLOCK_SIZE };

	// openssl dgst -sha256 -mac hmac -macopt hexkey:AC1D0522AC1D0522FAD4CC29FAD4CC29DA3CEEA4820DAA50ACFEBB341D0522AC
	uint8_t openssl_test_hmac_sha256_vector[][SHA256_LEN] = {
		{ 0x30, 0xdb, 0x3a, 0xff, 0x87, 0xd8, 0x45, 0x95, 0x22, 0x3d, 0x8e, 0x98, 0x59, 0xd1, 0x2f, 0xdb,
		  0x9f, 0x5c, 0xb7, 0xae, 0xcf, 0x00, 0x03, 0xb2, 0xed, 0x68, 0x67, 0x7b, 0xea, 0x12, 0xdd, 0xf8 },

		{ 0x16, 0x62, 0x3b, 0xfa, 0xd5, 0x84, 0x32, 0x70, 0xc4, 0x61, 0x07, 0xd5, 0xa8, 0xe4, 0x65, 0xd4,
		  0xee, 0x55, 0xe3, 0x48, 0xec, 0xe0, 0x10, 0x2e, 0x3d, 0x8b, 0x44, 0x07, 0x2d, 0x25, 0x0d, 0x6f },

		{ 0x33, 0x06, 0x2d, 0x41, 0x7c, 0x0d, 0x9b, 0x89, 0xa5, 0xdb, 0x65, 0x10, 0xe5, 0xf2, 0x03, 0xf0,
		  0x81, 0x03, 0x16, 0x3a, 0x98, 0x2b, 0x44, 0xaa, 0x78, 0x3e, 0x81, 0x69, 0xc4, 0x94, 0x1d, 0xd4 },

		{ 0x7e, 0x11, 0x96, 0x0f, 0x2a, 0x82, 0x33, 0xae, 0x4a, 0x67, 0x59, 0xb0, 0x82, 0xdf, 0x71, 0xca,
		  0xcb, 0x0a, 0x9d, 0x1f, 0x84, 0xed, 0x22, 0x07, 0x5d, 0xfb, 0xce, 0x2d, 0x27, 0x86, 0xb3, 0x16 },

		{ 0x76, 0x8c, 0x73, 0x53, 0xaf, 0x39, 0x75, 0xe6, 0x17, 0x1a, 0x82, 0x42, 0xbe, 0xfa, 0x7d, 0xda,
		  0x92, 0x4a, 0x29, 0x3e, 0xa0, 0xa2, 0xc2, 0x90, 0x60, 0x02, 0x21, 0x68, 0x4c, 0xa2, 0x75, 0xc3 },

		{ 0xd2, 0x04, 0xb5, 0x7a, 0x7d, 0x20, 0x31, 0xf7, 0x83, 0x4c, 0x20, 0xea, 0x35, 0x77, 0x3d, 0x60,
		  0xf6, 0x0d, 0x26, 0xa3, 0xc7, 0x5c, 0xe9, 0xd3, 0xc9, 0x16, 0x63, 0x3e, 0x08, 0x48, 0x69, 0x7c },
	};

	// Test precondition
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(digest, 0x00, sizeof(digest));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// SHA2_256
	params.algo = SID_PAL_HASH_SHA256;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data_copy;
	params.digest = digest;
	params.digest_size = SHA256_LEN;

	// Verify test conditions
	TEST_ASSERT_EQUAL(ARRAY_SIZE(openssl_test_hmac_sha256_vector), ARRAY_SIZE(test_vector_data_in_len));
	// Test algorithm with variable data length
	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_data_in_len); test_it++) {
		memset(digest, 0x00, sizeof(digest));

		params.data_size = test_vector_data_in_len[test_it];
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
		TEST_ASSERT_EQUAL_UINT8_ARRAY(openssl_test_hmac_sha256_vector[test_it], digest, SHA256_LEN);
	}
}

void test_sid_pal_crypto_hmac_sha256_fake_key(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data_copy[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	// openssl dgst -sha256 -mac hmac -macopt hexkey:AC1D0522AC1D0522FAD4CC29FAD4CC29DA3CEEA4820DAA50ACFEBB341D0522AC
	uint8_t openssl_test_hmac_sha256_vector[] = {
		0xd2, 0x04, 0xb5, 0x7a, 0x7d, 0x20, 0x31, 0xf7, 0x83, 0x4c, 0x20, 0xea, 0x35, 0x77, 0x3d, 0x60,
		0xf6, 0x0d, 0x26, 0xa3, 0xc7, 0x5c, 0xe9, 0xd3, 0xc9, 0x16, 0x63, 0x3e, 0x08, 0x48, 0x69, 0x7c
	};

	// Test precondition
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(digest, 0x00, sizeof(digest));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// SHA2_256
	params.algo = SID_PAL_HASH_SHA256;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data_copy;
	params.data_size = HMAC_TEST_DATA_BLOCK_SIZE;
	params.digest = digest;
	params.digest_size = SHA256_LEN;

	// With proper key
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(openssl_test_hmac_sha256_vector, digest, SHA256_LEN);

	// The same data with fake key (digest must be different)
	params.key = hmac_test_fake_key;
	params.key_size = sizeof(hmac_test_fake_key);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(openssl_test_hmac_sha256_vector, digest, SHA256_LEN));
}

void test_sid_pal_crypto_hmac_sha512(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data_copy[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	size_t test_vector_data_in_len[] = { 8, 16, 24, 55, 64, HMAC_TEST_DATA_BLOCK_SIZE };

	uint8_t openssl_test_hmac_sha512_vector[][SHA512_LEN] = {
		{ 0xed, 0x3e, 0xcd, 0xc5, 0x14, 0x40, 0xee, 0x98, 0xb7, 0x26, 0x5c, 0xa5, 0xa6, 0xc7, 0x74, 0x39,
		  0xf5, 0x0d, 0x9a, 0x54, 0xff, 0x56, 0xe7, 0x2c, 0x10, 0x0e, 0x76, 0xba, 0xe4, 0x53, 0x13, 0x40,
		  0x1c, 0xa9, 0xf7, 0x20, 0x97, 0x28, 0xeb, 0x20, 0xd8, 0x11, 0x46, 0x91, 0x7d, 0x95, 0x0c, 0x94,
		  0x96, 0x13, 0x14, 0x53, 0x9d, 0x00, 0xa6, 0x7b, 0xf7, 0xa7, 0x29, 0xbd, 0x02, 0x53, 0x5f, 0x05 },

		{ 0xf3, 0xac, 0xf5, 0xeb, 0xde, 0x0b, 0x70, 0x07, 0x6a, 0xf7, 0x7e, 0xe7, 0xf1, 0xf6, 0x6e, 0x9a,
		  0xad, 0x2d, 0x94, 0xaf, 0xa1, 0x35, 0x16, 0xca, 0x77, 0xba, 0x55, 0x41, 0x8f, 0x4c, 0x11, 0x1d,
		  0x99, 0xaa, 0x3d, 0x68, 0x56, 0xe2, 0x36, 0xa9, 0x1b, 0x1f, 0x09, 0x4a, 0x5f, 0x9c, 0xf5, 0x0f,
		  0x71, 0x6d, 0xb2, 0x53, 0xb6, 0xd4, 0x53, 0x6b, 0x0a, 0xe3, 0x4b, 0x18, 0x7e, 0x6a, 0x9d, 0x7f },

		{ 0xfd, 0x13, 0xdc, 0x69, 0x60, 0x35, 0xc0, 0xef, 0x29, 0x35, 0x21, 0x37, 0xbd, 0xf4, 0x38, 0x8f,
		  0xdf, 0x0e, 0x94, 0x0e, 0xa1, 0x03, 0xcf, 0xfb, 0x60, 0xdf, 0x72, 0xec, 0xca, 0x6b, 0x31, 0xe8,
		  0xe5, 0x45, 0x29, 0xe0, 0xae, 0x07, 0xa4, 0xcd, 0xad, 0xcc, 0xb9, 0x44, 0xfe, 0x44, 0xbe, 0xf3,
		  0xf8, 0x6f, 0xe3, 0x72, 0x32, 0x82, 0x4d, 0x11, 0xb0, 0xf6, 0xc9, 0x4b, 0x37, 0x65, 0x09, 0x23 },

		{ 0xa3, 0x55, 0x6e, 0x89, 0x40, 0xf4, 0x78, 0x9d, 0xce, 0x62, 0xd6, 0x99, 0x1c, 0x6b, 0x2b, 0x80,
		  0xe0, 0x72, 0xae, 0x37, 0x72, 0x8f, 0x8b, 0x8c, 0x28, 0x4c, 0xc2, 0x76, 0x05, 0x25, 0x47, 0x18,
		  0x7f, 0x09, 0x95, 0x56, 0x71, 0x71, 0x4c, 0xf8, 0x54, 0x68, 0x1c, 0x9e, 0x2c, 0xd4, 0x42, 0xbc,
		  0xa5, 0xf2, 0x9c, 0xbf, 0xb2, 0xf8, 0x1c, 0xf1, 0x25, 0x9a, 0xd6, 0x90, 0x31, 0x55, 0x24, 0xcb },

		{ 0x58, 0xca, 0x39, 0x1f, 0x93, 0xb9, 0xee, 0x92, 0xbc, 0xe0, 0xff, 0xc6, 0x4d, 0xf0, 0x27, 0x1b,
		  0x7f, 0x52, 0xb1, 0xf6, 0xe9, 0xeb, 0xe7, 0x9e, 0x2c, 0xb5, 0x43, 0x58, 0xbb, 0x8d, 0xc2, 0xc2,
		  0x11, 0x07, 0x78, 0x89, 0x2c, 0xc8, 0x12, 0x50, 0x3a, 0x4b, 0x3b, 0x8c, 0x80, 0xb8, 0xca, 0xa1,
		  0x24, 0x03, 0x4d, 0xce, 0x64, 0x71, 0xd3, 0x60, 0x2d, 0x12, 0xf2, 0xb6, 0x23, 0xff, 0xc3, 0xf3 },

		{ 0x39, 0xcc, 0x0f, 0x33, 0xc5, 0x9e, 0xfa, 0xe8, 0x9f, 0xf2, 0x29, 0x5f, 0x92, 0x67, 0xe0, 0xb3,
		  0x7e, 0x9b, 0x0d, 0x92, 0xfd, 0x50, 0x65, 0x06, 0x1f, 0x5b, 0xed, 0x21, 0x42, 0x5c, 0x7a, 0x1f,
		  0xd7, 0xa5, 0xd8, 0xb3, 0x49, 0x8b, 0x36, 0xbf, 0x35, 0xf3, 0x6d, 0x41, 0xe1, 0x28, 0xf3, 0xeb,
		  0x61, 0xc2, 0x14, 0x77, 0x47, 0x2a, 0x8e, 0x39, 0x14, 0xbc, 0xf6, 0xc3, 0xa9, 0xdf, 0xeb, 0x0a },
	};

	TEST_IGNORE_MESSAGE("Not supported yet. Waiting for support in nRF Crypto.");

	// Test precondition
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(digest, 0x00, sizeof(digest));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// SHA2_512
	params.algo = SID_PAL_HASH_SHA512;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data_copy;
	params.digest = digest;
	params.digest_size = SHA512_LEN;

	// Verify test conditions
	TEST_ASSERT_EQUAL(ARRAY_SIZE(openssl_test_hmac_sha512_vector), ARRAY_SIZE(test_vector_data_in_len));
	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_data_in_len); test_it++) {
		memset(digest, 0x00, sizeof(digest));

		params.data_size = test_vector_data_in_len[test_it];
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
		TEST_ASSERT_EQUAL_UINT8_ARRAY(openssl_test_hmac_sha512_vector[test_it], digest, SHA512_LEN);
	}
}

void test_sid_pal_crypto_hmac_sha512_fake_key(void)
{
	sid_pal_hmac_params_t params;
	uint8_t data_copy[HMAC_TEST_DATA_BLOCK_SIZE];
	uint8_t digest[SHA_MAX_DIGEST_LEN];

	// openssl dgst -sha512 -mac hmac -macopt hexkey:AC1D0522AC1D0522FAD4CC29FAD4CC29DA3CEEA4820DAA50ACFEBB341D0522AC
	uint8_t openssl_test_hmac_sha512_vector[] = {
		0x39, 0xcc, 0x0f, 0x33, 0xc5, 0x9e, 0xfa, 0xe8, 0x9f, 0xf2, 0x29, 0x5f, 0x92, 0x67, 0xe0, 0xb3,
		0x7e, 0x9b, 0x0d, 0x92, 0xfd, 0x50, 0x65, 0x06, 0x1f, 0x5b, 0xed, 0x21, 0x42, 0x5c, 0x7a, 0x1f,
		0xd7, 0xa5, 0xd8, 0xb3, 0x49, 0x8b, 0x36, 0xbf, 0x35, 0xf3, 0x6d, 0x41, 0xe1, 0x28, 0xf3, 0xeb,
		0x61, 0xc2, 0x14, 0x77, 0x47, 0x2a, 0x8e, 0x39, 0x14, 0xbc, 0xf6, 0xc3, 0xa9, 0xdf, 0xeb, 0x0a
	};

	TEST_IGNORE_MESSAGE("Not supported yet. Waiting for support in nRF Crypto.");

	// Test precondition
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memset(digest, 0x00, sizeof(digest));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// SHA2_512
	params.algo = SID_PAL_HASH_SHA512;
	params.key = hmac_test_key;
	params.key_size = sizeof(hmac_test_key);
	params.data = data_copy;
	params.data_size = HMAC_TEST_DATA_BLOCK_SIZE;
	params.digest = digest;
	params.digest_size = SHA512_LEN;

	// With proper key
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(openssl_test_hmac_sha512_vector, digest, SHA512_LEN);

	// The same data with fake key (digest must be different)
	params.key = hmac_test_fake_key;
	params.key_size = sizeof(hmac_test_fake_key);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_hmac(&params));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(openssl_test_hmac_sha512_vector, digest, SHA512_LEN));
}
/**********************************************
* END HMAC
* ********************************************/

void test_sid_pal_crypto_aes_crypt(void)
{
	sid_pal_aes_params_t params;

	uint8_t data_copy[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t iv[AES_MAX_BLOCK_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t cmac[AES_MAX_BLOCK_SIZE];

	// echo -n "Openssl data in." | openssl enc -aes-128-ctr -K AC1D0522AC1D0522FAD4CC29FAD4CC29  -iv B1B1B1B1B1B1B1B1B1B1B1B1B1B1B1B1 -e
	uint8_t external_data[16] = { "Openssl data in." };
	uint8_t external_data_encrypted[AES_MAX_BLOCK_SIZE] = { 0xf0, 0xeb, 0x3f, 0x56, 0x38, 0xfd, 0xd3, 0x14,
								0x3d, 0x14, 0x8b, 0xa9, 0x73, 0xc3, 0x8e, 0x45 };

	size_t test_vector_data_in_len[] = { 8, 16, 24, 55, 64, AES_TEST_DATA_BLOCK_SIZE };

	// openssl dgst -mac cmac -macopt cipher:aes-128-cbc -macopt hexkey:AC1D0522AC1D0522FAD4CC29FAD4CC29
	uint8_t openssl_test_cmac_vector[][AES_MAX_BLOCK_SIZE] = {
		{ 0xd6, 0xe0, 0x33, 0x97, 0x57, 0x6b, 0xe6, 0x1d, 0xd1, 0x78, 0xf4, 0x97, 0x32, 0x81, 0x39, 0x00 },
		{ 0x99, 0x22, 0x80, 0x72, 0xd9, 0x41, 0x1f, 0x14, 0xe1, 0x79, 0x37, 0xab, 0x87, 0xd6, 0x96, 0x40 },
		{ 0xfc, 0xa4, 0x23, 0xf1, 0x00, 0x41, 0x17, 0x26, 0x15, 0xe4, 0xfc, 0xe7, 0xfa, 0x1e, 0x51, 0x1e },
		{ 0x4f, 0x50, 0xf4, 0x69, 0x80, 0x5d, 0xb4, 0x50, 0x05, 0xf4, 0xee, 0xae, 0x76, 0x73, 0xbf, 0x8f },
		{ 0xd0, 0xe4, 0x8f, 0x0f, 0xa3, 0x27, 0x98, 0x55, 0x00, 0xc3, 0x89, 0x34, 0x2a, 0x8c, 0xe8, 0x78 },
		{ 0x6d, 0xb4, 0x78, 0x04, 0xe7, 0xb7, 0xce, 0xf5, 0xd7, 0x34, 0xdf, 0x47, 0x78, 0x52, 0x53, 0x2c },
	};


	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// NULL pointer
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data_copy;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = NULL;
	params.out = encrypted_data;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = NULL;
	params.out = NULL;
	params.key = aes_128_test_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = NULL;
	params.out = encrypted_data;
	params.key = aes_128_test_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data_copy;
	params.out = NULL;
	params.key = aes_128_test_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data_copy;
	params.out = encrypted_data;
	params.key = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data_copy;
	params.out = encrypted_data;
	params.key = aes_128_test_key;
	params.algo = SID_PAL_AES_CTR_128;
	params.iv = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	params.in = data_copy;
	params.out = encrypted_data;
	params.key = aes_128_test_key;

	params.algo = SID_PAL_AES_CTR_128;
	params.iv = iv;
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aes_crypt(&params));

	// Invalid arguments
	params.algo = SID_PAL_AES_CMAC_128;
	params.in_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.algo = SID_PAL_AES_CMAC_128;
	params.in_size = sizeof(data_copy);
	params.key_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.key_size = AES_MAX_BLOCK_SIZE - 5;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.algo = SID_PAL_AES_CTR_128;
	params.key_size = sizeof(aes_128_test_key) * 8; // bits
	params.iv_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.algo = SID_PAL_AES_CTR_128;
	params.key_size = sizeof(aes_128_test_key) * 8; // bits
	params.iv_size = AES_MAX_BLOCK_SIZE - 5;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.key_size = sizeof(aes_128_test_key) * 8; // bits
	params.in_size = sizeof(data_copy);

	// Incorrect algorithm
	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_aes_crypt(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_aes_crypt(&params));

	// Common IV
	memset(iv, 0xC1, sizeof(iv));
	// Reset buffers
	memset(encrypted_data, 0x00, sizeof(encrypted_data));
	memset(decrypted_data, 0x00, sizeof(decrypted_data));

	// Time to encrypt/decrypt data
	params.algo = SID_PAL_AES_CTR_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));

	// Key and iv are the same.
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.in = encrypted_data;
	params.in_size = sizeof(encrypted_data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));

	// We can check if the data are match with original data content
	TEST_ASSERT_EQUAL_UINT8_ARRAY(decrypted_data, test_string, sizeof(decrypted_data));

	// Bad iv
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	// Key is the same.
	iv[2] = 0xFF;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));
	// data shall be corrupted
	TEST_ASSERT_NOT_EQUAL(0, memcmp(decrypted_data, test_string, sizeof(decrypted_data)));

	// Untrusted key
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	// restore iv
	memset(iv, 0xC1, sizeof(iv));

	params.key = aes_128_test_fake_key;
	params.key_size = sizeof(aes_128_test_fake_key) * 8;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));
	// data shall be corrupted
	TEST_ASSERT_NOT_EQUAL(0, memcmp(decrypted_data, test_string, sizeof(decrypted_data)));

	// We can provide to small output buffer
	params.out_size = sizeof(decrypted_data) / 2;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_aes_crypt(&params));

	// Common IV
	memset(iv, 0xC1, sizeof(iv));
	// Reset buffers
	memset(encrypted_data, 0x00, sizeof(encrypted_data));
	memset(decrypted_data, 0x00, sizeof(decrypted_data));

	// Time to encrypt/decrypt something small
	params.algo = SID_PAL_AES_CTR_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);
	params.in = data_copy;
	params.in_size = 16;
	params.out = encrypted_data;
	params.out_size = 16;

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));

	// Key and iv are the same.
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.in = encrypted_data;
	params.in_size = 16;
	params.out = decrypted_data;
	params.out_size = 16;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));

	// We can check if the data are match with part of the original data content
	TEST_ASSERT_EQUAL_UINT8_ARRAY(decrypted_data, test_string, 16);

	// Data is not aligned to 16 byte.
	params.in_size = 12;

	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.in = data_copy;
	params.out = encrypted_data;
	params.out_size = 64;

	// The SID_ERROR_INVALID_ARGS error code is generated by PSA library.
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	params.in_size = 34;
	// The SID_ERROR_INVALID_ARGS error code is generated by PSA library.
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aes_crypt(&params));

	// Common IV
	memset(iv, 0xB1, sizeof(iv));
	// Reset buffers
	memset(encrypted_data, 0x00, sizeof(encrypted_data));
	memset(decrypted_data, 0x00, sizeof(decrypted_data));

	// Time to decrypt external data
	params.algo = SID_PAL_AES_CTR_128;
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.in = external_data_encrypted;
	params.in_size = sizeof(external_data_encrypted);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = sizeof(iv);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));

	// We can check if the data are match with expected data.
	TEST_ASSERT_EQUAL_UINT8_ARRAY(decrypted_data, external_data, sizeof(external_data));

	// TODO: CMAC
	// CMAC calculation
	params.algo = SID_PAL_AES_CMAC_128;
	params.mode = SID_PAL_CRYPTO_MAC_CALCULATE;
	params.in = data_copy;
	params.out = cmac;
	params.out_size = sizeof(cmac);
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = NULL;

	// Verify test conditions
	TEST_ASSERT_EQUAL(ARRAY_SIZE(openssl_test_cmac_vector), ARRAY_SIZE(test_vector_data_in_len));
	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_data_in_len); test_it++) {
		memset(cmac, 0x00, sizeof(cmac));

		params.in_size = test_vector_data_in_len[test_it];
		TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aes_crypt(&params));
	}
}

void test_sid_pal_crypto_aead_crypt(void)
{
	sid_pal_aead_params_t params;

	uint8_t data_copy[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Additional data..." };
	uint8_t fake_additional_data[AES_TEST_DATA_BLOCK_SIZE] = { "Fake data..." };
	uint8_t iv[AES_IV_SIZE];
	uint8_t encrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t decrypted_data[AES_TEST_DATA_BLOCK_SIZE];
	uint8_t mac[AES_MAX_BLOCK_SIZE];
	uint8_t fake_mac[AES_MAX_BLOCK_SIZE];

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memcpy(data_copy, test_string, sizeof(data_copy));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// NULL pointer
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = NULL;
	params.in = data_copy;
	params.out = encrypted_data;
	params.aad = additional_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.in = NULL;
	params.out = encrypted_data;
	params.aad = additional_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.in = data_copy;
	params.out = NULL;
	params.aad = additional_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.in = data_copy;
	params.out = encrypted_data;
	params.aad = NULL;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.in = data_copy;
	params.out = encrypted_data;
	params.aad = additional_data;
	params.mac = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.in = NULL;
	params.out = encrypted_data;
	params.aad = NULL;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = aes_128_test_key;
	params.in = data_copy;
	params.out = NULL;
	params.aad = additional_data;
	params.mac = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = NULL;
	params.in = NULL;
	params.out = NULL;
	params.aad = additional_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	params.key = NULL;
	params.in = NULL;
	params.out = encrypted_data;
	params.aad = NULL;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_aead_crypt(&params));

	// Invalid arguments
	params.key = aes_128_test_key;
	params.in = data_copy;
	params.out = encrypted_data;
	params.aad = additional_data;
	params.mac = mac;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.aad_size = sizeof(additional_data);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.aad_size = 0;
	params.in_size = sizeof(data_copy);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.algo = SID_PAL_AEAD_GCM_128;
	params.aad_size = sizeof(data_copy);
	params.in_size = sizeof(data_copy);
	params.key_size = (AES_MAX_BLOCK_SIZE - 1) * 8;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.key_size = (AES_MAX_BLOCK_SIZE + 1) * 8;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.mode = 0;
	params.key_size = AES_MAX_BLOCK_SIZE * 8;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.mode = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key_size = AES_MAX_BLOCK_SIZE * 8;
	params.iv = iv;
	params.iv_size = AES_GCM_IV_SIZE + 2;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.iv_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.iv_size = AES_GCM_IV_SIZE - 2;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.algo = SID_PAL_AEAD_CCM_128;
	params.aad_size = sizeof(data_copy);
	params.in_size = sizeof(data_copy);
	params.key_size = (AES_MAX_BLOCK_SIZE - 1) * 8;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.key_size = (AES_MAX_BLOCK_SIZE + 1) * 8;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.mode = 0;
	params.key_size = AES_MAX_BLOCK_SIZE * 8;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.mode = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key_size = AES_MAX_BLOCK_SIZE * 8;
	params.iv = iv;
	params.iv_size = AES_CCM_IV_SIZE + 2;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.iv_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	params.iv_size = AES_CCM_IV_SIZE + 2;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_aead_crypt(&params));

	// Incorrect algorithm
	params.algo = 0;
	params.aad_size = sizeof(data_copy);
	params.in_size = sizeof(data_copy);
	params.key_size = sizeof(aes_128_test_key) * 8;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_aead_crypt(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_aead_crypt(&params));

	// Set common IV
	memset(iv, 0xB1, AES_GCM_IV_SIZE);
	// Reset buffers
	memset(encrypted_data, 0x00, sizeof(encrypted_data));
	memset(decrypted_data, 0x00, sizeof(decrypted_data));

	// GCM_128
	params.algo = SID_PAL_AEAD_GCM_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_GCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));

	// Key and IV are the same
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.in = encrypted_data;
	params.in_size = sizeof(encrypted_data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(decrypted_data, data_copy, sizeof(data_copy));

	// Bad key
	params.key = aes_128_test_fake_key;
	params.key_size = sizeof(aes_128_test_fake_key) * 8;
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(decrypted_data, data_copy, sizeof(data_copy)));

	// Incorrect IV
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	memset(iv, 0xCC, AES_GCM_IV_SIZE);
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(decrypted_data, data_copy, sizeof(data_copy)));

	// Broken MAC
	memset(iv, 0xB1, AES_GCM_IV_SIZE);
	memcpy(fake_mac, mac, sizeof(fake_mac));
	memset(&fake_mac[2], 0x4C, 0x02);
	params.mac = fake_mac;
	params.mac_size = sizeof(fake_mac);
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	// Verification fail
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	// But data should be correct
	TEST_ASSERT_EQUAL_UINT8_ARRAY(decrypted_data, data_copy, sizeof(data_copy));

	// Incorrect additional data
	params.mac = mac;
	params.mac_size = sizeof(mac);
	params.aad = fake_additional_data;
	params.aad_size = sizeof(fake_additional_data);
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	// Verification fail
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	// But data should be correct
	TEST_ASSERT_EQUAL_UINT8_ARRAY(decrypted_data, data_copy, sizeof(data_copy));


	// Set common IV
	memset(iv, 0xB1, AES_GCM_IV_SIZE);
	// Reset buffers
	memset(encrypted_data, 0x00, sizeof(encrypted_data));
	memset(decrypted_data, 0x00, sizeof(decrypted_data));

	// CCM_128
	params.algo = SID_PAL_AEAD_CCM_128;
	params.mode = SID_PAL_CRYPTO_ENCRYPT;
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	params.iv = iv;
	params.iv_size = AES_CCM_IV_SIZE;
	params.aad = additional_data;
	params.aad_size = sizeof(additional_data);
	params.in = data_copy;
	params.in_size = sizeof(data_copy);
	params.out = encrypted_data;
	params.out_size = sizeof(encrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));

	// Key and IV are the same
	params.mode = SID_PAL_CRYPTO_DECRYPT;
	params.in = encrypted_data;
	params.in_size = sizeof(encrypted_data);
	params.out = decrypted_data;
	params.out_size = sizeof(decrypted_data);
	params.mac = mac;
	params.mac_size = sizeof(mac);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(decrypted_data, data_copy, sizeof(data_copy));

	// Bad key
	params.key = aes_128_test_fake_key;
	params.key_size = sizeof(aes_128_test_fake_key) * 8;
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(decrypted_data, data_copy, sizeof(data_copy)));

	// Incorrect IV
	params.key = aes_128_test_key;
	params.key_size = sizeof(aes_128_test_key) * 8;
	memset(iv, 0xCC, AES_CCM_IV_SIZE);
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(decrypted_data, data_copy, sizeof(data_copy)));

	// Broken MAC
	memset(iv, 0xB1, AES_CCM_IV_SIZE);
	memcpy(fake_mac, mac, sizeof(fake_mac));
	memset(&fake_mac[2], 0x4C, 0x02);
	params.mac = fake_mac;
	params.mac_size = sizeof(fake_mac);
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	// Verification fail
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(decrypted_data, data_copy, sizeof(data_copy)));

	// Incorrect additional data
	params.mac = mac;
	params.mac_size = sizeof(mac);
	params.aad = fake_additional_data;
	params.aad_size = sizeof(fake_additional_data);
	memset(decrypted_data, 0x00, sizeof(decrypted_data));
	// Verification fail
	TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_aead_crypt(&params));
	TEST_ASSERT_NOT_EQUAL(0, memcmp(decrypted_data, data_copy, sizeof(data_copy)));

	// TODO: data encrypted in openssl
}

void test_sid_pal_crypto_ecc_dsa(void)
{
	sid_pal_dsa_params_t params;

	uint8_t data_copy[AES_TEST_DATA_BLOCK_SIZE];
	size_t test_vector_data_in_len[] = { 8, 16, 24, 55, 64, AES_TEST_DATA_BLOCK_SIZE };

	uint8_t signature[ECDSA_SIGNATURE_SIZE];
	uint8_t fake_signature[ECDSA_SIGNATURE_SIZE];

	// Keys are generated in the openssl.
	uint8_t private_SECP256R1[EC_SECP256R1_PRIV_KEY_LEN] = { 0xd0, 0xb2, 0x29, 0xdd, 0x13, 0x97, 0x62, 0xee,
								 0xca, 0x9a, 0xb9, 0x28, 0x57, 0x82, 0xbf, 0x76,
								 0xd9, 0x5e, 0xf0, 0x7f, 0x5c, 0x84, 0x5f, 0x51,
								 0x3f, 0x46, 0x19, 0x59, 0x91, 0x00, 0x10, 0xed };
	uint8_t public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN] = { 0xe4, 0xe9, 0x19, 0x68, 0x99, 0xdd, 0x4e, 0x1e,
							       0xf6, 0xfd, 0xe6, 0x81, 0xa9, 0x0d, 0x1e, 0x61,
							       0x65, 0xf6, 0x42, 0x2b, 0xe1, 0x9e, 0xf5, 0x0c,
							       0x8e, 0x4d, 0x64, 0x64, 0x27, 0xfc, 0xda, 0x5a,
							       0xf1, 0xb2, 0x6d, 0x27, 0xbf, 0xe0, 0x79, 0xdd,
							       0x50, 0x71, 0x89, 0xeb, 0x4a, 0xfb, 0x55, 0x4d,
							       0x79, 0x78, 0x93, 0xab, 0xf5, 0x94, 0xec, 0x2c,
							       0x85, 0x33, 0x99, 0x87, 0x5c, 0x23, 0x1c, 0x37 };
	uint8_t fake_public_SECP256R1[EC_SECP256R1_PUB_KEY_LEN];

	uint8_t private_Ed25519[EC_ED25519_PRIV_KEY_LEN] = { 0x81, 0xa5, 0x9c, 0x6b, 0xd7, 0xcc, 0xe8, 0x07,
							     0xa1, 0xb7, 0xa7, 0x25, 0xfc, 0xed, 0xb0, 0x65,
							     0xea, 0x3c, 0xac, 0x36, 0x90, 0x5e, 0xf4, 0x1b,
							     0x05, 0x34, 0x1e, 0x5c, 0x0b, 0x72, 0x2d, 0x40 };
	uint8_t public_Ed25519[EC_ED25519_PRIV_KEY_LEN] = { 0x67, 0xcb, 0x27, 0xa7, 0xe4, 0x45, 0x28, 0x9c,
							    0x8f, 0x0a, 0xc2, 0x42, 0x6d, 0x0c, 0x05, 0x32,
							    0xce, 0xda, 0x4b, 0xc2, 0x77, 0xfe, 0x2a, 0x01,
							    0x5a, 0xc0, 0xb8, 0x91, 0xbe, 0xd3, 0x50, 0x29 };
	uint8_t fake_public_Ed25519[EC_ED25519_PRIV_KEY_LEN];

	// Test vector with signatures generated in openssl for variable input data length.
	uint8_t openssl_test_sig_vector_SECP256R1[][ECDSA_SIGNATURE_SIZE] = {
		{ 0x18, 0xdd, 0xd9, 0x07, 0x2d, 0xfd, 0xc9, 0x2f, 0x54, 0x12, 0xd6, 0xbb, 0xdd, 0x8c, 0xbe, 0xe3,
		  0x44, 0xce, 0xe2, 0xa7, 0x83, 0xda, 0x90, 0x22, 0xc0, 0x96, 0x91, 0xf1, 0x22, 0x45, 0x3e, 0x26,
		  0xbf, 0x7d, 0x72, 0x6c, 0x60, 0xc9, 0x42, 0x90, 0x10, 0x7b, 0x02, 0xe9, 0x6a, 0xfa, 0x90, 0xea,
		  0xfa, 0x82, 0x22, 0xf0, 0x5e, 0x6d, 0x17, 0x6d, 0x6f, 0xf6, 0x34, 0x69, 0xb4, 0xdb, 0x23, 0x5b },

		{ 0x15, 0x8d, 0xbd, 0xaf, 0x0b, 0xc1, 0x17, 0xac, 0xa3, 0x36, 0xe8, 0xc6, 0x98, 0x68, 0x01, 0xdc,
		  0xaf, 0xd8, 0xf9, 0xbb, 0xaf, 0x0c, 0xca, 0xa3, 0xd4, 0x25, 0x63, 0x54, 0x42, 0x70, 0x34, 0xfb,
		  0x56, 0x55, 0xba, 0x57, 0xad, 0x64, 0xf8, 0x53, 0x21, 0xe0, 0x9c, 0x9c, 0x8c, 0x5d, 0x6d, 0x92,
		  0xf1, 0x55, 0xdf, 0xad, 0xbf, 0x6d, 0x4f, 0xa3, 0x2a, 0xe0, 0x77, 0x8a, 0x37, 0xaa, 0x5c, 0xed },

		{ 0xcc, 0x01, 0x12, 0x60, 0x88, 0x02, 0x9a, 0x49, 0x34, 0x72, 0x23, 0xbf, 0x74, 0x62, 0x2d, 0x5b,
		  0x01, 0x7c, 0x52, 0x85, 0x52, 0x07, 0x8d, 0xfa, 0xb2, 0xa9, 0x1c, 0xd3, 0xae, 0xdc, 0x4c, 0x47,
		  0x8a, 0x6d, 0xa1, 0xd2, 0x5b, 0x20, 0x71, 0x24, 0x5a, 0x94, 0x27, 0xf2, 0x52, 0x7a, 0xf8, 0xbf,
		  0x94, 0xe1, 0x9a, 0x5c, 0xf8, 0x6c, 0x3e, 0x46, 0x37, 0x80, 0x0b, 0x7e, 0x18, 0x05, 0x06, 0x9f },

		{ 0xc3, 0x37, 0x1b, 0x4c, 0x2c, 0x73, 0xe8, 0x3c, 0x0a, 0xab, 0x78, 0x30, 0x80, 0xeb, 0x75, 0xea,
		  0x3b, 0x2c, 0x74, 0x15, 0xba, 0x9a, 0x6d, 0x1a, 0x4f, 0xb4, 0xde, 0x1f, 0x91, 0x3b, 0x0f, 0x8c,
		  0x68, 0xb7, 0x65, 0x6b, 0x61, 0x16, 0xa0, 0x6c, 0xe2, 0x91, 0x63, 0xb9, 0xef, 0xc6, 0x5f, 0x9b,
		  0x42, 0xc8, 0x0a, 0xbe, 0x2d, 0xb9, 0x8d, 0x8e, 0x12, 0x17, 0x68, 0x5f, 0xcc, 0x19, 0x42, 0x5c },

		{ 0xc5, 0xf0, 0x68, 0x7c, 0xf4, 0x94, 0xff, 0x2b, 0x0e, 0xe1, 0x76, 0x51, 0xd2, 0x44, 0x88, 0x3c,
		  0x3d, 0xdb, 0xdd, 0x42, 0xd6, 0x43, 0xf0, 0x4b, 0x10, 0x9e, 0x5e, 0xa8, 0x4f, 0x5c, 0xaf, 0xd7,
		  0xc2, 0x36, 0x52, 0x4e, 0x42, 0xaf, 0x7e, 0x8b, 0xf4, 0x45, 0xef, 0x13, 0x0c, 0xcc, 0xcd, 0x3d,
		  0x51, 0x50, 0xa5, 0xc7, 0x82, 0x1a, 0x88, 0x9d, 0xb8, 0x1c, 0xba, 0x6e, 0x04, 0xd0, 0x55, 0x17 },

		{ 0xa4, 0x4b, 0xa1, 0xe2, 0x0f, 0xc7, 0xfe, 0xbe, 0x51, 0x16, 0x88, 0x88, 0xa1, 0xae, 0x5a, 0x89,
		  0x3c, 0x33, 0x84, 0xec, 0xed, 0x65, 0x4b, 0xd2, 0xc0, 0x93, 0xa1, 0xc9, 0xdc, 0xa4, 0xdb, 0xf8,
		  0x42, 0x12, 0xa4, 0x43, 0x92, 0xdb, 0x4e, 0x1d, 0xc9, 0xb1, 0xeb, 0x5b, 0xe6, 0xe2, 0x6c, 0x43,
		  0x51, 0x6c, 0x23, 0x4e, 0xd0, 0x43, 0x31, 0x35, 0x97, 0x48, 0x5a, 0x83, 0x71, 0xa0, 0x1e, 0xdf }
	};

	uint8_t openssl_test_sig_vector_Ed25519[][ECDSA_SIGNATURE_SIZE] = {
		{ 0xc1, 0xc9, 0x91, 0x2d, 0xcf, 0x6f, 0x33, 0xc5, 0x54, 0x59, 0xc1, 0xf5, 0xaa, 0xbf, 0xe0, 0x6c,
		  0xa4, 0x1c, 0x26, 0xd4, 0xc9, 0x88, 0x53, 0xb8, 0x68, 0x21, 0x06, 0x7e, 0x84, 0xee, 0xa2, 0xf7,
		  0x70, 0x31, 0x3d, 0x1d, 0xb8, 0x7f, 0xcd, 0x5e, 0x3e, 0xb6, 0x9c, 0xf8, 0x86, 0xc0, 0x66, 0x14,
		  0xc9, 0xf4, 0xe6, 0xc0, 0xf5, 0x36, 0xfa, 0xe5, 0x13, 0xf6, 0x2e, 0x5d, 0x52, 0xd1, 0x65, 0x06 },

		{ 0xd2, 0xa1, 0x2d, 0x74, 0x9e, 0xc9, 0x4a, 0x65, 0x39, 0x20, 0xfb, 0xb8, 0x06, 0x6d, 0x0a, 0x38,
		  0x4d, 0x2d, 0x5c, 0xee, 0xba, 0x1c, 0x5e, 0x9d, 0x0c, 0x76, 0x98, 0x0d, 0xfc, 0xa9, 0xcc, 0xfb,
		  0x05, 0x15, 0xe7, 0xf4, 0xab, 0x1c, 0xd2, 0xb3, 0xd6, 0x16, 0x4f, 0x12, 0xe5, 0x8a, 0x3a, 0x56,
		  0x31, 0x84, 0x1b, 0x9c, 0x13, 0x14, 0x91, 0x61, 0x09, 0x96, 0x1a, 0x26, 0xf9, 0xc0, 0x44, 0x0f },

		{ 0x88, 0x88, 0x9d, 0x6d, 0x53, 0x60, 0x4f, 0x75, 0xf5, 0x49, 0x96, 0xe0, 0xb4, 0x4c, 0x6a, 0x5f,
		  0x70, 0x3b, 0xd5, 0x4a, 0x93, 0x8b, 0xe9, 0xaa, 0x3b, 0xd9, 0xef, 0x62, 0x5a, 0x72, 0x01, 0x9a,
		  0x78, 0x07, 0x82, 0x2b, 0x5d, 0x47, 0xa2, 0x83, 0x1f, 0x5c, 0xfa, 0x95, 0xbd, 0xc3, 0x5c, 0x04,
		  0xe2, 0x7c, 0xc6, 0xe1, 0x4a, 0x8e, 0x4f, 0x47, 0x9e, 0x23, 0x50, 0xc5, 0x13, 0x47, 0x6b, 0x0e },

		{ 0xbb, 0x7e, 0x83, 0x33, 0x57, 0x55, 0xad, 0x6b, 0xd3, 0xe2, 0x67, 0x30, 0xad, 0x27, 0x63, 0x6a,
		  0xf1, 0x63, 0xda, 0x6d, 0x6e, 0xcc, 0x17, 0xee, 0x5a, 0xa5, 0xd8, 0x08, 0x31, 0x26, 0x52, 0x79,
		  0x2d, 0xc1, 0x21, 0x3a, 0xe1, 0xd6, 0x72, 0xe9, 0x4f, 0x04, 0x44, 0x44, 0x00, 0xc9, 0xe4, 0x96,
		  0x9a, 0x29, 0xa8, 0xc9, 0x3a, 0x5f, 0x87, 0x48, 0xbc, 0xb6, 0xe2, 0xbd, 0xb9, 0xc3, 0x8f, 0x0d },

		{ 0x32, 0x55, 0xe0, 0x34, 0x96, 0x23, 0x5f, 0x0f, 0x08, 0xcb, 0x2d, 0x84, 0x71, 0xd8, 0x8a, 0x35,
		  0xf7, 0x32, 0x26, 0xa4, 0x27, 0x63, 0x63, 0x5e, 0xe0, 0xf7, 0xcd, 0xf8, 0xde, 0x13, 0x10, 0xb9,
		  0x7f, 0x13, 0x83, 0xf9, 0x11, 0xed, 0x91, 0xff, 0x33, 0xda, 0x79, 0xac, 0xb3, 0x68, 0xf9, 0x85,
		  0x09, 0xca, 0xef, 0xd9, 0xf3, 0x35, 0xe2, 0x8e, 0x49, 0xd3, 0xa0, 0xd9, 0xc0, 0xf6, 0xf6, 0x0b },

		{ 0xf0, 0x69, 0x48, 0xf8, 0xce, 0x40, 0x13, 0x64, 0xb3, 0xf0, 0x3a, 0x07, 0xb3, 0x3a, 0x92, 0x1b,
		  0x1e, 0x21, 0x55, 0xd3, 0xef, 0xe1, 0x61, 0x14, 0xda, 0x46, 0xbf, 0x1a, 0x33, 0x55, 0x09, 0x79,
		  0x74, 0x56, 0xee, 0x6d, 0x17, 0x0c, 0x7a, 0x1c, 0x23, 0x10, 0x7b, 0x8b, 0x9b, 0x3a, 0x4e, 0x4b,
		  0xe3, 0x30, 0x3f, 0xbf, 0x94, 0x21, 0x37, 0xab, 0x02, 0x05, 0xcb, 0xf2, 0x72, 0x70, 0x12, 0x07 },
	};

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(&signature, 0x00, sizeof(signature));
	memcpy(data_copy, test_string, sizeof(data_copy));
	memcpy(fake_public_SECP256R1, public_SECP256R1, EC_SECP256R1_PUB_KEY_LEN - 5);
	memcpy(fake_public_Ed25519, public_Ed25519, EC_ED25519_PUB_KEY_LEN - 5);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_rand(fake_signature, sizeof(fake_signature)));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// NULL pointer
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = public_SECP256R1;
	params.in = data_copy;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = public_SECP256R1;
	params.in = data_copy;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = NULL;
	params.in = data_copy;
	params.signature = signature;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = NULL;
	params.in = NULL;
	params.signature = signature;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	params.key = public_SECP256R1;
	params.in = NULL;
	params.signature = signature;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_dsa(&params));

	// Incorrect arguments
	params.algo = SID_PAL_ECDSA_SECP256R1;
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_SECP256R1;
	params.in = data_copy;
	params.in_size = 0;
	params.signature = signature;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_dsa(&params));

	params.in_size = sizeof(data_copy);

	params.mode = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_dsa(&params));

	params.mode = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_dsa(&params));

	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key_size = 0;
	params.sig_size = sizeof(signature);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_dsa(&params));

	params.key_size = 1024;
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, sid_pal_crypto_ecc_dsa(&params));

	params.key_size = sizeof(public_SECP256R1);
	params.sig_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, sid_pal_crypto_ecc_dsa(&params));

	params.mode = SID_PAL_CRYPTO_SIGN;
	params.key_size = 0;
	params.sig_size = sizeof(signature);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_dsa(&params));

	params.key_size = 1024;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_dsa(&params));

	// Too small buffer
	params.key_size = sizeof(private_SECP256R1);
	params.sig_size = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_ecc_dsa(&params));

	// Bad algo
	params.algo = SID_PAL_ECDH_CURVE25519;
	params.key_size = sizeof(private_SECP256R1);
	params.sig_size = sizeof(signature);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_dsa(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_dsa(&params));

	// SID_PAL_ECDSA_SECP256R1
	params.algo = SID_PAL_ECDSA_SECP256R1;

	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_data_in_len); test_it++) {
		memset(signature, 0x00, sizeof(signature));

		// Sign message
		params.mode = SID_PAL_CRYPTO_SIGN;
		params.key = private_SECP256R1;
		params.key_size = sizeof(private_SECP256R1);
		params.in = data_copy;
		params.in_size = test_vector_data_in_len[test_it];
		params.signature = signature;
		params.sig_size = sizeof(signature);
		// SID_ERROR_NONE means signature is match
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

		// Verify message
		params.mode = SID_PAL_CRYPTO_VERIFY;
		params.key = public_SECP256R1;
		params.key_size = sizeof(public_SECP256R1);
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

		// Bad public key
		params.key = fake_public_SECP256R1;
		params.key_size = sizeof(fake_public_SECP256R1);
		TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

		// Bad signature
		params.signature = fake_signature;
		params.sig_size = sizeof(fake_signature);
		TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
	}

	// Verify openssl signatures.
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_SECP256R1;
	params.key_size = sizeof(public_SECP256R1);
	params.in = data_copy;
	params.sig_size = ECDSA_SIGNATURE_SIZE;

	// Verify test conditions
	TEST_ASSERT_EQUAL(ARRAY_SIZE(openssl_test_sig_vector_SECP256R1), ARRAY_SIZE(test_vector_data_in_len));
	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_data_in_len); test_it++) {
		memset(signature, 0x00, sizeof(signature));

		params.in_size = test_vector_data_in_len[test_it];;
		params.signature = openssl_test_sig_vector_SECP256R1[test_it];

		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
	}

	// TODO: NOT SUPPORTED YET --> this test will fail when EDDSA be supported
	// SID_PAL_EDDSA_ED25519
	params.algo = SID_PAL_EDDSA_ED25519;

	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_data_in_len); test_it++) {
		memset(signature, 0x00, sizeof(signature));

		// Sign message
		params.mode = SID_PAL_CRYPTO_SIGN;
		params.key = private_Ed25519;
		params.key_size = sizeof(private_Ed25519);
		params.in = data_copy;
		params.in_size = test_vector_data_in_len[test_it];
		params.signature = signature;
		params.sig_size = sizeof(signature);
		// SID_ERROR_NONE means signature is match
		TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_dsa(&params));

		// Verify message
		params.mode = SID_PAL_CRYPTO_VERIFY;
		params.key = public_Ed25519;
		params.key_size = sizeof(public_Ed25519);
		TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

		// Bad public key
		params.key = fake_public_SECP256R1;
		params.key_size = sizeof(fake_public_SECP256R1);
		TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));

		// Bad signature
		params.signature = fake_signature;
		params.sig_size = sizeof(fake_signature);
		TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
	}

	// Verify openssl signatures.
	params.mode = SID_PAL_CRYPTO_VERIFY;
	params.key = public_Ed25519;
	params.key_size = sizeof(public_Ed25519);
	params.in = data_copy;
	params.sig_size = ECDSA_SIGNATURE_SIZE;

	// Verify test conditions
	TEST_ASSERT_EQUAL(ARRAY_SIZE(openssl_test_sig_vector_Ed25519), ARRAY_SIZE(test_vector_data_in_len));
	for (int test_it = 0; test_it < ARRAY_SIZE(test_vector_data_in_len); test_it++) {
		memset(signature, 0x00, sizeof(signature));

		params.in_size = test_vector_data_in_len[test_it];;
		params.signature = openssl_test_sig_vector_Ed25519[test_it];

		TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_dsa(&params));
	}
}

void test_sid_pal_crypto_ecc_key_gen(void)
{
	sid_pal_ecc_key_gen_params_t params;

	size_t prk_size_test_vector[] = { EC_CURVE25519_PRIV_KEY_LEN, EC_SECP256R1_PRIV_KEY_LEN,
					  EC_ED25519_PRIV_KEY_LEN, EC_SECP256R1_PRIV_KEY_LEN };
	size_t puk_size_test_vector[] = { EC_CURVE25519_PUB_KEY_LEN, EC_SECP256R1_PUB_KEY_LEN,
					  EC_ED25519_PUB_KEY_LEN, EC_SECP256R1_PUB_KEY_LEN };

	uint8_t private_key[ECC_PRIVATE_KEY_MAX_LEN];
	uint8_t public_key[ECC_PUBLIC_KEY_MAX_LEN];

	// Prepare test
	memset(&params, 0x00, sizeof(params));
	memset(private_key, 0x00, sizeof(private_key));
	memset(public_key, 0x00, sizeof(public_key));

	// Initialize crypto module
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_init());

	// NULL pointer
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_key_gen(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_key_gen(&params));

	params.prk = private_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_key_gen(&params));

	params.prk = NULL;
	params.puk = public_key;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_crypto_ecc_key_gen(&params));

	params.prk = private_key;
	params.puk = public_key;
	params.puk_size = EC_SECP256R1_PUB_KEY_LEN;
	// Incorrect algorithm
	params.algo = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_key_gen(&params));

	params.algo = 9;
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_crypto_ecc_key_gen(&params));

	// Incorrect arguments
	params.algo = SID_PAL_ECDSA_SECP256R1;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_ecc_key_gen(&params));

	params.puk_size = EC_SECP256R1_PUB_KEY_LEN;
	TEST_ASSERT_EQUAL(SID_ERROR_OUT_OF_RESOURCES, sid_pal_crypto_ecc_key_gen(&params));

	params.puk_size = 0;
	params.prk_size = EC_SECP256R1_PRIV_KEY_LEN;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_key_gen(&params));

	// Generate key pair few times, next key should be different from last one.
	for (sid_pal_ecc_algo_t algo = SID_PAL_ECDH_CURVE25519; algo <= SID_PAL_ECDSA_SECP256R1; algo++) {
		uint8_t tmp_private_key[ECC_PRIVATE_KEY_MAX_LEN];
		uint8_t tmp_public_key[ECC_PUBLIC_KEY_MAX_LEN];

		params.algo = algo;
		params.prk = private_key;
		params.prk_size = prk_size_test_vector[algo - 1];
		params.puk = public_key;
		params.puk_size = puk_size_test_vector[algo - 1];

		// Reset buffers before next test loop execution
		memset(tmp_private_key, 0x00, sizeof(tmp_private_key));
		memset(tmp_public_key, 0x00, sizeof(tmp_public_key));

		for (int i = 0; i < 8; i++) {
			memset(private_key, 0x00, sizeof(private_key));
			memset(public_key, 0x00, sizeof(public_key));

			TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_key_gen(&params));
			TEST_ASSERT_NOT_EQUAL(0, memcmp(tmp_private_key, private_key, ECC_PRIVATE_KEY_MAX_LEN));
			TEST_ASSERT_NOT_EQUAL(0, memcmp(tmp_public_key, public_key, ECC_PUBLIC_KEY_MAX_LEN));

			memcpy(tmp_private_key, private_key, ECC_PRIVATE_KEY_MAX_LEN);
			memcpy(tmp_public_key, public_key, ECC_PUBLIC_KEY_MAX_LEN);
		}
	}
}

void test_sid_pal_crypto_ecc_ecdh_secp256r1(void)
{
	/* Keys generated in the openssl */
	uint8_t private_secp256r1_a[EC_SECP256R1_PRIV_KEY_LEN] = {
		0xd0, 0xb2, 0x29, 0xdd, 0x13, 0x97, 0x62, 0xee,
		0xca, 0x9a, 0xb9, 0x28, 0x57, 0x82, 0xbf, 0x76,
		0xd9, 0x5e, 0xf0, 0x7f, 0x5c, 0x84, 0x5f, 0x51,
		0x3f, 0x46, 0x19, 0x59, 0x91, 0x00, 0x10, 0xed
	};

	uint8_t public_secp256r1_b1[EC_SECP256R1_PUB_KEY_LEN] = {
		0x89, 0xed, 0x69, 0x79, 0xf6, 0x76, 0xd6, 0xe2,
		0xe9, 0x4f, 0x5e, 0xef, 0x14, 0x7e, 0x45, 0x1c,
		0x6d, 0xec, 0x24, 0xdb, 0x75, 0xd2, 0xc5, 0x8c,
		0x3c, 0xc1, 0xa0, 0x2d, 0x8e, 0xb3, 0xda, 0x17,
		0xf4, 0x65, 0xc8, 0x13, 0xf9, 0xf7, 0xc6, 0x6a,
		0xec, 0xc3, 0xde, 0x67, 0x5c, 0x51, 0xe0, 0x0a,
		0x20, 0x96, 0xe1, 0x8a, 0x72, 0x25, 0xc2, 0xc3,
		0xc0, 0x72, 0x1b, 0x8c, 0xd0, 0xea, 0x1a, 0x28
	};

	uint8_t secret_expected_b1[ECDH_SECRET_SIZE] = {
		0xa9, 0x08, 0x7e, 0x9b, 0xc0, 0x63, 0x57, 0x74,
		0xd0, 0xe8, 0x6d, 0x9c, 0xb9, 0xc4, 0xc9, 0x16,
		0x21, 0x4e, 0x2e, 0x90, 0xaf, 0xda, 0xc5, 0xa5,
		0xeb, 0xb4, 0xd4, 0xf6, 0x8f, 0x96, 0x22, 0xd6
	};

	uint8_t public_secp256r1_b2[EC_SECP256R1_PUB_KEY_LEN] = {
		0x09, 0x24, 0x02, 0x61, 0x43, 0x0d, 0x3f, 0xaa,
		0x34, 0xca, 0xcc, 0x1c, 0xfa, 0x9b, 0x17, 0xf5,
		0x2d, 0xf8, 0x5b, 0x8b, 0x9a, 0x58, 0x8d, 0x98,
		0x42, 0x73, 0x45, 0x78, 0x2f, 0x03, 0xad, 0x9b,
		0xe3, 0x2c, 0xc3, 0x71, 0x46, 0x41, 0x20, 0xf4,
		0x16, 0x0a, 0xa9, 0xe1, 0x0c, 0x3d, 0xa5, 0x83,
		0x49, 0xe2, 0x40, 0x1a, 0x17, 0x4d, 0xa7, 0xbc,
		0xbf, 0x29, 0x3d, 0x3b, 0xe4, 0x86, 0xd4, 0x75
	};

	uint8_t secret_expected_b2[ECDH_SECRET_SIZE] = {
		0xf0, 0x07, 0x6e, 0x24, 0x5b, 0xb4, 0xc9, 0x1e,
		0x15, 0xa8, 0x78, 0x7d, 0xd2, 0x00, 0x99, 0xa8,
		0xaa, 0x6f, 0x62, 0x52, 0x7e, 0x5b, 0x5f, 0x1f,
		0x2a, 0x92, 0x98, 0x64, 0x9e, 0x78, 0x66, 0xa6
	};

	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	sid_pal_ecdh_params_t ecdh_params;

	/* Derive and verify secret with private key A and public key B1 */
	ecdh_params.algo = SID_PAL_ECDH_SECP256R1;
	ecdh_params.prk = private_secp256r1_a;
	ecdh_params.prk_size = EC_SECP256R1_PRIV_KEY_LEN;
	ecdh_params.puk = public_secp256r1_b1;
	ecdh_params.puk_size = EC_SECP256R1_PUB_KEY_LEN;
	ecdh_params.shared_secret = secret_result;
	ecdh_params.shared_secret_sz = ECDH_SECRET_SIZE;

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(secret_expected_b1, ecdh_params.shared_secret, ecdh_params.shared_secret_sz);

	/* Derive and verify secret with private key A and public key B2 */
	memset(secret_result, 0x00, sizeof(secret_result));

	ecdh_params.puk = public_secp256r1_b2;
	ecdh_params.puk_size = EC_SECP256R1_PUB_KEY_LEN;

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(secret_expected_b2, ecdh_params.shared_secret, ecdh_params.shared_secret_sz);

}

void test_sid_pal_crypto_ecc_ecdh_secp256r1_negative(void)
{
	/* Keys generated in the openssl */
	uint8_t private_secp256r1_a[EC_SECP256R1_PRIV_KEY_LEN] = {
		0xd0, 0xb2, 0x29, 0xdd, 0x13, 0x97, 0x62, 0xee,
		0xca, 0x9a, 0xb9, 0x28, 0x57, 0x82, 0xbf, 0x76,
		0xd9, 0x5e, 0xf0, 0x7f, 0x5c, 0x84, 0x5f, 0x51,
		0x3f, 0x46, 0x19, 0x59, 0x91, 0x00, 0x10, 0xed
	};

	uint8_t public_secp256r1_b1[EC_SECP256R1_PUB_KEY_LEN] = {
		0x89, 0xed, 0x69, 0x79, 0xf6, 0x76, 0xd6, 0xe2,
		0xe9, 0x4f, 0x5e, 0xef, 0x14, 0x7e, 0x45, 0x1c,
		0x6d, 0xec, 0x24, 0xdb, 0x75, 0xd2, 0xc5, 0x8c,
		0x3c, 0xc1, 0xa0, 0x2d, 0x8e, 0xb3, 0xda, 0x17,
		0xf4, 0x65, 0xc8, 0x13, 0xf9, 0xf7, 0xc6, 0x6a,
		0xec, 0xc3, 0xde, 0x67, 0x5c, 0x51, 0xe0, 0x0a,
		0x20, 0x96, 0xe1, 0x8a, 0x72, 0x25, 0xc2, 0xc3,
		0xc0, 0x72, 0x1b, 0x8c, 0xd0, 0xea, 0x1a, 0x28
	};

	uint8_t secret_expected_b1[ECDH_SECRET_SIZE] = {
		0xa9, 0x08, 0x7e, 0x9b, 0xc0, 0x63, 0x57, 0x74,
		0xd0, 0xe8, 0x6d, 0x9c, 0xb9, 0xc4, 0xc9, 0x16,
		0x21, 0x4e, 0x2e, 0x90, 0xaf, 0xda, 0xc5, 0xa5,
		0xeb, 0xb4, 0xd4, 0xf6, 0x8f, 0x96, 0x22, 0xd6
	};

	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	sid_pal_ecdh_params_t ecdh_params;

	ecdh_params.algo = SID_PAL_ECDH_SECP256R1;
	ecdh_params.prk = private_secp256r1_a;
	ecdh_params.prk_size = EC_SECP256R1_PRIV_KEY_LEN;
	ecdh_params.puk = public_secp256r1_b1;
	ecdh_params.puk_size = EC_SECP256R1_PUB_KEY_LEN;
	ecdh_params.shared_secret = secret_result;
	ecdh_params.shared_secret_sz = ECDH_SECRET_SIZE;

	int secret_diff = 0;

	/* Malfrom peer public key */
	public_secp256r1_b1[0] = ~public_secp256r1_b1[0];

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_crypto_ecc_ecdh(&ecdh_params));
	secret_diff = memcmp(secret_expected_b1, ecdh_params.shared_secret, ecdh_params.shared_secret_sz);
	TEST_ASSERT_NOT_EQUAL_INT(0, secret_diff);

	public_secp256r1_b1[0] = ~public_secp256r1_b1[0];

	/* Malform private key */
	private_secp256r1_a[0] = ~private_secp256r1_a[0];

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params));
	secret_diff = memcmp(secret_expected_b1, ecdh_params.shared_secret, ecdh_params.shared_secret_sz);
	TEST_ASSERT_NOT_EQUAL_INT(0, secret_diff);
}

void test_sid_pal_crypto_ecc_ecdh_x25519(void)
{
	TEST_IGNORE_MESSAGE("Not supported yet. Waiting for support in nRF Crypto.");

	/* Keys generated in the openssl */
	uint8_t private_x25519_a[EC_CURVE25519_PRIV_KEY_LEN] = {
		0x28, 0x8e, 0x32, 0x0f, 0xc8, 0x74, 0x4e, 0x58,
		0xd8, 0x49, 0x57, 0x42, 0x72, 0xb6, 0xe2, 0x3c,
		0x4a, 0x71, 0x36, 0xd4, 0x87, 0x28, 0x34, 0x17,
		0x5d, 0xc9, 0xf9, 0x5c, 0xfc, 0x45, 0x34, 0x65
	};

	uint8_t public_x25519_b[EC_CURVE25519_PUB_KEY_LEN] = {
		0x33, 0x01, 0x40, 0x0a, 0x7d, 0xb0, 0x3b, 0x29,
		0xee, 0x5b, 0xc3, 0x03, 0xbd, 0x85, 0x07, 0xe4,
		0xbd, 0x3a, 0x11, 0x53, 0xce, 0x39, 0xaf, 0x39,
		0x10, 0x88, 0x64, 0x6a, 0xbb, 0xb5, 0x56, 0x71
	};

	uint8_t secret_expected_b[ECDH_SECRET_SIZE] = {
		0xd8, 0xf2, 0x6c, 0xed, 0x96, 0x29, 0x32, 0x0f,
		0xe4, 0xe8, 0xc6, 0x10, 0x2f, 0x4f, 0xcd, 0xd2,
		0x98, 0x55, 0xe2, 0x47, 0xd5, 0x9f, 0xc1, 0x1c,
		0x4a, 0xee, 0x80, 0x5e, 0xff, 0x07, 0x86, 0x1a
	};

	uint8_t secret_result[ECDH_SECRET_SIZE] = { 0 };

	sid_pal_ecdh_params_t ecdh_params;

	/* Derive and verify secret with private key A and public key B1 */
	ecdh_params.algo = SID_PAL_ECDH_CURVE25519;
	ecdh_params.prk = private_x25519_a;
	ecdh_params.prk_size = EC_CURVE25519_PRIV_KEY_LEN;
	ecdh_params.puk = public_x25519_b;
	ecdh_params.puk_size = EC_CURVE25519_PUB_KEY_LEN;
	ecdh_params.shared_secret = secret_result;
	ecdh_params.shared_secret_sz = ECDH_SECRET_SIZE;

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(secret_expected_b, ecdh_params.shared_secret, ecdh_params.shared_secret_sz);
}

void test_sid_pal_crypto_ecc_ecdh_shared_secret(void)
{
	// Keys are generated in the openssl.
	uint8_t private_secp256r1_a[EC_SECP256R1_PRIV_KEY_LEN] = {
		0xd0, 0xb2, 0x29, 0xdd, 0x13, 0x97, 0x62, 0xee,
		0xca, 0x9a, 0xb9, 0x28, 0x57, 0x82, 0xbf, 0x76,
		0xd9, 0x5e, 0xf0, 0x7f, 0x5c, 0x84, 0x5f, 0x51,
		0x3f, 0x46, 0x19, 0x59, 0x91, 0x00, 0x10, 0xed
	};
	uint8_t public_secp256r1_a[EC_SECP256R1_PUB_KEY_LEN] = {
		0xe4, 0xe9, 0x19, 0x68, 0x99, 0xdd, 0x4e, 0x1e,
		0xf6, 0xfd, 0xe6, 0x81, 0xa9, 0x0d, 0x1e, 0x61,
		0x65, 0xf6, 0x42, 0x2b, 0xe1, 0x9e, 0xf5, 0x0c,
		0x8e, 0x4d, 0x64, 0x64, 0x27, 0xfc, 0xda, 0x5a,
		0xf1, 0xb2, 0x6d, 0x27, 0xbf, 0xe0, 0x79, 0xdd,
		0x50, 0x71, 0x89, 0xeb, 0x4a, 0xfb, 0x55, 0x4d,
		0x79, 0x78, 0x93, 0xab, 0xf5, 0x94, 0xec, 0x2c,
		0x85, 0x33, 0x99, 0x87, 0x5c, 0x23, 0x1c, 0x37
	};

	uint8_t secret_a[ECDH_SECRET_SIZE] = { 0 };

	uint8_t private_secp256r1_b[EC_SECP256R1_PRIV_KEY_LEN] = {
		0x28, 0x0b, 0xea, 0xfa, 0x60, 0x55, 0xf9, 0x26,
		0xb7, 0xfa, 0xd7, 0xce, 0x06, 0x62, 0x76, 0x06,
		0xda, 0x34, 0x75, 0xc5, 0x47, 0xa3, 0x06, 0x8f,
		0xd5, 0x73, 0xa2, 0xd4, 0x11, 0xa2, 0xd5, 0x0b
	};
	uint8_t public_secp256r1_b[EC_SECP256R1_PUB_KEY_LEN] = {
		0x89, 0xed, 0x69, 0x79, 0xf6, 0x76, 0xd6, 0xe2,
		0xe9, 0x4f, 0x5e, 0xef, 0x14, 0x7e, 0x45, 0x1c,
		0x6d, 0xec, 0x24, 0xdb, 0x75, 0xd2, 0xc5, 0x8c,
		0x3c, 0xc1, 0xa0, 0x2d, 0x8e, 0xb3, 0xda, 0x17,
		0xf4, 0x65, 0xc8, 0x13, 0xf9, 0xf7, 0xc6, 0x6a,
		0xec, 0xc3, 0xde, 0x67, 0x5c, 0x51, 0xe0, 0x0a,
		0x20, 0x96, 0xe1, 0x8a, 0x72, 0x25, 0xc2, 0xc3,
		0xc0, 0x72, 0x1b, 0x8c, 0xd0, 0xea, 0x1a, 0x28
	};

	uint8_t secret_b[ECDH_SECRET_SIZE] = { 0 };

	/* Derive shared secret using private key A and public key B */
	sid_pal_ecdh_params_t ecdh_params_a = {
		.algo = SID_PAL_ECDH_SECP256R1,
		.prk = private_secp256r1_a,
		.prk_size = EC_SECP256R1_PRIV_KEY_LEN,
		.puk = public_secp256r1_b,
		.puk_size = EC_SECP256R1_PUB_KEY_LEN,
		.shared_secret = secret_a,
		.shared_secret_sz = ECDH_SECRET_SIZE,
	};

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params_a));

	/* Derive shared secret using private key 2 and public key 1 */
	sid_pal_ecdh_params_t ecdh_params_b = {
		.algo = SID_PAL_ECDH_SECP256R1,
		.prk = private_secp256r1_b,
		.prk_size = EC_SECP256R1_PRIV_KEY_LEN,
		.puk = public_secp256r1_a,
		.puk_size = EC_SECP256R1_PUB_KEY_LEN,
		.shared_secret = secret_b,
		.shared_secret_sz = ECDH_SECRET_SIZE,
	};

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_crypto_ecc_ecdh(&ecdh_params_b));
	/* Compare secrets */
	TEST_ASSERT_EQUAL_UINT8_ARRAY(secret_a, secret_b, ECDH_SECRET_SIZE);
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
