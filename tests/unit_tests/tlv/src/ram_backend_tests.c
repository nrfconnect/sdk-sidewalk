/**
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * 
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "zephyr/ztest_assert.h"
#include <string.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>
#include <errno.h>
#include <tlv/tlv.h>
#include <tlv/tlv_storage_impl.h>

// clang-format off
#define PARAMETRIZED_TEST(siute, name, test_foo, ...) ZTEST(siute, name){ test_foo(__VA_ARGS__); }
// clang-format on

uint8_t TLV_RAM_STORAGE[64] = { 0 };

tlv_ctx RAM_TLV_OBJECT = (tlv_ctx){ .start_offset = 0,
				    .end_offset = sizeof(TLV_RAM_STORAGE),
				    .storage_impl = { .ctx = TLV_RAM_STORAGE,
						      .read = tlv_storage_ram_read,
						      .write = tlv_storage_ram_write } };

tlv_ctx RAM_TLV_OBJECT_WITH_MAGIC = (tlv_ctx){ .start_offset = 0,
					       .end_offset = sizeof(TLV_RAM_STORAGE),
					       .tlv_storage_start_marker_size = 8,
					       .storage_impl = { .ctx = TLV_RAM_STORAGE,
								 .read = tlv_storage_ram_read,
								 .write = tlv_storage_ram_write } };

tlv_ctx RAM_TLV_OBJECT_NO_WRITE = (tlv_ctx){
	.start_offset = 0,
	.end_offset = sizeof(TLV_RAM_STORAGE),
	.storage_impl = { .ctx = TLV_RAM_STORAGE, .read = tlv_storage_ram_read, .write = NULL }
};

tlv_ctx RAM_TLV_OBJECT_NO_READ = (tlv_ctx){
	.start_offset = 0,
	.end_offset = sizeof(TLV_RAM_STORAGE),
	.storage_impl = { .ctx = TLV_RAM_STORAGE, .read = NULL, .write = tlv_storage_ram_write }
};

tlv_ctx RAM_TLV_OBJECT_OFFSET = (tlv_ctx){ .start_offset = 3,
					   .end_offset = sizeof(TLV_RAM_STORAGE),
					   .storage_impl = { .ctx = TLV_RAM_STORAGE,
							     .read = tlv_storage_ram_read,
							     .write = tlv_storage_ram_write } };

tlv_ctx RAM_TLV_OBJECT_LAST_OFFSET_LE_FIRST =
	(tlv_ctx){ .start_offset = 0,
		   .end_offset = 0,
		   .storage_impl = { .ctx = TLV_RAM_STORAGE,
				     .read = tlv_storage_ram_read,
				     .write = tlv_storage_ram_write } };

tlv_ctx RAM_TLV_OBJECT_LAST_OFFSET_ONLY_HEADER =
	(tlv_ctx){ .start_offset = 0,
		   .end_offset = 5,
		   .storage_impl = { .ctx = TLV_RAM_STORAGE,
				     .read = tlv_storage_ram_read,
				     .write = tlv_storage_ram_write } };

/**
 * @brief Set up for test.
 * 
 * @param f Pointer to fixture.
 */
static void setUp(void *f)
{
	memset(TLV_RAM_STORAGE, 0x0, sizeof(TLV_RAM_STORAGE));
}

ZTEST_SUITE(valid_ctx, NULL, NULL, setUp, NULL, NULL);
ZTEST_SUITE(missing_read, NULL, NULL, setUp, NULL, NULL);
ZTEST_SUITE(missing_write, NULL, NULL, setUp, NULL, NULL);
ZTEST_SUITE(start_addr_offset, NULL, NULL, setUp, NULL, NULL);
ZTEST_SUITE(invalid_offset, NULL, NULL, setUp, NULL, NULL);

struct test_tlv_lookup_parameters {
	tlv_type tlv_type;
	int expected_return;
	tlv_ctx tlv;
	tlv_header header;
	uint8_t initial_storage_content[64];
};

static void test_tlv_lookup(struct test_tlv_lookup_parameters parameters)
{
	memcpy(TLV_RAM_STORAGE, parameters.initial_storage_content, 64);
	tlv_header header = {};
	int ret = tlv_lookup(&parameters.tlv, parameters.tlv_type, &header);
	zassert_equal(ret, parameters.expected_return);
	zassert_mem_equal(&header, &parameters.header, sizeof(header));
}

static void test_tlv_lookup_no_header(struct test_tlv_lookup_parameters parameters)
{
	memcpy(TLV_RAM_STORAGE, parameters.initial_storage_content, 64);
	int ret = tlv_lookup(&parameters.tlv, parameters.tlv_type, NULL);
	zassert_equal(ret, parameters.expected_return);
}

struct test_tlv_read_parameters {
	tlv_type tlv_type;
	int expected_return;
	tlv_ctx tlv;
	uint8_t expect_read_data[64];
	uint8_t read_size;
	uint8_t initial_storage_content[64];
};

static void test_tlv_read(struct test_tlv_read_parameters parameters)
{
	memcpy(TLV_RAM_STORAGE, parameters.initial_storage_content, 64);
	uint8_t read_data[64] = { 0 };
	int ret = tlv_read(&parameters.tlv, parameters.tlv_type, read_data, parameters.read_size);
	zassert_equal(ret, parameters.expected_return);
	zassert_mem_equal(read_data, parameters.expect_read_data, parameters.read_size);
}

struct test_tlv_write_parameters {
	tlv_type tlv_type;
	int expected_return;
	tlv_ctx tlv;
	uint8_t write_data[64];
	uint8_t write_size;
	uint8_t initial_storage_content[64];
	uint8_t final_storage_content[64];
};

static void test_tlv_write(struct test_tlv_write_parameters parameters)
{
	memcpy(TLV_RAM_STORAGE, parameters.initial_storage_content, 64);
	int ret = tlv_write(&parameters.tlv, parameters.tlv_type, parameters.write_data,
			    parameters.write_size);
	zassert_equal(ret, parameters.expected_return);
	zassert_mem_equal(TLV_RAM_STORAGE, parameters.final_storage_content, 64);
}

struct test_tlv_read_start_marker_parameters {
	int expected_return;
	tlv_ctx tlv;
	uint8_t expect_read_data[64];
	uint8_t read_size;
	uint8_t initial_storage_content[64];
};

static void test_tlv_read_start_marker(struct test_tlv_read_start_marker_parameters parameters)
{
	memcpy(TLV_RAM_STORAGE, parameters.initial_storage_content, 64);
	uint8_t read_data[64] = { 0 };
	int ret = tlv_read_start_marker(&parameters.tlv, read_data, parameters.read_size);
	zassert_equal(ret, parameters.expected_return);
	zassert_mem_equal(read_data, parameters.expect_read_data, parameters.read_size);
}

struct test_tlv_write_start_marker_parameters {
	int expected_return;
	tlv_ctx tlv;
	uint8_t write_data[64];
	uint8_t write_size;
	uint8_t initial_storage_content[64];
	uint8_t final_storage_content[64];
};

static void test_tlv_write_start_marker(struct test_tlv_write_start_marker_parameters parameters)
{
	memcpy(TLV_RAM_STORAGE, parameters.initial_storage_content, 64);
	int ret = tlv_write_start_marker(&parameters.tlv, parameters.write_data,
					 parameters.write_size);
	zassert_equal(ret, parameters.expected_return);
	zassert_mem_equal(TLV_RAM_STORAGE, parameters.final_storage_content, 64);
}
/* tlv lookup */
PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0_ram_empty, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .tlv_type = 0,
			  .expected_return = 0,
			  .header = { .type = 0,
				      .payload_size = { .data_size = 0, .padding = 0 } } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0xff_ram_empty, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){ .tlv = RAM_TLV_OBJECT,
						       .tlv_type = 0xFF,
						       .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0xff_ram_empty_no_header, test_tlv_lookup_no_header,
		  (struct test_tlv_lookup_parameters){ .tlv = RAM_TLV_OBJECT,
						       .tlv_type = 0xFF,
						       .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_1_ram_empty, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){ .tlv = RAM_TLV_OBJECT,
						       .tlv_type = 1,
						       .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_max_1_ram_empty, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){ .tlv = RAM_TLV_OBJECT,
						       .tlv_type = UINT16_MAX - 1,
						       .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_max_ram_empty, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){ .tlv = RAM_TLV_OBJECT,
						       .tlv_type = UINT16_MAX,
						       .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_rndom_ram_empty, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){ .tlv = RAM_TLV_OBJECT,
						       .tlv_type = 123,
						       .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x0, 0x0, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 0,
			  .expected_return = 0,
			  .header = { .type = 0,
				      .payload_size = { .padding = 0, .data_size = 4 } } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_1_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = 0,
			  .header = { .type = 1,
				      .payload_size = { .padding = 0, .data_size = 4 } } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_1_filled_no_header, test_tlv_lookup_no_header,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = 0,
		  })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0xff_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x0, 0xff, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 0xff,
			  .expected_return = 0,
			  .header = { .type = 0xff,
				      .payload_size = { .padding = 0, .data_size = 4 } } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0x100_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3 },
			  .tlv_type = 0x100,
			  .expected_return = 0,
			  .header = { .type = 0x100,
				      .payload_size = { .padding = 1, .data_size = 3 } } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0x100_first_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0x0, 0x0,
						       0xff, 0x3, 0x1, 0xff },
			  .tlv_type = 0x100,
			  .expected_return = 0,
			  .header = { .type = 0x100,
				      .payload_size = { .padding = 1, .data_size = 3 } } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0xff_second_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0x0, 0x0,
						       0xff, 0x3, 0x1, 0xff },
			  .tlv_type = 0xff,
			  .expected_return = 0,
			  .header = { .type = 0xff,
				      .payload_size = { .padding = 3, .data_size = 1 } } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_lookup_0x102_second_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0x0, 0x1,
						       0x2, 0x3, 0x1, 0xff },
			  .tlv_type = 0x102,
			  .expected_return = 0,
			  .header = { .type = 0x102,
				      .payload_size = { .padding = 3, .data_size = 1 } } })

/* tlv read */
PARAMETRIZED_TEST(valid_ctx, test_tlv_read_0_ram_empty, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .tlv_type = 0,
			  .expected_return = 0,
		  })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_0xff_ram_empty, test_tlv_read,
		  (struct test_tlv_read_parameters){ .tlv = RAM_TLV_OBJECT,
						     .tlv_type = 0xFF,
						     .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_1_ram_empty, test_tlv_read,
		  (struct test_tlv_read_parameters){ .tlv = RAM_TLV_OBJECT,
						     .tlv_type = 1,
						     .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_max_1_ram_empty, test_tlv_read,
		  (struct test_tlv_read_parameters){ .tlv = RAM_TLV_OBJECT,
						     .tlv_type = UINT16_MAX - 1,
						     .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_max_ram_empty, test_tlv_read,
		  (struct test_tlv_read_parameters){ .tlv = RAM_TLV_OBJECT,
						     .tlv_type = UINT16_MAX,
						     .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_rndom_ram_empty, test_tlv_read,
		  (struct test_tlv_read_parameters){ .tlv = RAM_TLV_OBJECT,
						     .tlv_type = 123,
						     .expected_return = -ENODATA })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_0_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x0, 0x0, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 0,
			  .expected_return = 0,
			  .read_size = 4,
			  .expect_read_data = { 0x1, 0x2, 0x3, 0x4 } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_1_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = 0,
			  .read_size = 4,
			  .expect_read_data = { 0x1, 0x2, 0x3, 0x4 } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_0xff_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x0, 0xff, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 0xff,
			  .expected_return = 0,
			  .read_size = 4,
			  .expect_read_data = { 0x1, 0x2, 0x3, 0x4 } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_0x100_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff },
			  .tlv_type = 0x100,
			  .expected_return = 0,
			  .read_size = 3,
			  .expect_read_data = { 0x1, 0x2, 0x3 } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_0x100_first_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff,
						       0x0, 0xff, 0x3, 0x1, 0xff, 0xff, 0xff,
						       0xff },
			  .tlv_type = 0x100,
			  .expected_return = 0,
			  .read_size = 3,
			  .expect_read_data = { 0x1, 0x2, 0x3 } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_0xff_second_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff,
						       0x0, 0xff, 0x3, 0x1, 0xff, 0xff, 0xff,
						       0xff },
			  .tlv_type = 0xff,
			  .expected_return = 0,
			  .read_size = 1,
			  .expect_read_data = { 0xff } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_0x102_second_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff,
						       0x1, 0x2, 0x3, 0x1, 0xff },
			  .tlv_type = 0x102,
			  .expected_return = 0,
			  .read_size = 1,
			  .expect_read_data = { 0xff } })

/* tlv write */

PARAMETRIZED_TEST(valid_ctx, test_tlv_write_empty, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x0 },
			  .tlv_type = 0x100,
			  .expected_return = 0,
			  .write_data = { 0x1, 0x2, 0x3 },
			  .write_size = 3,
			  .final_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_write_second, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff },
			  .tlv_type = 0x101,
			  .expected_return = 0,
			  .write_data = { 0x1, 0x2, 0x3, 0x4 },
			  .write_size = 4,
			  .final_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff, 0x1,
						     0x01, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_write_floded_ff, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT,
			  .initial_storage_content = { [0 ... 63] = 0xff },
			  .tlv_type = 0x101,
			  .expected_return = 0,
			  .write_data = { 0x1, 0x2, 0x3, 0x4 },
			  .write_size = 4,
			  .final_storage_content = { 0x01, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4,
						     [8 ... 63] = 0xff } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_write_floded_ff_magic, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT_WITH_MAGIC,
			  .initial_storage_content = { [0 ... 63] = 0xff },
			  .tlv_type = 0x101,
			  .expected_return = 0,
			  .write_data = { 0x1, 0x2, 0x3, 0x4 },
			  .write_size = 4,
			  .final_storage_content = { [0 ... 7] = 0xff,
						     0x01,
						     0x1,
						     0x0,
						     0x4,
						     0x1,
						     0x2,
						     0x3,
						     0x4,
						     [16 ... 63] = 0xff } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_start_marker_real, test_tlv_read_start_marker,
		  (struct test_tlv_read_start_marker_parameters){
			  .tlv = RAM_TLV_OBJECT_WITH_MAGIC,
			  .initial_storage_content = { 0x53, 0x49, 0x44, 0x30, 0x00, 0x00, 0x00,
						       0x08, [8 ... 63] = 0xff },
			  .expected_return = 0,
			  .expect_read_data = { 0x53, 0x49, 0x44, 0x30, 0x00, 0x00, 0x00, 0x08 },
			  .read_size = 8 })

PARAMETRIZED_TEST(valid_ctx, test_tlv_write_start_marker_real, test_tlv_write_start_marker,
		  (struct test_tlv_write_start_marker_parameters){
			  .tlv = RAM_TLV_OBJECT_WITH_MAGIC,
			  .initial_storage_content = { [0 ... 63] = 0xff },
			  .write_data = { 0x53, 0x49, 0x44, 0x30, 0x00, 0x00, 0x00, 0x08 },
			  .write_size = 8,
			  .expected_return = 0,
			  .final_storage_content = { 0x53, 0x49, 0x44, 0x30, 0x00, 0x00, 0x00, 0x08,
						     [8 ... 63] = 0xff } })

PARAMETRIZED_TEST(valid_ctx, test_tlv_read_start_marker_real_nomem, test_tlv_read_start_marker,
		  (struct test_tlv_read_start_marker_parameters){
			  .tlv = RAM_TLV_OBJECT_WITH_MAGIC,
			  .initial_storage_content = { 0x53, 0x49, 0x44, 0x30, 0x00, 0x00, 0x00,
						       0x08, [8 ... 63] = 0xff },
			  .expected_return = -ENOMEM,
			  .expect_read_data = { 0x0 },
			  .read_size = 5 })

PARAMETRIZED_TEST(valid_ctx, test_tlv_write_start_marker_real_nomem, test_tlv_write_start_marker,
		  (struct test_tlv_write_start_marker_parameters){
			  .tlv = RAM_TLV_OBJECT_WITH_MAGIC,
			  .initial_storage_content = { [0 ... 63] = 0xff },
			  .write_data = { 0x53, 0x49, 0x44, 0x30, 0x00, 0x00, 0x00, 0x08 },
			  .write_size = 12,
			  .expected_return = -ENOMEM,
			  .final_storage_content = { [0 ... 63] = 0xff } })
/* ctx not fully provided */

PARAMETRIZED_TEST(missing_read, test_tlv_read_start_marker_real, test_tlv_read_start_marker,
		  (struct test_tlv_read_start_marker_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_READ,
			  .initial_storage_content = { 0x53, 0x49, 0x44, 0x30, 0x00, 0x00, 0x00,
						       0x08, [8 ... 63] = 0xff },
			  .expected_return = -EINVAL,
			  .expect_read_data = { 0 },
			  .read_size = 8 })

PARAMETRIZED_TEST(missing_read, test_tlv_lookup_1_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_READ,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = -EINVAL,
		  })

PARAMETRIZED_TEST(missing_read, test_tlv_read_0x100_first_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_READ,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff,
						       0x0, 0xff, 0x3, 0x1, 0xff, 0xff, 0xff,
						       0xff },
			  .tlv_type = 0x100,
			  .expected_return = -EINVAL,
			  .read_size = 3,
			  .expect_read_data = { 0 } })

PARAMETRIZED_TEST(missing_read, test_tlv_write_empty, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_READ,
			  .initial_storage_content = { 0x0 },
			  .tlv_type = 0x100,
			  .expected_return = -EINVAL,
			  .write_data = { 0x1, 0x2, 0x3 },
			  .write_size = 3,
		  })

PARAMETRIZED_TEST(missing_write, test_tlv_write_start_marker_real, test_tlv_write_start_marker,
		  (struct test_tlv_write_start_marker_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_WRITE,
			  .initial_storage_content = { [0 ... 63] = 0xff },
			  .write_data = { 0x53, 0x49, 0x44, 0x30, 0x00, 0x00, 0x00, 0x08 },
			  .write_size = 8,
			  .expected_return = -EINVAL,
			  .final_storage_content = { [0 ... 63] = 0xff } })

PARAMETRIZED_TEST(missing_write, test_tlv_lookup_1_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_WRITE,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = 0,
			  .header = { .type = 1,
				      .payload_size = { .padding = 0, .data_size = 4 } } })

PARAMETRIZED_TEST(missing_write, test_tlv_read_0x100_first_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_WRITE,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff,
						       0x0, 0xff, 0x3, 0x1, 0xff, 0xff, 0xff,
						       0xff },
			  .tlv_type = 0x100,
			  .expected_return = 0,
			  .read_size = 3,
			  .expect_read_data = { 0x1, 0x2, 0x3 } })

PARAMETRIZED_TEST(missing_write, test_tlv_write_empty, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_WRITE,
			  .initial_storage_content = { 0x0 },
			  .tlv_type = 0x100,
			  .expected_return = -EINVAL,
			  .write_data = { 0x1, 0x2, 0x3 },
			  .write_size = 3,
		  })

PARAMETRIZED_TEST(missing_write, test_tlv_write_second, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT_NO_WRITE,
			  .initial_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff },
			  .tlv_type = 0x101,
			  .expected_return = -EINVAL,
			  .write_data = { 0x1, 0x2, 0x3, 0x4 },
			  .write_size = 4,
			  .final_storage_content = { 0x1, 0x00, 0x1, 0x3, 0x1, 0x2, 0x3, 0xff } })

PARAMETRIZED_TEST(start_addr_offset, test_tlv_write_empty, test_tlv_write,
		  (struct test_tlv_write_parameters){ .tlv = RAM_TLV_OBJECT_OFFSET,
						      .initial_storage_content = { 0x0 },
						      .tlv_type = 0x100,
						      .expected_return = 0,
						      .write_data = { 0x1, 0x2, 0x3 },
						      .write_size = 3,
						      .final_storage_content = { [0 ... 2] = 0x0,
										 0x1,
										 0x00,
										 0x1,
										 0x3,
										 0x1,
										 0x2,
										 0x3,
										 0xff } })

PARAMETRIZED_TEST(start_addr_offset, test_tlv_write_second, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT_OFFSET,
			  .initial_storage_content = { 0xaa, 0xab, 0xac, 0x1, 0x00, 0x1, 0x3, 0x1,
						       0x2, 0x3, 0xff },
			  .tlv_type = 0x101,
			  .expected_return = 0,
			  .write_data = { 0x1, 0x2, 0x3, 0x4 },
			  .write_size = 4,
			  .final_storage_content = { 0xaa, 0xab, 0xac, 0x1, 0x00, 0x1, 0x3, 0x1,
						     0x2, 0x3, 0xff, 0x1, 0x01, 0x0, 0x4, 0x1, 0x2,
						     0x3, 0x4 } })

PARAMETRIZED_TEST(start_addr_offset, test_tlv_write_floded_ff, test_tlv_write,
		  (struct test_tlv_write_parameters){
			  .tlv = RAM_TLV_OBJECT_OFFSET,
			  .initial_storage_content = { [0 ... 63] = 0xff },
			  .tlv_type = 0x101,
			  .expected_return = 0,
			  .write_data = { 0x1, 0x2, 0x3, 0x4 },
			  .write_size = 4,
			  .final_storage_content = { [0 ... 2] = 0xff,
						     0x01,
						     0x1,
						     0x0,
						     0x4,
						     0x1,
						     0x2,
						     0x3,
						     0x4,
						     [11 ... 63] = 0xff } })

PARAMETRIZED_TEST(invalid_offset, test_tlv_write_empty, test_tlv_write,
		  (struct test_tlv_write_parameters){ .tlv = RAM_TLV_OBJECT_LAST_OFFSET_LE_FIRST,
						      .initial_storage_content = { 0x0 },
						      .tlv_type = 0x100,
						      .expected_return = -ENOMEM,
						      .write_data = { 0x1, 0x2, 0x3 },
						      .write_size = 3 })

PARAMETRIZED_TEST(invalid_offset, test_tlv_lookup_1_filled, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT_LAST_OFFSET_LE_FIRST,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = -ENODATA })

PARAMETRIZED_TEST(invalid_offset, test_tlv_read_1_filled, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT_LAST_OFFSET_LE_FIRST,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = -ENODATA,
			  .read_size = 4,
			  .expect_read_data = { 0 } })

PARAMETRIZED_TEST(invalid_offset, test_tlv_lookup_1_filled_last_offset_too_small, test_tlv_lookup,
		  (struct test_tlv_lookup_parameters){
			  .tlv = RAM_TLV_OBJECT_LAST_OFFSET_ONLY_HEADER,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = 0,
			  .header = { .type = 1,
				      .payload_size = { .padding = 0, .data_size = 4 } } })

PARAMETRIZED_TEST(invalid_offset, test_tlv_read_1_filled_last_offset_too_small, test_tlv_read,
		  (struct test_tlv_read_parameters){
			  .tlv = RAM_TLV_OBJECT_LAST_OFFSET_ONLY_HEADER,
			  .initial_storage_content = { 0x0, 0x1, 0x0, 0x4, 0x1, 0x2, 0x3, 0x4 },
			  .tlv_type = 1,
			  .expected_return = -ENODATA,
			  .read_size = 4,
			  .expect_read_data = { 0 } })
