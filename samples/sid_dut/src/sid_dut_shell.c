/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <sys/errno.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include <sid_api.h>
#include <sid_900_cfg.h>

#include <sid_dut_shell.h>
#include <sid_thread.h>
#include <sid_api_delegated.h>
#include <sid_sdk_version.h>
#include <sidewalk_version.h>

#define CLI_CMD_OPT_LINK_BLE 1
#define CLI_CMD_OPT_LINK_FSK 2
#define CLI_CMD_OPT_LINK_LORA 3
#define CLI_CMD_OPT_LINK_LORA_BLE 4
#define CLI_CMD_OPT_LINK_FSK_BLE 5

#define CLI_MAX_DATA_LEN (CONFIG_SHELL_CMD_BUFF_SIZE / 2)
#define CLI_MAX_HEX_STR_LEN CONFIG_SHELL_CMD_BUFF_SIZE

#define CHECK_SHELL_INITIALIZED(shell, cli_cfg)                                                    \
	if (cli_cfg.app_cxt == NULL || cli_cfg.app_cxt->sidewalk_handle == NULL) {                 \
		shell_error(shell, "Sidewalk CLI not initialized");                                \
		return -EINVAL;                                                                    \
	}

#define CHECK_ARGUMENT_COUNT(argc, required, optional)                                             \
	if ((argc < required) || (argc > (required + optional))) {                                 \
		return -EINVAL;                                                                    \
	}

static uint8_t send_cmd_buf[CLI_MAX_DATA_LEN];

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_sid_option, SHELL_CMD_ARG(-b, NULL, "battery_level", cmd_sid_option_battery, 2, 0),
	SHELL_CMD_ARG(
		-lp_set, NULL,
		"profile(hex) <val2>\nprofiles: 0x80, 0x81, 0x83, 0x01,0x02\n<val2> - uint16_t value",
		cmd_sid_option_lp_set, 2, 1),
	SHELL_CMD_ARG(-lp_get_l2, NULL, "", cmd_sid_option_lp_get_l2, 1, 0),
	SHELL_CMD_ARG(-lp_get_l3, NULL, "", cmd_sid_option_lp_get_l3, 1, 0),
	SHELL_CMD_ARG(-d, NULL, "<0,1>", cmd_sid_option_d, 2, 0),
	SHELL_CMD_ARG(-gd, NULL, "", cmd_sid_option_gd, 1, 0), SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_services,
	SHELL_CMD_ARG(init, NULL, CMD_SID_INIT_DESCRIPTION, cmd_sid_init, CMD_SID_INIT_ARG_REQUIRED,
		      CMD_SID_INIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(deinit, NULL, CMD_SID_DEINIT_DESCRIPTION, cmd_sid_deinit,
		      CMD_SID_DEINIT_ARG_REQUIRED, CMD_SID_DEINIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(start, NULL, CMD_SID_START_DESCRIPTION, cmd_sid_start,
		      CMD_SID_START_ARG_REQUIRED, CMD_SID_START_ARG_OPTIONAL),
	SHELL_CMD_ARG(stop, NULL, CMD_SID_STOP_DESCRIPTION, cmd_sid_stop, CMD_SID_STOP_ARG_REQUIRED,
		      CMD_SID_STOP_ARG_OPTIONAL),
	SHELL_CMD_ARG(send, NULL, CMD_SID_SEND_DESCRIPTION, cmd_sid_send, CMD_SID_SEND_ARG_REQUIRED,
		      CMD_SID_SEND_ARG_OPTIONAL),
	SHELL_CMD_ARG(factory_reset, NULL, CMD_SID_FACTORY_RESET_DESCRIPTION, cmd_sid_factory_reset,
		      CMD_SID_FACTORY_RESET_ARG_REQUIRED, CMD_SID_FACTORY_RESET_ARG_OPTIONAL),
	SHELL_CMD_ARG(get_mtu, NULL, CMD_SID_GET_MTU_DESCRIPTION, cmd_sid_get_mtu,
		      CMD_SID_GET_MTU_ARG_REQUIRED, CMD_SID_GET_MTU_ARG_OPTIONAL),
	SHELL_CMD_ARG(option, &sub_sid_option, CMD_SID_SET_OPTION_DESCRIPTION, NULL,
		      CMD_SID_SET_OPTION_ARG_REQUIRED, 0),
	SHELL_CMD_ARG(last_status, NULL, CMD_SID_LAST_STATUS_DESCRIPTION, cmd_sid_last_status,
		      CMD_SID_LAST_STATUS_ARG_REQUIRED, CMD_SID_LAST_STATUS_ARG_OPTIONAL),
	SHELL_CMD_ARG(conn_req, NULL, CMD_SID_CONN_REQUEST_DESCRIPTION, cmd_sid_conn_request,
		      CMD_SID_CONN_REQUEST_ARG_REQUIRED, CMD_SID_CONN_REQUEST_ARG_OPTIONAL),
	SHELL_CMD_ARG(get_time, NULL, CMD_SID_GET_TIME_DESCRIPTION, cmd_sid_get_time,
		      CMD_SID_GET_TIME_ARG_REQUIRED, CMD_SID_GET_TIME_ARG_OPTIONAL),
	SHELL_CMD_ARG(set_dst_id, NULL, CMD_SID_SET_DST_ID_DESCRIPTION, cmd_sid_set_dst_id,
		      CMD_SID_SET_DST_ID_ARG_REQUIRED, CMD_SID_SET_DST_ID_ARG_OPTIONAL),
	SHELL_CMD_ARG(set_send_link, NULL, CMD_SID_SET_SEND_LINK_DESCRIPTION, cmd_sid_set_send_link,
		      CMD_SID_SET_SEND_LINK_ARG_REQUIRED, CMD_SID_SET_SEND_LINK_ARG_OPTIONAL),
	SHELL_CMD_ARG(set_rsp_id, NULL, CMD_SID_SET_RSP_ID_DESCRIPTION, cmd_sid_set_rsp_id,
		      CMD_SID_SET_RSP_ID_ARG_REQUIRED, CMD_SID_SET_RSP_ID_ARG_OPTIONAL),
	SHELL_CMD_ARG(sdk_version, NULL, CMD_SID_SDK_VERSION_DESCRIPTION, cmd_sid_sdk_version,
		      CMD_SID_SDK_VERSION_DESCRIPTION_ARG_REQUIRED,
		      CMD_SID_SDK_VERSION_DESCRIPTION_ARG_OPTIONAL),
	SHELL_CMD_ARG(sdk_config, NULL, CMD_SID_SDK_CONFIG_DESCRIPTION, cmd_sid_sdk_config,
		      CMD_SID_SDK_CONFIG_DESCRIPTION_ARG_REQUIRED,
		      CMD_SID_SDK_CONFIG_DESCRIPTION_ARG_OPTIONAL),
	SHELL_SUBCMD_SET_END);

// command, subcommands, help, handler
SHELL_CMD_REGISTER(sid, &sub_services, "sidewalk testing CLI", NULL);

static struct cli_config cli_cfg = {
	.send_link_type = SID_LINK_TYPE_ANY,
	.rsp_msg_id = 0,
};

void initialize_sidewalk_shell(struct sid_config *sid_cfg, struct app_context *app_cxt)
{
	cli_cfg.app_cxt = app_cxt;
	cli_cfg.sid_cfg = sid_cfg;
}

// utils

static bool cli_parse_link_mask_opt(uint8_t arg, uint32_t *link_mask)
{
	if (link_mask == NULL) {
		return false;
	}

	switch (arg) {
	case CLI_CMD_OPT_LINK_BLE:
		*link_mask = SID_LINK_TYPE_1;
		break;
	case CLI_CMD_OPT_LINK_FSK:
		*link_mask = SID_LINK_TYPE_2;
		break;
	case CLI_CMD_OPT_LINK_LORA:
		*link_mask = SID_LINK_TYPE_3;
		break;
	case CLI_CMD_OPT_LINK_LORA_BLE:
		*link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_3;
		break;
	case CLI_CMD_OPT_LINK_FSK_BLE:
		*link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_2;
		break;
	default:
		return false;
	}
	return true;
}

static int cli_parse_hexstr(const char *str_p, uint8_t *data_p, size_t buf_len)
{
	if (str_p == NULL) {
		return -1;
	}

	if (strlen(str_p) > CLI_MAX_HEX_STR_LEN) {
		return -2;
	}

	size_t str_len = strlen(str_p);

	if ((str_len % 2) != 0) {
		return -3;
	}

	size_t data_len = str_len / 2;

	if (buf_len < data_len) {
		return -4;
	}

	memset(data_p, 0, buf_len);

	size_t idx = 0;

	while (idx < str_len) {
		char c = str_p[idx];
		int value = 0;
		if (c >= '0' && c <= '9') {
			value = (c - '0');
		} else if (c >= 'A' && c <= 'F') {
			value = (10 + (c - 'A'));
		} else if (c >= 'a' && c <= 'f') {
			value = (10 + (c - 'a'));
		} else {
			return -5;
		}
		data_p[(idx / 2)] += value << (((idx + 1) % 2) * 4);
		idx++;
	}
	return data_len;
}

static int sid_option_get_window_separation_ms(uint32_t value,
					       enum sid_link2_rx_window_separation_ms *out)
{
	switch (value) {
	case SID_LINK2_RX_WINDOW_SEPARATION_1:
		*out = SID_LINK2_RX_WINDOW_SEPARATION_1;
		return 0;
	case SID_LINK2_RX_WINDOW_SEPARATION_2:
		*out = SID_LINK2_RX_WINDOW_SEPARATION_2;
		return 0;
	case SID_LINK2_RX_WINDOW_SEPARATION_3:
		*out = SID_LINK2_RX_WINDOW_SEPARATION_3;
		return 0;
	case SID_LINK2_RX_WINDOW_SEPARATION_4:
		*out = SID_LINK2_RX_WINDOW_SEPARATION_4;
		return 0;
	case SID_LINK2_RX_WINDOW_SEPARATION_5:
		*out = SID_LINK2_RX_WINDOW_SEPARATION_5;
		return 0;
	case SID_LINK2_RX_WINDOW_SEPARATION_6:
		*out = SID_LINK2_RX_WINDOW_SEPARATION_6;
		return 0;
	case SID_LINK2_RX_WINDOW_SEPARATION_7:
		*out = SID_LINK2_RX_WINDOW_SEPARATION_7;
		return 0;
	default:
		return -EINVAL;
	}
}

static int cmd_sid_option_handle_set_link3_profile(const char *value,
						   struct sid_device_profile *out_profile)
{
	long rx_window_count_raw = 0l;
	char *end = NULL;

	rx_window_count_raw = strtol(value, &end, 0);
	if (end == value) {
		return -EINVAL;
	}
	if (!IN_RANGE(rx_window_count_raw, 0, UINT16_MAX)) {
		return -EINVAL;
	}
	out_profile->unicast_params.rx_window_count = (enum sid_rx_window_count)rx_window_count_raw;
	out_profile->unicast_params.unicast_window_interval.async_rx_interval_ms =
		SID_LINK3_RX_WINDOW_SEPARATION_3;
	return 0;
}

static void sid_option_get(const struct shell *shell, enum sid_device_profile_id profile)
{
	static struct sid_device_profile dev_cfg;

	dev_cfg = (struct sid_device_profile){ .unicast_params = { .device_profile_id = profile } };
	sid_error_t ret = sid_option_delegated(*cli_cfg.app_cxt->sidewalk_handle,
					       SID_OPTION_900MHZ_GET_DEVICE_PROFILE, &dev_cfg,
					       sizeof(dev_cfg));

	if (IS_LINK2_PROFILE_ID(dev_cfg.unicast_params.device_profile_id) ||
	    IS_LINK3_PROFILE_ID(dev_cfg.unicast_params.device_profile_id)) {
		char rx_int_output[32] = {};
		if (dev_cfg.unicast_params.device_profile_id == SID_LINK2_PROFILE_2) {
			snprintf(
				rx_int_output, sizeof(rx_int_output), " Rx_Int = %d",
				dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms);
		}
		shell_info(shell, "sid_option returned %d; Link_profile ID: %d Wndw_cnt: %d%s", ret,
			   dev_cfg.unicast_params.device_profile_id,
			   dev_cfg.unicast_params.rx_window_count, rx_int_output);
	}
}

// shell handlers

int cmd_sid_init(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_INIT_ARG_REQUIRED, CMD_SID_INIT_ARG_OPTIONAL);

	const char *connection_type_arg = argv[1];
	uint8_t connection_type = atoi(connection_type_arg);

	if (!IN_RANGE(connection_type, 1, 5)) {
		return -EINVAL;
	}
	if (!cli_parse_link_mask_opt(connection_type, &cli_cfg.sid_cfg->link_mask)) {
		return -EINVAL;
	}

	sid_error_t ret = sid_init_delegated(cli_cfg.sid_cfg, cli_cfg.app_cxt->sidewalk_handle);

	shell_info(shell, "sid_init returned %d", ret);
	return 0;
}

int cmd_sid_deinit(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_DEINIT_ARG_REQUIRED, CMD_SID_DEINIT_ARG_OPTIONAL);

	sid_error_t ret = sid_deinit_delegated(*cli_cfg.app_cxt->sidewalk_handle);

	shell_info(shell, "sid_deinit returned %d", ret);
	return 0;
}

int cmd_sid_start(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_START_ARG_REQUIRED, CMD_SID_START_ARG_OPTIONAL);
	uint32_t link_mask = 0;

	if (argc == 1) {
		link_mask = cli_cfg.sid_cfg->link_mask;
	} else {
		if (!cli_parse_link_mask_opt(atoi(argv[1]), &link_mask)) {
			return -EINVAL;
		}
	}

	sid_error_t ret = sid_start_delegated(*cli_cfg.app_cxt->sidewalk_handle, link_mask);

	shell_info(shell, "sid_start returned %d", ret);
	return 0;
}

int cmd_sid_stop(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_STOP_ARG_REQUIRED, CMD_SID_STOP_ARG_OPTIONAL);
	uint32_t link_mask = 0;

	if (argc == 1) {
		link_mask = cli_cfg.sid_cfg->link_mask;
	} else {
		if (!cli_parse_link_mask_opt(atoi(argv[1]), &link_mask)) {
			return -EINVAL;
		}
	}

	sid_error_t ret = sid_stop_delegated(*cli_cfg.app_cxt->sidewalk_handle, link_mask);

	shell_info(shell, "sid_stop returned %d", ret);
	return 0;
}

int cmd_sid_send(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SEND_ARG_REQUIRED, CMD_SID_SEND_ARG_OPTIONAL);

	static struct sid_msg msg;
	static struct sid_msg_desc desc;

	msg = (struct sid_msg){ .size = strlen(argv[argc - 1]), .data = (void *)argv[argc - 1] };
	desc = (struct sid_msg_desc){
		.type = SID_MSG_TYPE_NOTIFY,
		.link_type = cli_cfg.send_link_type,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

	for (int opt = 1; opt < argc; opt++) {
		if (strcmp("-t", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-t need a value");
				return -EINVAL;
			}
			switch (argv[opt][0]) {
			case '0':
				desc.type = SID_MSG_TYPE_GET;
				break;
			case '1':
				desc.type = SID_MSG_TYPE_SET;
				break;
			case '2':
				desc.type = SID_MSG_TYPE_NOTIFY;
				break;
			case '3':
				desc.type = SID_MSG_TYPE_RESPONSE;
				break;
			default: {
				shell_error(shell, "invalid type");
				return -EINVAL;
			}
			}
			continue;
		}
		if (strcmp("-d", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-d need a value");
				return -EINVAL;
			}
			switch (argv[opt][0]) {
			case '1':
				desc.link_mode = SID_LINK_MODE_CLOUD;
				break;
			case '2':
				desc.link_mode = SID_LINK_MODE_MOBILE;
				break;
			default: {
				shell_error(shell, "invalid mode");
				return -EINVAL;
			}
			}
			continue;
		}
		if (strcmp("-r", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-r need a value");
				return -EINVAL;
			}
			int res = cli_parse_hexstr(argv[opt], send_cmd_buf, CLI_MAX_DATA_LEN);
			if (res < 0) {
				shell_error(shell, "failed to parse value as hexstring");
				return -EINVAL;
			}
			msg.size = (size_t)res;
			msg.data = send_cmd_buf;
			continue;
		}
		if (strcmp("-l", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-l need a value");
				return -EINVAL;
			}

			if (!cli_parse_link_mask_opt(atoi(argv[opt]), &desc.link_type)) {
				return -EINVAL;
			}
			continue;
		}
		if (strcmp("-i", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-i need a value");
				return -EINVAL;
			}
			char *end = NULL;
			long data_raw = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(data_raw, 0, UINT16_MAX) || end == argv[opt]) {
				shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
					    argv[opt], (unsigned int)UINT16_MAX);
				return -EINVAL;
			}
			cli_cfg.rsp_msg_id = (uint16_t)data_raw;
			continue;
		}
		if (strcmp("-a", argv[opt]) == 0) {
			if (opt + 3 > argc) {
				shell_error(shell, "-a need 3 positional arguments");
				return -EINVAL;
			}

			const char *ack = argv[++opt];
			const char *retry = argv[++opt];
			const char *ttl = argv[++opt];
			char *end = NULL;
			long ack_val = strtol(ack, &end, 0);
			if (!IN_RANGE(ack_val, 0, 1) || end == ack) {
				shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
					    ack, 1);
				return -EINVAL;
			}
			desc.msg_desc_attr.tx_attr.request_ack = (bool)ack_val;

			long retry_val = strtol(retry, &end, 0);
			if (!IN_RANGE(ack_val, 0, UINT8_MAX) || end == ack) {
				shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
					    retry, (unsigned int)UINT8_MAX);
				return -EINVAL;
			}
			desc.msg_desc_attr.tx_attr.num_retries = (uint8_t)retry_val;

			long ttl_val = strtol(ttl, &end, 0);
			if (!IN_RANGE(ttl_val, 0, UINT16_MAX) || end == ack) {
				shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
					    ttl, (unsigned int)UINT16_MAX);
				return -EINVAL;
			}
			desc.msg_desc_attr.tx_attr.ttl_in_seconds = (uint16_t)ttl_val;
		}
		/* The last condition does not need 'continue' statement. */
	}

	if (desc.type == SID_MSG_TYPE_RESPONSE) {
		desc.id = cli_cfg.rsp_msg_id;
	}

	sid_error_t ret = sid_put_msg_delegated(*cli_cfg.app_cxt->sidewalk_handle, &msg, &desc);

	shell_info(shell, "sid_put_msg returned %d, TYPE: %d ID: %d", ret, desc.type, desc.id);
	return 0;
}

int cmd_sid_factory_reset(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_FACTORY_RESET_ARG_REQUIRED,
			     CMD_SID_FACTORY_RESET_ARG_OPTIONAL);

	sid_error_t ret = sid_set_factory_reset_delegated(*cli_cfg.app_cxt->sidewalk_handle);

	shell_info(shell, "sid_set_factory_reset returned %d", ret);
	return 0;
}

int cmd_sid_get_mtu(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_GET_MTU_ARG_REQUIRED, CMD_SID_GET_MTU_ARG_OPTIONAL);

	enum sid_link_type link_type = SID_LINK_TYPE_1;

	switch (argv[1][0]) {
	case '1': {
		link_type = SID_LINK_TYPE_1;
		break;
	}
	case '2': {
		link_type = SID_LINK_TYPE_2;
		break;
	}
	case '3': {
		link_type = SID_LINK_TYPE_3;
		break;
	}
	default: {
		return -EINVAL;
	}
	}

	size_t mtu = 0;
	sid_error_t ret = sid_get_mtu_delegated(*cli_cfg.app_cxt->sidewalk_handle, link_type, &mtu);

	shell_info(shell, "sid_get_mtu returned %d, MTU: %d", ret, mtu);
	return 0;
}

int cmd_sid_option_battery(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, 2, 0);

	long data_raw = 0l;
	static uint8_t data = 0;
	char *end = NULL;

	data_raw = strtol(argv[1], &end, 0);
	if (end == argv[1]) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>", argv[1],
			    (unsigned int)UINT8_MAX);
		return -EINVAL;
	}
	if (!IN_RANGE(data_raw, 0, UINT8_MAX)) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>", argv[1],
			    (unsigned int)UINT8_MAX);
		return -EINVAL;
	}
	data = (uint8_t)data_raw;
	sid_error_t ret = sid_option_delegated(*cli_cfg.app_cxt->sidewalk_handle,
					       SID_OPTION_BLE_BATTERY_LEVEL, &data, sizeof(data));

	shell_info(shell, "sid_option returned %d", ret);
	return 0;
}

int cmd_sid_option_lp_set(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, 2, 1);

	long data_raw = 0l;
	static enum sid_device_profile_id dev_profile = SID_LINK3_PROFILE_LAST;
	char *end = NULL;

	data_raw = strtol(argv[1], &end, 0);
	if (end == argv[1]) {
		return -EINVAL;
	}
	if (!IN_RANGE(data_raw, 0, UINT8_MAX)) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>", argv[1],
			    (unsigned int)UINT8_MAX);
		return -EINVAL;
	}

	dev_profile = (enum sid_device_profile_id)data_raw;
	static struct sid_device_profile dev_cfg;

	memset(&dev_cfg, 0, sizeof(dev_cfg));
	dev_cfg = (struct sid_device_profile){ .unicast_params = { .device_profile_id = dev_profile,
								   .wakeup_type =
									   SID_TX_AND_RX_WAKEUP } };

	switch (dev_profile) {
	case SID_LINK3_PROFILE_A:
	case SID_LINK3_PROFILE_B:
	case SID_LINK3_PROFILE_D: {
		CHECK_ARGUMENT_COUNT(argc, 3, 0);
		if (cmd_sid_option_handle_set_link3_profile(argv[2], &dev_cfg) != 0) {
			shell_error(shell, "Invalid argument [%s], must be value <0, %d>", argv[2],
				    (unsigned int)UINT16_MAX);
			return -EINVAL;
		}
		break;
	}
	case SID_LINK2_PROFILE_1: {
		CHECK_ARGUMENT_COUNT(argc, 2, 0);
		dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
		break;
	}
	case SID_LINK2_PROFILE_2: {
		long window_separation_ms_raw = 0;
		dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms =
			SID_LINK2_RX_WINDOW_SEPARATION_1;
		if (argc == 3) {
			char *end = NULL;
			window_separation_ms_raw = strtol(argv[2], &end, 0);
			if (sid_option_get_window_separation_ms(
				    window_separation_ms_raw,
				    &dev_cfg.unicast_params.unicast_window_interval
					     .sync_rx_interval_ms) != 0) {
				shell_error(
					shell,
					"Invalid separation window value: [%s]\n valid values are [%d, %d, %d, %d, %d, %d, %d]",
					argv[2], SID_LINK2_RX_WINDOW_SEPARATION_1,
					SID_LINK2_RX_WINDOW_SEPARATION_2,
					SID_LINK2_RX_WINDOW_SEPARATION_3,
					SID_LINK2_RX_WINDOW_SEPARATION_4,
					SID_LINK2_RX_WINDOW_SEPARATION_5,
					SID_LINK2_RX_WINDOW_SEPARATION_6,
					SID_LINK2_RX_WINDOW_SEPARATION_7);
				return -EINVAL;
			}
		}
		dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
		break;
	}
	default: {
		shell_error(
			shell,
			"Invalid argument: [%s]\n valid values for profile are: [0x80, 0x81, 0x83, 0x01, 0x02]",
			argv[1]);
		return -EINVAL;
	}
	}

	sid_error_t ret = sid_option_delegated(*cli_cfg.app_cxt->sidewalk_handle,
					       SID_OPTION_900MHZ_SET_DEVICE_PROFILE, &dev_cfg,
					       sizeof(dev_cfg));

	shell_info(shell, "sid_option returned %d", ret);
	return 0;
}

int cmd_sid_option_lp_get_l2(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, 1, 0);
	sid_option_get(shell, SID_LINK2_PROFILE_1);
	return 0;
}

int cmd_sid_option_lp_get_l3(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, 1, 0);
	sid_option_get(shell, SID_LINK3_PROFILE_A);
	return 0;
}

int cmd_sid_option_d(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, 2, 0);

	long data_raw = 0l;
	static uint8_t data = 0;
	char *end = NULL;

	data_raw = strtol(argv[1], &end, 0);
	if (end == argv[1] || !IN_RANGE(data_raw, 0, 1)) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
			    argv[1] == NULL ? "NULL" : argv[1], (unsigned int)1);
		return -EINVAL;
	}
	data = (uint8_t)data_raw;
	sid_error_t ret = sid_option_delegated(*cli_cfg.app_cxt->sidewalk_handle,
					       SID_OPTION_SET_MSG_POLICY_FILTER_DUPLICATES, &data,
					       sizeof(data));

	shell_info(shell, "sid_option returned %d", ret);
	return 0;
}

int cmd_sid_option_gd(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	static uint8_t data = 0;

	sid_error_t ret = sid_option_delegated(*cli_cfg.app_cxt->sidewalk_handle,
					       SID_OPTION_GET_MSG_POLICY_FILTER_DUPLICATES, &data,
					       sizeof(data));

	shell_info(shell, "sid_option returned %d; Filter Duplicates: %d", ret, data);
	return 0;
}

int cmd_sid_last_status(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_LAST_STATUS_ARG_REQUIRED,
			     CMD_SID_LAST_STATUS_ARG_OPTIONAL);
	static struct sid_status status = {};

	memset(&status, 0, sizeof(status));
	sid_error_t ret = sid_get_status_delegated(*cli_cfg.app_cxt->sidewalk_handle, &status);

	if (ret == SID_ERROR_NONE) {
		shell_info(shell, "EVENT SID STATUS: State: %d, Reg: %d, Time: %d, Link_Mask: %x",
			   status.state, status.detail.registration_status,
			   status.detail.time_sync_status, status.detail.link_status_mask);
		shell_info(shell, "EVENT SID STATUS LINK MODE: LORA: %x, FSK: %x, BLE: %x",
			   status.detail.supported_link_modes[2],
			   status.detail.supported_link_modes[1],
			   status.detail.supported_link_modes[0]);
	}

	shell_info(shell, "sid_get_status returned %d", ret);
	return 0;
}

int cmd_sid_conn_request(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_CONN_REQUEST_ARG_REQUIRED,
			     CMD_SID_CONN_REQUEST_ARG_OPTIONAL);

	bool conn_req = false;

	switch (argv[1][0]) {
	case '1': {
		conn_req = true;
		break;
	}
	case '0': {
		conn_req = false;
		break;
	}
	default: {
		return -EINVAL;
	}
	}

	sid_error_t ret = sid_ble_bcn_connection_request_delegated(
		*cli_cfg.app_cxt->sidewalk_handle, conn_req);

	shell_info(shell, "sid_ble_bcn_connection_request returned %d", ret);
	return 0;
}

int cmd_sid_get_time(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_GET_TIME_ARG_REQUIRED, CMD_SID_GET_TIME_ARG_OPTIONAL);
	enum sid_time_format type = SID_GET_GPS_TIME;
	struct sid_timespec curr_time;

	switch (argv[1][0]) {
	case '0': {
		type = SID_GET_GPS_TIME;
		break;
	}
	case '1': {
		type = SID_GET_UTC_TIME;
		break;
	}
	case '2': {
		type = SID_GET_LOCAL_TIME;
		break;
	}
	default: {
		return -EINVAL;
	}
	}

	sid_error_t ret =
		sid_get_time_delegated(*cli_cfg.app_cxt->sidewalk_handle, type, &curr_time);

	shell_info(shell, "sid_get_time returned %d, SEC: %d NSEC: %d", ret, curr_time.tv_sec,
		   curr_time.tv_nsec);
	return 0;
}

int cmd_sid_set_dst_id(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_DST_ID_ARG_REQUIRED,
			     CMD_SID_SET_DST_ID_ARG_OPTIONAL);
	uint32_t dst_id = atoi(argv[1]);
	sid_error_t ret = sid_set_msg_dest_id_delegated(*cli_cfg.app_cxt->sidewalk_handle, dst_id);

	shell_info(shell, "sid_set_msg_dest_id returned %d", ret);
	return 0;
}

int cmd_sid_set_send_link(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_SEND_LINK_ARG_REQUIRED,
			     CMD_SID_SET_SEND_LINK_ARG_OPTIONAL);

	uint32_t opt = atoi(argv[1]);

	switch (opt) {
	case 0: {
		cli_cfg.send_link_type = SID_LINK_TYPE_ANY;
		break;
	}
	case 1: {
		cli_cfg.send_link_type = SID_LINK_TYPE_1;
		break;
	}
	case 2: {
		cli_cfg.send_link_type = SID_LINK_TYPE_2;
		break;
	}
	case 3: {
		cli_cfg.send_link_type = SID_LINK_TYPE_3;
		break;
	}
	default: {
		return -EINVAL;
	}
	}

	return 0;
}

int cmd_sid_set_rsp_id(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_RSP_ID_ARG_REQUIRED,
			     CMD_SID_SET_RSP_ID_ARG_OPTIONAL);

	char *end;
	long data_raw = strtol(argv[1], &end, 0);

	if (end == argv[1] || !IN_RANGE(data_raw, 0, UINT16_MAX)) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>", argv[1],
			    (unsigned int)UINT16_MAX);
		return -EINVAL;
	}
	cli_cfg.rsp_msg_id = (uint16_t)data_raw;
	return 0;
}

int cmd_sid_sdk_version(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SDK_VERSION_DESCRIPTION_ARG_REQUIRED,
			     CMD_SID_SDK_VERSION_DESCRIPTION_ARG_OPTIONAL);

#define SID_SDK_VERSION_STR                                                                        \
	STRINGIFY(SID_SDK_MAJOR_VERSION)                                                           \
	"." STRINGIFY(SID_SDK_MINOR_VERSION) "." STRINGIFY(SID_SDK_PATCH_VERSION) "-" STRINGIFY(   \
		SID_SDK_BUILD_VERSION)
	shell_info(shell, "SID_SDK_VERSION: " SID_SDK_VERSION_STR);

	shell_info(shell, "SID_SDK_MAJOR_VERSION: %d", SID_SDK_MAJOR_VERSION);
	shell_info(shell, "SID_SDK_MINOR_VERSION: %d", SID_SDK_MINOR_VERSION);
	shell_info(shell, "SID_SDK_PATCH_VERSION: %d", SID_SDK_PATCH_VERSION);
	shell_info(shell, "SID_SDK_BUILD_VERSION: %d", SID_SDK_BUILD_VERSION);

	shell_info(shell, "Nordic version: %s", sidewalk_version_component[0]);
	return 0;
}

int cmd_sid_sdk_config(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SDK_CONFIG_DESCRIPTION_ARG_REQUIRED,
			     CMD_SID_SDK_CONFIG_DESCRIPTION_ARG_OPTIONAL);
	shell_info(shell, "SID_SDK_CONFIG_ENABLE_LINK_TYPE_1: %d", true);
	shell_info(shell, "SID_SDK_CONFIG_ENABLE_LINK_TYPE_2: %d",
		   IS_ENABLED(CONFIG_SIDEWALK_LINK_MASK_FSK) ||
			   IS_ENABLED(CONFIG_SIDEWALK_LINK_MASK_LORA));
	shell_info(shell, "SID_SDK_CONFIG_ENABLE_LINK_TYPE_3: %d",
		   IS_ENABLED(CONFIG_SIDEWALK_LINK_MASK_FSK) ||
			   IS_ENABLED(CONFIG_SIDEWALK_LINK_MASK_LORA));
	return 0;
}
