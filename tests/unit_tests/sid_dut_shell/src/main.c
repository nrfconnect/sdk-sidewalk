/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "fff.h"
#include "toolchain/gcc.h"
#include "ztest_assert.h"
#include <asm-generic/errno-base.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/ztest.h>

/* header under test */
#include <sid_dut_shell.h>
#include <sid_900_cfg.h>

/* mocks and dummy headers */
#include <mock/sid_api_delegated_mock.h>
#include <mock/shell_mock.h>
#include <sid_thread.h>

// sidewalk handle pointer used in valid initialization
static struct sid_handle *sidewalk_handle = NULL;
static struct app_context app_ctx_valid;
static struct sid_config sid_cfg_valid;
static struct app_context app_ctx_invalid;
static struct sid_config sid_cfg_invalid;

// clang-format off
#define PARAMETRIZED_TEST(siute, name, test_foo, ...) ZTEST(siute, name){ test_foo(__VA_ARGS__); }
// clang-format on

// ////////////////////////////////////////////////////////////////////////////
// sid_dut_shell_api_shell_uninitialized
// ////////////////////////////////////////////////////////////////////////////

struct test_init_parameters {
	int argc;
	const char **argv;
	int return_code;
	enum sid_link_type expected_link_type;
};

void test_sid_init(struct test_init_parameters parameters)
{
	int ret = cmd_sid_init(NULL, parameters.argc, parameters.argv);

	zassert_equal(parameters.return_code, ret, "Returned error code %d", ret);
	if (parameters.return_code == 0) {
		zassert_equal(1, sid_init_delegated_fake.call_count);
		zassert_equal_ptr(&sid_cfg_valid, sid_init_delegated_fake.arg0_val,
				  "Invalid sid_config passed");
		zassert_equal(parameters.expected_link_type, sid_cfg_valid.link_mask,
			      "Invalid link mask has been set");
		zassert_equal_ptr(&sidewalk_handle, sid_init_delegated_fake.arg1_val,
				  "Invalid sidewalk_handle passed");
	} else {
		zassert_equal(0, sid_init_delegated_fake.call_count);
	}
}

PARAMETRIZED_TEST(sid_dut_shell_api, test_init_0_args, test_sid_init,
		  (struct test_init_parameters){ .argc = 1,
						 .argv = (const char *[]){ "init" },
						 .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_init_1, test_sid_init,
		  (struct test_init_parameters){ .argc = 2,
						 .argv = (const char *[]){ "init", "1" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_1 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_init_2, test_sid_init,
		  (struct test_init_parameters){ .argc = 2,
						 .argv = (const char *[]){ "init", "2" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_2 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_init_3, test_sid_init,
		  (struct test_init_parameters){ .argc = 2,
						 .argv = (const char *[]){ "init", "3" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_3 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_init_4, test_sid_init,
		  (struct test_init_parameters){ .argc = 2,
						 .argv = (const char *[]){ "init", "4" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_1 |
								       SID_LINK_TYPE_3 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_init_5, test_sid_init,
		  (struct test_init_parameters){ .argc = 2,
						 .argv = (const char *[]){ "init", "5" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_1 |
								       SID_LINK_TYPE_2 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_init_6, test_sid_init,
		  (struct test_init_parameters){ .argc = 2,
						 .argv = (const char *[]){ "init", "6" },
						 .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_init_6_1, test_sid_init,
		  (struct test_init_parameters){ .argc = 3,
						 .argv = (const char *[]){ "init", "6", "1" },
						 .return_code = -EINVAL })
// ////////////////////////////////////////////////////////////////////////////

ZTEST(sid_dut_shell_api, test_sid_deinit)
{
	const char *argv[] = { "deinit" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_deinit(NULL, argc, argv);

	zassert_equal(0, ret, "Returned error code %d", ret);
	zassert_equal(1, sid_deinit_delegated_fake.call_count);
}

ZTEST(sid_dut_shell_api, test_sid_deinit_1)
{
	const char *argv[] = { "deinit", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_deinit(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_deinit_delegated_fake.call_count);
}

// ////////////////////////////////////////////////////////////////////////////

struct test_start_parameters {
	int argc;
	const char **argv;
	int return_code;
	enum sid_link_type expected_link_type;
};

void test_sid_start(struct test_start_parameters parameters)
{
	int ret = cmd_sid_start(NULL, parameters.argc, parameters.argv);

	zassert_equal(parameters.return_code, ret, "Returned error code %d", ret);
	if (parameters.return_code == 0) {
		zassert_equal(1, sid_start_delegated_fake.call_count);
		zassert_equal_ptr(sidewalk_handle, sid_start_delegated_fake.arg0_val,
				  "Invalid sidewalk handle");
		zassert_equal(parameters.expected_link_type, sid_start_delegated_fake.arg1_val,
			      "Invalid link_mask");
	} else {
		zassert_equal(0, sid_start_delegated_fake.call_count);
	}
}

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_start_no_argument, test_sid_start,
		  (struct test_start_parameters){ .argc = 1,
						  .argv = (const char *[]){ "start" },
						  .return_code = 0,
						  .expected_link_type = sid_cfg_valid.link_mask })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_start_1, test_sid_start,
		  (struct test_start_parameters){ .argc = 2,
						  .argv = (const char *[]){ "start", "1" },
						  .return_code = 0,
						  .expected_link_type = SID_LINK_TYPE_1 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_start_2, test_sid_start,
		  (struct test_start_parameters){ .argc = 2,
						  .argv = (const char *[]){ "start", "2" },
						  .return_code = 0,
						  .expected_link_type = SID_LINK_TYPE_2 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_start_3, test_sid_start,
		  (struct test_start_parameters){ .argc = 2,
						  .argv = (const char *[]){ "start", "3" },
						  .return_code = 0,
						  .expected_link_type = SID_LINK_TYPE_3 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_start_4, test_sid_start,
		  (struct test_start_parameters){ .argc = 2,
						  .argv = (const char *[]){ "start", "4" },
						  .return_code = 0,
						  .expected_link_type = SID_LINK_TYPE_1 |
									SID_LINK_TYPE_3 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_start_5, test_sid_start,
		  (struct test_start_parameters){ .argc = 2,
						  .argv = (const char *[]){ "start", "5" },
						  .return_code = 0,
						  .expected_link_type = SID_LINK_TYPE_1 |
									SID_LINK_TYPE_2 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_start_6, test_sid_start,
		  (struct test_start_parameters){ .argc = 2,
						  .argv = (const char *[]){ "start", "6" },
						  .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_start_6_1, test_sid_start,
		  (struct test_start_parameters){ .argc = 3,
						  .argv = (const char *[]){ "start", "6", "1" },
						  .return_code = -EINVAL })

// ////////////////////////////////////////////////////////////////////////////
struct test_stop_parameters {
	int argc;
	const char **argv;
	int return_code;
	enum sid_link_type expected_link_type;
};

void test_sid_stop(struct test_stop_parameters parameters)
{
	int ret = cmd_sid_stop(NULL, parameters.argc, parameters.argv);

	zassert_equal(parameters.return_code, ret, "Returned error code %d", ret);
	if (parameters.return_code == 0) {
		zassert_equal(1, sid_stop_delegated_fake.call_count);
		zassert_equal_ptr(sidewalk_handle, sid_stop_delegated_fake.arg0_val,
				  "Invalid sidewalk handle");
		zassert_equal(parameters.expected_link_type, sid_stop_delegated_fake.arg1_val,
			      "Invalid link_mask");
	} else {
		zassert_equal(0, sid_stop_delegated_fake.call_count);
	}
}

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_stop_no_argument, test_sid_stop,
		  (struct test_stop_parameters){ .argc = 1,
						 .argv = (const char *[]){ "stop" },
						 .return_code = 0,
						 .expected_link_type = sid_cfg_valid.link_mask })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_stop_1, test_sid_stop,
		  (struct test_stop_parameters){ .argc = 2,
						 .argv = (const char *[]){ "stop", "1" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_1 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_stop_2, test_sid_stop,
		  (struct test_stop_parameters){ .argc = 2,
						 .argv = (const char *[]){ "stop", "2" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_2 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_stop_3, test_sid_stop,
		  (struct test_stop_parameters){ .argc = 2,
						 .argv = (const char *[]){ "stop", "3" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_3 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_stop_4, test_sid_stop,
		  (struct test_stop_parameters){ .argc = 2,
						 .argv = (const char *[]){ "stop", "4" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_1 |
								       SID_LINK_TYPE_3 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_stop_5, test_sid_stop,
		  (struct test_stop_parameters){ .argc = 2,
						 .argv = (const char *[]){ "stop", "5" },
						 .return_code = 0,
						 .expected_link_type = SID_LINK_TYPE_1 |
								       SID_LINK_TYPE_2 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_stop_6, test_sid_stop,
		  (struct test_stop_parameters){ .argc = 2,
						 .argv = (const char *[]){ "stop", "6" },
						 .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_stop_6_1, test_sid_stop,
		  (struct test_stop_parameters){ .argc = 3,
						 .argv = (const char *[]){ "stop", "6", "1" },
						 .return_code = -EINVAL })

// ////////////////////////////////////////////////////////////////////////////
struct test_send_parameters {
	int argc;
	const char **argv;
	int return_code;
	struct sid_msg msg;
	struct sid_msg_desc desc;
};

void test_sid_send(struct test_send_parameters parameters)
{
	int ret = cmd_sid_send(NULL, parameters.argc, parameters.argv);

	zassert_equal(parameters.return_code, ret, "Returned error code %d", ret);
	if (parameters.return_code == 0) {
		zassert_equal(1, sid_put_msg_delegated_fake.call_count);
		zassert_equal_ptr(sid_put_msg_delegated_fake.arg0_val, sidewalk_handle,
				  "Invalid sidewalk handle");
		zassert_equal(sid_put_msg_delegated_fake.arg1_val->size, parameters.msg.size,
			      "Invalid message size");
		zassert_mem_equal(sid_put_msg_delegated_fake.arg1_val->data, parameters.msg.data,
				  parameters.msg.size, "Invalid message");
		zassert_equal(sid_put_msg_delegated_fake.arg2_val->type, parameters.desc.type,
			      "Invalid link type");
		zassert_equal(sid_put_msg_delegated_fake.arg2_val->link_mode,
			      parameters.desc.link_mode, "Invalid link mode type");
		zassert_equal(sid_put_msg_delegated_fake.arg2_val->id, parameters.desc.id,
			      "Invalid desc.id");
		zassert_equal(
			sid_put_msg_delegated_fake.arg2_val->msg_desc_attr.tx_attr.request_ack,
			parameters.desc.msg_desc_attr.tx_attr.request_ack,
			"Invalid desc.msg_desc_attr.tx_attr.request_ack");
		zassert_equal(
			sid_put_msg_delegated_fake.arg2_val->msg_desc_attr.tx_attr.num_retries,
			parameters.desc.msg_desc_attr.tx_attr.num_retries,
			"Invalid desc.msg_desc_attr.tx_attr.num_retries");
		zassert_equal(
			sid_put_msg_delegated_fake.arg2_val->msg_desc_attr.tx_attr.ttl_in_seconds,
			parameters.desc.msg_desc_attr.tx_attr.ttl_in_seconds,
			"Invalid desc.msg_desc_attr.tx_attr.ttl_in_seconds");
	} else {
		zassert_equal(0, sid_put_msg_delegated_fake.call_count);
	}
}

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_plain, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 2,
			  .argv = (const char *[]){ "send", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .size = strlen("ASCII_text_as_data"),
						   .data = "ASCII_text_as_data" },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_t0, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-t", "0", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_GET,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_t1, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-t", "1", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_SET,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_t2, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-t", "2", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_t3, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-t", "3", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_RESPONSE,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_t4, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-t", "4", "ASCII_text_as_data" },
			  .return_code = -EINVAL,
		  })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_t_missing, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 3,
			  .argv = (const char *[]){ "send", "-t", "ASCII_text_as_data" },
			  .return_code = -EINVAL,
		  })
PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_t_missing2, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 2,
			  .argv =
				  (const char *[]){
					  "send",
					  "-t",
				  },
			  .return_code = -EINVAL,
		  })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_d1, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-d", "1", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_d2, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-d", "2", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_MOBILE } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_d0, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-d", "0", "ASCII_text_as_data" },
			  .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_d3, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-d", "3", "ASCII_text_as_data" },
			  .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_d_missing, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 3,
			  .argv = (const char *[]){ "send", "-d", "ASCII_text_as_data" },
			  .return_code = -EINVAL,
		  })
PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_ascii_d_missing2, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 2,
			  .argv =
				  (const char *[]){
					  "send",
					  "-d",
				  },
			  .return_code = -EINVAL,
		  })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_r1b, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 3,
			  .argv = (const char *[]){ "send", "-r", "ff" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = (char[]){ 0xff }, .size = 1 },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_r2b, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 3,
			  .argv = (const char *[]){ "send", "-r", "ffab" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = (char[]){ 0xff, 0xab }, .size = 2 },
			  .desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_r_with_ascii, test_sid_send,
	(struct test_send_parameters){
		.argc = 4,
		.argv = (const char *[]){ "send", "-r", "deadbeef", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = (char[]){ 0xde, 0xad, 0xbe, 0xef }, .size = 4 },
		.desc = (struct sid_msg_desc){ .type = SID_MSG_TYPE_NOTIFY,
					       .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_l_1_with_ascii, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-l", "1", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1,
							 .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_l_2_with_ascii, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-l", "2", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_2,
							 .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_l_3_with_ascii, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-l", "3", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_3,
							 .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_l_4_with_ascii, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-l", "4", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 |
								      SID_LINK_TYPE_3,
							 .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_l_5_with_ascii, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-l", "5", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 |
								      SID_LINK_TYPE_2,
							 .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_l_6_with_ascii, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-l", "6", "ASCII_text_as_data" },
			  .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_l_0_with_ascii, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-l", "0", "ASCII_text_as_data" },
			  .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_l_no_arg_with_ascii, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 3,
			  .argv = (const char *[]){ "send", "-l", "ASCII_text_as_data" },
			  .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_i_123_not_response, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 4,
			  .argv = (const char *[]){ "send", "-i", "123", "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 |
								      SID_LINK_TYPE_2,
							 .type = SID_MSG_TYPE_NOTIFY,
							 .link_mode = SID_LINK_MODE_CLOUD,
							 .id = 0 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_i_123_response, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-i", "123", "-t", "3", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_RESPONSE,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .id = 123 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_i_0_response, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-i", "0", "-t", "3", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_RESPONSE,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .id = 0 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_i_UINT16_MAX_response, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-i", "0xFFFF", "-t", "3", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_RESPONSE,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .id = UINT16_MAX } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_i_UINT16_MAX1_response, test_sid_send,
		  (struct test_send_parameters){ .argc = 6,
						 .argv = (const char *[]){ "send", "-i", "0x10000",
									   "-t", "3",
									   "ASCII_text_as_data" },
						 .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_i_no_arg_response, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 5,
			  .argv = (const char *[]){ "send", "-i", "-t", "3", "ASCII_text_as_data" },
			  .return_code = -EINVAL })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_a_0_0_0_with_ascii, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-a", "0", "0", "0", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_NOTIFY,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .msg_desc_attr.tx_attr.request_ack = 0,
					       .msg_desc_attr.tx_attr.num_retries = 0,
					       .msg_desc_attr.tx_attr.ttl_in_seconds = 0 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_a_1_0_0_with_ascii, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-a", "1", "0", "0", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_NOTIFY,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .msg_desc_attr.tx_attr.request_ack = 1,
					       .msg_desc_attr.tx_attr.num_retries = 0,
					       .msg_desc_attr.tx_attr.ttl_in_seconds = 0 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_a_0_123_0_with_ascii, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-a", "0", "123", "0", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_NOTIFY,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .msg_desc_attr.tx_attr.request_ack = 0,
					       .msg_desc_attr.tx_attr.num_retries = 123,
					       .msg_desc_attr.tx_attr.ttl_in_seconds = 0 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_a_0_255_0_with_ascii, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-a", "0", "255", "0", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_NOTIFY,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .msg_desc_attr.tx_attr.request_ack = 0,
					       .msg_desc_attr.tx_attr.num_retries = 255,
					       .msg_desc_attr.tx_attr.ttl_in_seconds = 0 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_a_0_ff_0_with_ascii, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-a", "0", "0xff", "0", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_NOTIFY,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .msg_desc_attr.tx_attr.request_ack = 0,
					       .msg_desc_attr.tx_attr.num_retries = 255,
					       .msg_desc_attr.tx_attr.ttl_in_seconds = 0 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_a_0_255_123_with_ascii, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-a", "0", "255", "123", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_NOTIFY,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .msg_desc_attr.tx_attr.request_ack = 0,
					       .msg_desc_attr.tx_attr.num_retries = 255,
					       .msg_desc_attr.tx_attr.ttl_in_seconds = 123 } })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_send_hex_a_0_255_u16_max_with_ascii, test_sid_send,
	(struct test_send_parameters){
		.argc = 6,
		.argv = (const char *[]){ "send", "-a", "0", "255", "65535", "ASCII_text_as_data" },
		.return_code = 0,
		.msg = (struct sid_msg){ .data = "ASCII_text_as_data",
					 .size = strlen("ASCII_text_as_data") },
		.desc = (struct sid_msg_desc){ .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
					       .type = SID_MSG_TYPE_NOTIFY,
					       .link_mode = SID_LINK_MODE_CLOUD,
					       .msg_desc_attr.tx_attr.request_ack = 0,
					       .msg_desc_attr.tx_attr.num_retries = 255,
					       .msg_desc_attr.tx_attr.ttl_in_seconds = 65535 } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_a_0_255_u16_max_hex_with_ascii,
		  test_sid_send,
		  (struct test_send_parameters){
			  .argc = 6,
			  .argv = (const char *[]){ "send", "-a", "0", "255", "0xffff",
						    "ASCII_text_as_data" },
			  .return_code = 0,
			  .msg = (struct sid_msg){ .data = "ASCII_text_as_data",
						   .size = strlen("ASCII_text_as_data") },
			  .desc = (struct sid_msg_desc){
				  .link_type = SID_LINK_TYPE_1 | SID_LINK_TYPE_2,
				  .type = SID_MSG_TYPE_NOTIFY,
				  .link_mode = SID_LINK_MODE_CLOUD,
				  .msg_desc_attr.tx_attr.request_ack = 0,
				  .msg_desc_attr.tx_attr.num_retries = 255,
				  .msg_desc_attr.tx_attr.ttl_in_seconds = 65535 } })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_hex_r_0_255_missing_ttl_max_with_ascii,
		  test_sid_send,
		  (struct test_send_parameters){ .argc = 5,
						 .argv = (const char *[]){ "send", "-r", "0", "123",
									   "ASCII_text_as_data" },
						 .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_send_no_arg, test_sid_send,
		  (struct test_send_parameters){
			  .argc = 1,
			  .argv = (const char *[]){ "send" },
			  .return_code = -EINVAL,
		  })

// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api, test_sid_factory_reset)
{
	const char *argv[] = { "factory_reset" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_factory_reset(NULL, argc, argv);

	zassert_equal(0, ret, "Returned error code %d", ret);
	zassert_equal(1, sid_set_factory_reset_delegated_fake.call_count);
}

ZTEST(sid_dut_shell_api, test_sid_factory_reset_1)
{
	const char *argv[] = { "factory_reset", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_factory_reset(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_set_factory_reset_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////

struct test_get_mtu_params {
	int argc;
	const char **argv;
	int return_code;
	enum sid_link_type expected_link_type;
};

void test_sid_get_mtu(struct test_get_mtu_params params)
{
	int ret = cmd_sid_get_mtu(NULL, params.argc, params.argv);

	zassert_equal(params.return_code, ret, "Returned error code %d", ret);
	if (params.return_code == 0) {
		zassert_equal(1, sid_get_mtu_delegated_fake.call_count);
		zassert_equal_ptr(sidewalk_handle, sid_get_mtu_delegated_fake.arg0_val,
				  "Invalid sidewalk handle");
		zassert_equal(params.expected_link_type, sid_get_mtu_delegated_fake.arg1_val,
			      "Invalid Link type");
	} else {
		zassert_equal(0, sid_get_mtu_delegated_fake.call_count);
	}
}

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_mtu_1, test_sid_get_mtu,
		  (struct test_get_mtu_params){ .argc = 2,
						.argv = (const char *[]){ "get_mtu", "1" },
						.return_code = 0,
						.expected_link_type = SID_LINK_TYPE_1 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_mtu_2, test_sid_get_mtu,
		  (struct test_get_mtu_params){ .argc = 2,
						.argv = (const char *[]){ "get_mtu", "2" },
						.return_code = 0,
						.expected_link_type = SID_LINK_TYPE_2 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_mtu_3, test_sid_get_mtu,
		  (struct test_get_mtu_params){ .argc = 2,
						.argv = (const char *[]){ "get_mtu", "3" },
						.return_code = 0,
						.expected_link_type = SID_LINK_TYPE_3 })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_mtu_0, test_sid_get_mtu,
		  (struct test_get_mtu_params){ .argc = 2,
						.argv = (const char *[]){ "get_mtu", "0" },
						.return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_mtu_4, test_sid_get_mtu,
		  (struct test_get_mtu_params){ .argc = 2,
						.argv = (const char *[]){ "get_mtu", "4" },
						.return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_mtu_no_arg, test_sid_get_mtu,
		  (struct test_get_mtu_params){ .argc = 1,
						.argv = (const char *[]){ "get_mtu" },
						.return_code = -EINVAL })

// ////////////////////////////////////////////////////////////////////////////

ZTEST(sid_dut_shell_api, test_sid_last_status)
{
	const char *argv[] = { "last_status" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_last_status(NULL, argc, argv);

	zassert_equal(0, ret, "Returned error code %d", ret);
	zassert_equal_ptr(sidewalk_handle, sid_get_status_delegated_fake.arg0_val,
			  "Invalid sidewalk handle");
	zassert_equal(1, sid_get_status_delegated_fake.call_count);
}

ZTEST(sid_dut_shell_api, test_sid_last_status_arg)
{
	const char *argv[] = { "last_status", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_last_status(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_get_status_delegated_fake.call_count);
}

// ////////////////////////////////////////////////////////////////////////////

ZTEST(sid_dut_shell_api, test_sid_conn_request_invalid)
{
	const char *argv[] = { "conn_request" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_conn_request(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_ble_bcn_connection_request_delegated_fake.call_count);
}

ZTEST(sid_dut_shell_api, test_sid_conn_request_0)
{
	const char *argv[] = { "conn_request", "0" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_conn_request(NULL, argc, argv);

	zassert_equal(0, ret, "Returned error code %d", ret);
	zassert_equal(1, sid_ble_bcn_connection_request_delegated_fake.call_count);
	zassert_equal_ptr(sidewalk_handle, sid_ble_bcn_connection_request_delegated_fake.arg0_val,
			  "Invalid sidewalk handle");
	zassert_equal(false, sid_ble_bcn_connection_request_delegated_fake.arg1_val,
		      "Invalid reqiest value");
}

ZTEST(sid_dut_shell_api, test_sid_conn_request_1)
{
	const char *argv[] = { "conn_request", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_conn_request(NULL, argc, argv);

	zassert_equal(0, ret, "Returned error code %d", ret);
	zassert_equal(1, sid_ble_bcn_connection_request_delegated_fake.call_count);
	zassert_equal_ptr(sidewalk_handle, sid_ble_bcn_connection_request_delegated_fake.arg0_val,
			  "Invalid sidewalk handle");
	zassert_equal(true, sid_ble_bcn_connection_request_delegated_fake.arg1_val,
		      "Invalid reqiest value");
}

// ////////////////////////////////////////////////////////////////////////////

struct test_sid_get_time_params {
	int argc;
	const char **argv;
	int return_code;
	enum sid_time_format type;
};

void test_sid_get_time(struct test_sid_get_time_params params)
{
	int ret = cmd_sid_get_time(NULL, params.argc, params.argv);

	zassert_equal(params.return_code, ret, "Returned error code %d", ret);
	if (params.return_code == 0) {
		zassert_equal(1, sid_get_time_delegated_fake.call_count);
		zassert_equal_ptr(sidewalk_handle, sid_get_time_delegated_fake.arg0_val,
				  "Invalid sidewalk handle");
		zassert_equal(params.type, sid_get_time_delegated_fake.arg1_val,
			      "Invalid Link type");
	} else {
		zassert_equal(0, sid_get_time_delegated_fake.call_count);
	}
}

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_time_0, test_sid_get_time,
		  (struct test_sid_get_time_params){ .argc = 2,
						     .argv = (const char *[]){ "get_time", "0" },
						     .return_code = 0,
						     .type = SID_GET_GPS_TIME })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_time_1, test_sid_get_time,
		  (struct test_sid_get_time_params){ .argc = 2,
						     .argv = (const char *[]){ "get_time", "1" },
						     .return_code = 0,
						     .type = SID_GET_UTC_TIME })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_time_2, test_sid_get_time,
		  (struct test_sid_get_time_params){ .argc = 2,
						     .argv = (const char *[]){ "get_time", "2" },
						     .return_code = 0,
						     .type = SID_GET_LOCAL_TIME })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_time_3, test_sid_get_time,
		  (struct test_sid_get_time_params){ .argc = 2,
						     .argv = (const char *[]){ "get_time", "3" },
						     .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_time_missing_argument, test_sid_get_time,
		  (struct test_sid_get_time_params){ .argc = 1,
						     .argv = (const char *[]){ "get_time" },
						     .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_get_time_invalid_argument_number, test_sid_get_time,
		  (struct test_sid_get_time_params){
			  .argc = 3,
			  .argv = (const char *[]){ "get_time", "1", "2" },
			  .return_code = -EINVAL })

// ////////////////////////////////////////////////////////////////////////////

ZTEST(sid_dut_shell_api, test_sid_set_dst_id_0)
{
	const char *argv[] = { "set_dst_id", "0" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_set_dst_id(NULL, argc, argv);

	zassert_equal(0, ret, "Returned error code %d", ret);
	zassert_equal(1, sid_set_msg_dest_id_delegated_fake.call_count);
	zassert_equal_ptr(sidewalk_handle, sid_set_msg_dest_id_delegated_fake.arg0_val,
			  "Invalid sidewalk handle");
	zassert_equal(0, sid_set_msg_dest_id_delegated_fake.arg1_val, "Invalid Link type");
}

ZTEST(sid_dut_shell_api, test_sid_set_dst_id_1)
{
	const char *argv[] = { "set_dst_id", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_set_dst_id(NULL, argc, argv);

	zassert_equal(0, ret, "Returned error code %d", ret);
	zassert_equal(1, sid_set_msg_dest_id_delegated_fake.call_count);
	zassert_equal_ptr(sidewalk_handle, sid_set_msg_dest_id_delegated_fake.arg0_val,
			  "Invalid sidewalk handle");
	zassert_equal(1, sid_set_msg_dest_id_delegated_fake.arg1_val, "Invalid Link type");
}

ZTEST(sid_dut_shell_api, test_sid_set_dst_id_max)
{
	const char *argv[] = { "set_dst_id", "2147483647" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_set_dst_id(NULL, argc, argv);

	zassert_equal(0, ret, "Returned error code %d", ret);
	zassert_equal(1, sid_set_msg_dest_id_delegated_fake.call_count);
	zassert_equal_ptr(sidewalk_handle, sid_set_msg_dest_id_delegated_fake.arg0_val,
			  "Invalid sidewalk handle");
	zassert_equal(2147483647, sid_set_msg_dest_id_delegated_fake.arg1_val, "Invalid Link type");
}

ZTEST(sid_dut_shell_api, test_sid_set_dst_id_invalid_argument)
{
	const char *argv[] = { "set_dst_id" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_set_dst_id(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_set_msg_dest_id_delegated_fake.call_count);
}

ZTEST(sid_dut_shell_api, test_sid_set_dst_id_invalid_argument_2)
{
	const char *argv[] = { "set_dst_id", "1", "2" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_set_dst_id(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_set_msg_dest_id_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////

struct test_sid_set_option_params {
	int argc;
	const char **argv;
	int return_code;
	enum sid_option option;
	void *data;
	size_t len;
};

static void verify_sid_option_asserts(struct test_sid_set_option_params params, int ret)
{
	zassert_equal(params.return_code, ret, "Returned error code %d", ret);
	if (params.return_code == 0) {
		zassert_equal(1, sid_option_delegated_fake.call_count);
		zassert_equal_ptr(sidewalk_handle, sid_option_delegated_fake.arg0_val,
				  "Invalid sidewalk handle");
		zassert_equal(params.option, sid_option_delegated_fake.arg1_val,
			      "Invalid option parameter");
		zassert_equal(params.len, sid_option_delegated_fake.arg3_val,
			      "Invalid size parameter");
		zassert_mem_equal(params.data, sid_option_delegated_fake.arg2_val, params.len,
				  "Invalid data parameter");
	} else {
		zassert_equal(0, sid_option_delegated_fake.call_count);
	}
}

void test_sid_set_option_b(struct test_sid_set_option_params params)
{
	int ret = cmd_sid_option_battery(NULL, params.argc, params.argv);

	verify_sid_option_asserts(params, ret);
}

void test_sid_set_option_lp_get_l2(struct test_sid_set_option_params params)
{
	int ret = cmd_sid_option_lp_get_l2(NULL, params.argc, params.argv);

	verify_sid_option_asserts(params, ret);
}

void test_sid_set_option_lp_get_l3(struct test_sid_set_option_params params)
{
	int ret = cmd_sid_option_lp_get_l3(NULL, params.argc, params.argv);

	verify_sid_option_asserts(params, ret);
}

void test_sid_set_option_lp_set(struct test_sid_set_option_params params)
{
	int ret = cmd_sid_option_lp_set(NULL, params.argc, params.argv);

	verify_sid_option_asserts(params, ret);
}

void test_sid_set_option_d(struct test_sid_set_option_params params)
{
	int ret = cmd_sid_option_d(NULL, params.argc, params.argv);

	verify_sid_option_asserts(params, ret);
}

void test_sid_set_option_gd(struct test_sid_set_option_params params)
{
	int ret = cmd_sid_option_gd(NULL, params.argc, params.argv);

	verify_sid_option_asserts(params, ret);
}

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_get_l3, test_sid_set_option_lp_get_l3,
	(struct test_sid_set_option_params){
		.argc = 1,
		.argv = (const char *[]){ "-lp_get_l3" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = SID_LINK3_PROFILE_A } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_get_l2, test_sid_set_option_lp_get_l2,
	(struct test_sid_set_option_params){
		.argc = 1,
		.argv = (const char *[]){ "-lp_get_l2" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = SID_LINK2_PROFILE_1 } },
		.len = sizeof(struct sid_device_profile) })

static uint8_t batery_level_1 = 1;
PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_baterry_level, test_sid_set_option_b,
		  (struct test_sid_set_option_params){ .argc = 2,
						       .argv = (const char *[]){ "-b", "1" },
						       .return_code = 0,
						       .option = SID_OPTION_BLE_BATTERY_LEVEL,
						       .data = &batery_level_1,
						       .len = sizeof(batery_level_1) })

static uint8_t batery_level_100 = 100;
PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_baterry_level_100, test_sid_set_option_b,
		  (struct test_sid_set_option_params){ .argc = 2,
						       .argv = (const char *[]){ "-b", "100" },
						       .return_code = 0,
						       .option = SID_OPTION_BLE_BATTERY_LEVEL,
						       .data = &batery_level_100,
						       .len = sizeof(batery_level_100) })

static uint8_t batery_level_255 = 255;
PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_baterry_level_255, test_sid_set_option_b,
		  (struct test_sid_set_option_params){ .argc = 2,
						       .argv = (const char *[]){ "-b", "255" },
						       .return_code = 0,
						       .option = SID_OPTION_BLE_BATTERY_LEVEL,
						       .data = &batery_level_255,
						       .len = sizeof(batery_level_255) })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_baterry_level_ff, test_sid_set_option_b,
		  (struct test_sid_set_option_params){ .argc = 2,
						       .argv = (const char *[]){ "-b", "0xff" },
						       .return_code = 0,
						       .option = SID_OPTION_BLE_BATTERY_LEVEL,
						       .data = &batery_level_255,
						       .len = sizeof(batery_level_255) })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_baterry_level_not_number,
		  test_sid_set_option_b,
		  (struct test_sid_set_option_params){ .argc = 2,
						       .argv = (const char *[]){ "-b", "kot" },
						       .return_code = -EINVAL })

static uint8_t d_0 = 0;
PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_d_0, test_sid_set_option_d,
		  (struct test_sid_set_option_params){
			  .argc = 2,
			  .argv = (const char *[]){ "-d", "0" },
			  .return_code = 0,
			  .option = SID_OPTION_SET_MSG_POLICY_FILTER_DUPLICATES,
			  .data = &d_0,
			  .len = sizeof(d_0) })

static uint8_t d_1 = 1;
PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_d_1, test_sid_set_option_d,
		  (struct test_sid_set_option_params){
			  .argc = 2,
			  .argv = (const char *[]){ "-d", "1" },
			  .return_code = 0,
			  .option = SID_OPTION_SET_MSG_POLICY_FILTER_DUPLICATES,
			  .data = &d_1,
			  .len = sizeof(d_1) })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_d_2, test_sid_set_option_d,
		  (struct test_sid_set_option_params){ .argc = 2,
						       .argv = (const char *[]){ "-d", "2" },
						       .return_code = -EINVAL })

static uint8_t gd = 0;
PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_gd, test_sid_set_option_gd,
		  (struct test_sid_set_option_params){
			  .argc = 1,
			  .argv = (const char *[]){ "-gd" },
			  .return_code = 0,
			  .option = SID_OPTION_GET_MSG_POLICY_FILTER_DUPLICATES,
			  .data = &gd,
			  .len = sizeof(gd) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_1, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 2,
		.argv = (const char *[]){ "-lp_set", "1" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 1,
						    .rx_window_count = 0,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_0x01, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 2,
		.argv = (const char *[]){ "-lp_set", "0x01" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 1,
						    .rx_window_count = 0,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_2, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 2,
		.argv = (const char *[]){ "-lp_set", "2" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 2,
						    .rx_window_count = 0,
						    .unicast_window_interval.sync_rx_interval_ms =
							    SID_LINK2_RX_WINDOW_SEPARATION_1,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_2_63, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "2", "63" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 2,
						    .rx_window_count = 0,
						    .unicast_window_interval.sync_rx_interval_ms =
							    SID_LINK2_RX_WINDOW_SEPARATION_1,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_2_315, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "2", "315" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 2,
						    .rx_window_count = 0,
						    .unicast_window_interval.sync_rx_interval_ms =
							    SID_LINK2_RX_WINDOW_SEPARATION_2,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_2_630, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "2", "630" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 2,
						    .rx_window_count = 0,
						    .unicast_window_interval.sync_rx_interval_ms =
							    SID_LINK2_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_2_945, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "2", "945" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 2,
						    .rx_window_count = 0,
						    .unicast_window_interval.sync_rx_interval_ms =
							    SID_LINK2_RX_WINDOW_SEPARATION_4,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_2_2520, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "2", "2520" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 2,
						    .rx_window_count = 0,
						    .unicast_window_interval.sync_rx_interval_ms =
							    SID_LINK2_RX_WINDOW_SEPARATION_5,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_2_3150, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "2", "3150" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 2,
						    .rx_window_count = 0,
						    .unicast_window_interval.sync_rx_interval_ms =
							    SID_LINK2_RX_WINDOW_SEPARATION_6,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_2_5040, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "2", "5040" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 2,
						    .rx_window_count = 0,
						    .unicast_window_interval.sync_rx_interval_ms =
							    SID_LINK2_RX_WINDOW_SEPARATION_7,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_128_0, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "128", "0" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 128,
						    .rx_window_count = 0,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_0x80_0, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "0x80", "0" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 128,
						    .rx_window_count = 0,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_128_1, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "128", "1" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 128,
						    .rx_window_count = 1,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_128_5, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "128", "5" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 128,
						    .rx_window_count = SID_RX_WINDOW_CNT_2,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_128_10, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "128", "10" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 128,
						    .rx_window_count = SID_RX_WINDOW_CNT_3,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_128_15, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "128", "15" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 128,
						    .rx_window_count = SID_RX_WINDOW_CNT_4,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_128_20, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "128", "20" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 128,
						    .rx_window_count = SID_RX_WINDOW_CNT_5,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_128_65535, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "128", "65535" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 128,
						    .rx_window_count = SID_RX_WINDOW_CONTINUOUS,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_129_0, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "129", "0" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 129,
						    .rx_window_count = 0,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_0x81_0, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "0x81", "0" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 129,
						    .rx_window_count = 0,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_129_1, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "129", "1" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 129,
						    .rx_window_count = 1,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_129_5, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "129", "5" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 129,
						    .rx_window_count = SID_RX_WINDOW_CNT_2,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_129_10, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "129", "10" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 129,
						    .rx_window_count = SID_RX_WINDOW_CNT_3,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_129_15, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "129", "15" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 129,
						    .rx_window_count = SID_RX_WINDOW_CNT_4,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_129_20, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "129", "20" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 129,
						    .rx_window_count = SID_RX_WINDOW_CNT_5,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_129_65535, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "129", "65535" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 129,
						    .rx_window_count = SID_RX_WINDOW_CONTINUOUS,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_131_0, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "131", "0" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 131,
						    .rx_window_count = 0,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_0x83_0, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "0x83", "0" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 131,
						    .rx_window_count = 0,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_131_1, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "131", "1" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 131,
						    .rx_window_count = 1,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_131_5, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "131", "5" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 131,
						    .rx_window_count = SID_RX_WINDOW_CNT_2,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_131_10, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "131", "10" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 131,
						    .rx_window_count = SID_RX_WINDOW_CNT_3,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_131_15, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "131", "15" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 131,
						    .rx_window_count = SID_RX_WINDOW_CNT_4,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_131_20, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "131", "20" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 131,
						    .rx_window_count = SID_RX_WINDOW_CNT_5,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(
	sid_dut_shell_api, test_sid_set_option_lp_set_131_65535, test_sid_set_option_lp_set,
	(struct test_sid_set_option_params){
		.argc = 3,
		.argv = (const char *[]){ "-lp_set", "131", "65535" },
		.return_code = 0,
		.option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
		.data =
			&(struct sid_device_profile){
				.unicast_params = { .device_profile_id = 131,
						    .rx_window_count = SID_RX_WINDOW_CONTINUOUS,
						    .unicast_window_interval.async_rx_interval_ms =
							    SID_LINK3_RX_WINDOW_SEPARATION_3,
						    .wakeup_type = SID_TX_AND_RX_WAKEUP } },
		.len = sizeof(struct sid_device_profile) })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_baterry_level_256, test_sid_set_option_b,
		  (struct test_sid_set_option_params){ .argc = 3,
						       .argv = (const char *[]){ "-b", "256" },
						       .return_code = -EINVAL })

PARAMETRIZED_TEST(sid_dut_shell_api, test_sid_set_option_lp_set_0, test_sid_set_option_lp_set,
		  (struct test_sid_set_option_params){ .argc = 2,
						       .argv = (const char *[]){ "-lp_set", "0" },
						       .return_code = -EINVAL })

// ////////////////////////////////////////////////////////////////////////////
// sid_dut_shell_api_shell_uninitialized
// ////////////////////////////////////////////////////////////////////////////

ZTEST(sid_dut_shell_api_shell_uninitialized, test_init)
{
	const char *argv[] = { "init", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_init(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_init_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_deinit)
{
	const char *argv[] = { "deinit" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_deinit(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_deinit_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_start)
{
	const char *argv[] = { "start", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_start(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_start_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_stop)
{
	const char *argv[] = { "stop" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_stop(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_stop_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_send)
{
	const char *argv[] = { "send", "ASCII_text_as_data" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_send(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_put_msg_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_factory_reset)
{
	const char *argv[] = { "factory_reset" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_factory_reset(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_set_factory_reset_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_get_mtu)
{
	const char *argv[] = { "get_mtu", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_get_mtu(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_get_mtu_delegated_fake.call_count);
}

// ////////////////////////////////////////////////////////////////////////////

ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_last_status)
{
	const char *argv[] = { "last_status" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_last_status(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_get_status_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_conn_request)
{
	const char *argv[] = { "conn_request", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_conn_request(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_ble_bcn_connection_request_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_get_time)
{
	const char *argv[] = { "get_time", "0" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_get_time(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_get_time_delegated_fake.call_count);
}

// ////////////////////////////////////////////////////////////////////////////

ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_set_dst_id)
{
	const char *argv[] = { "set_dst_id", "1" };
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_set_dst_id(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_set_msg_dest_id_delegated_fake.call_count);
}
// ////////////////////////////////////////////////////////////////////////////
ZTEST(sid_dut_shell_api_shell_uninitialized, test_sid_option_lp_get_l3)
{
	const char *argv[] = {
		"-lp_get_l3",
	};
	int argc = sizeof(argv) / sizeof(argv[0]);

	int ret = cmd_sid_option_lp_get_l3(NULL, argc, argv);

	zassert_equal(-EINVAL, ret, "Returned error code %d", ret);
	zassert_equal(0, sid_option_delegated_fake.call_count);
}

static void *correct_initialize(void)
{
	app_ctx_valid = (struct app_context){ .sidewalk_handle = &sidewalk_handle };
	memset(&sid_cfg_valid, 0, sizeof(sid_cfg_valid));
	initialize_sidewalk_shell(&sid_cfg_valid, &app_ctx_valid);
	return NULL;
}

static void *NULL_initialize(void)
{
	memset(&app_ctx_invalid, 0, sizeof(app_ctx_invalid));
	memset(&sid_cfg_invalid, 0, sizeof(sid_cfg_invalid));
	initialize_sidewalk_shell(&sid_cfg_invalid, &app_ctx_invalid);
	return NULL;
}

void setup(void *fixture)
{
	ARG_UNUSED(fixture);
	// Register resets
	RESET_FAKE(sid_init_delegated);
	RESET_FAKE(sid_deinit_delegated);
	RESET_FAKE(sid_start_delegated);
	RESET_FAKE(sid_stop_delegated);
	RESET_FAKE(sid_put_msg_delegated);
	RESET_FAKE(sid_set_factory_reset_delegated);
	RESET_FAKE(sid_get_mtu_delegated);
	RESET_FAKE(sid_get_status_delegated);
	RESET_FAKE(sid_ble_bcn_connection_request_delegated);
	RESET_FAKE(sid_get_time_delegated);
	RESET_FAKE(sid_set_msg_dest_id_delegated);
	RESET_FAKE(sid_option_delegated);
	FFF_RESET_HISTORY();
}

ZTEST_SUITE(sid_dut_shell_api, NULL, correct_initialize, setup, NULL, NULL);
ZTEST_SUITE(sid_dut_shell_api_shell_uninitialized, NULL, NULL_initialize, setup, NULL, NULL);
