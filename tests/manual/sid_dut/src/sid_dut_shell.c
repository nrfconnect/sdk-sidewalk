/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
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

#define CLI_CMD_OPT_LINK_BLE        1
#define CLI_CMD_OPT_LINK_FSK        2
#define CLI_CMD_OPT_LINK_LORA       3
#define CLI_CMD_OPT_LINK_LORA_BLE   4

#define CLI_MAX_DATA_LEN            (CONFIG_SHELL_CMD_BUFF_SIZE / 2)
#define CLI_MAX_HEX_STR_LEN         CONFIG_SHELL_CMD_BUFF_SIZE

#define CHECK_SHELL_INITIALIZED(shell, cli_cfg)					   \
	if (cli_cfg.app_cxt == NULL || cli_cfg.app_cxt->sidewalk_handle == NULL) { \
		shell_error(shell, "Sidewalk CLI not initialized");		   \
		return -EINVAL;							   \
	}									   \

#define CHECK_ARGUMENT_COUNT(argc, required, \
			     optional) if ((argc < required) || (argc > (required + optional))) { return -EINVAL; }

static uint8_t send_cmd_buf[CLI_MAX_DATA_LEN];

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_services,
	SHELL_CMD_ARG(init, NULL, CMD_SID_INIT_DESCRIPTION, cmd_sid_init, CMD_SID_INIT_ARG_REQUIRED,
		      CMD_SID_INIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(de_init, NULL, CMD_SID_DEINIT_DESCRIPTION, cmd_sid_deinit, CMD_SID_DEINIT_ARG_REQUIRED,
		      CMD_SID_DEINIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(start, NULL, CMD_SID_START_DESCRIPTION, cmd_sid_start, CMD_SID_START_ARG_REQUIRED,
		      CMD_SID_START_ARG_OPTIONAL),
	SHELL_CMD_ARG(stop, NULL, CMD_SID_STOP_DESCRIPTION, cmd_sid_stop, CMD_SID_STOP_ARG_REQUIRED,
		      CMD_SID_STOP_ARG_OPTIONAL),
	SHELL_CMD_ARG(send, NULL, CMD_SID_SEND_DESCRIPTION, cmd_sid_send, CMD_SID_SEND_ARG_REQUIRED,
		      CMD_SID_SEND_ARG_OPTIONAL),
	SHELL_CMD_ARG(factory_reset, NULL, CMD_SID_FACTORY_RESET_DESCRIPTION, cmd_sid_factory_reset,
		      CMD_SID_FACTORY_RESET_ARG_REQUIRED, CMD_SID_FACTORY_RESET_ARG_OPTIONAL),
	SHELL_CMD_ARG(get_mtu, NULL, CMD_SID_GET_MTU_DESCRIPTION, cmd_sid_get_mtu, CMD_SID_GET_MTU_ARG_REQUIRED,
		      CMD_SID_GET_MTU_ARG_OPTIONAL),
	SHELL_CMD_ARG(option, NULL, CMD_SID_SET_OPTION_DESCRIPTION, cmd_sid_set_option, CMD_SID_SET_OPTION_ARG_REQUIRED,
		      CMD_SID_SET_OPTION_ARG_OPTIONAL),
	SHELL_CMD_ARG(last_status, NULL, CMD_SID_LAST_STATUS_DESCRIPTION, cmd_sid_last_status,
		      CMD_SID_LAST_STATUS_ARG_REQUIRED, CMD_SID_LAST_STATUS_ARG_OPTIONAL),
	SHELL_CMD_ARG(conn_req, NULL, CMD_SID_CONN_REQUEST_DESCRIPTION, cmd_sid_conn_request,
		      CMD_SID_CONN_REQUEST_ARG_REQUIRED, CMD_SID_CONN_REQUEST_ARG_OPTIONAL),
	SHELL_CMD_ARG(get_time, NULL, CMD_SID_GET_TIME_DESCRIPTION, cmd_sid_get_time, CMD_SID_GET_TIME_ARG_REQUIRED,
		      CMD_SID_GET_TIME_ARG_OPTIONAL),
	SHELL_CMD_ARG(set_dst_id, NULL, CMD_SID_SET_DST_ID_DESCRIPTION, cmd_sid_set_dst_id,
		      CMD_SID_SET_DST_ID_ARG_REQUIRED, CMD_SID_SET_DST_ID_ARG_OPTIONAL),
	SHELL_CMD_ARG(set_send_link, NULL, CMD_SID_SET_SEND_LINK_DESCRIPTION, cmd_sid_set_send_link,
		      CMD_SID_SET_SEND_LINK_ARG_REQUIRED, CMD_SID_SET_SEND_LINK_ARG_OPTIONAL),
	SHELL_CMD_ARG(set_rsp_id, NULL, CMD_SID_SET_RSP_ID_DESCRIPTION, cmd_sid_set_rsp_id,
		      CMD_SID_SET_RSP_ID_ARG_REQUIRED, CMD_SID_SET_RSP_ID_ARG_OPTIONAL),
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

// shell handlers

int cmd_sid_init(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_INIT_ARG_REQUIRED, CMD_SID_INIT_ARG_OPTIONAL);

	const char *connection_type_arg = argv[1];
	uint8_t connection_type = atoi(connection_type_arg);

	if (!IN_RANGE(connection_type, 1, 4)) {
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

	switch (argc) {
	case 1: link_mask = cli_cfg.sid_cfg->link_mask;
		break;
	case 2:
		if (!cli_parse_link_mask_opt(atoi(argv[1]), &link_mask)) {
			return -EINVAL;
		}
		break;
	default: return -EINVAL;
	}
	sid_error_t ret = sid_start_delegated(*cli_cfg.app_cxt->sidewalk_handle,
					      link_mask);

	shell_info(shell, "sid_start returned %d", ret);
	return 0;
}

int cmd_sid_stop(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_STOP_ARG_REQUIRED, CMD_SID_STOP_ARG_OPTIONAL);
	uint32_t link_mask = 0;

	switch (argc) {
	case 1: link_mask = cli_cfg.sid_cfg->link_mask;
		break;
	case 2:
		if (!cli_parse_link_mask_opt(atoi(argv[1]), &link_mask)) {
			return -EINVAL;
		}
		break;
	default: return -EINVAL;
	}

	sid_error_t ret = sid_stop_delegated(*cli_cfg.app_cxt->sidewalk_handle,
					     link_mask);

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
			case '0': desc.type = SID_MSG_TYPE_GET; break;
			case '1': desc.type = SID_MSG_TYPE_SET; break;
			case '2': desc.type = SID_MSG_TYPE_NOTIFY; break;
			case '3': desc.type = SID_MSG_TYPE_RESPONSE; break;
			default: { shell_error(shell, "invalid type"); return -EINVAL; }
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
			case '1': desc.link_mode = SID_LINK_MODE_CLOUD; break;
			case '2': desc.link_mode = SID_LINK_MODE_MOBILE; break;
			default: { shell_error(shell, "invalid mode"); return -EINVAL; }
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
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_FACTORY_RESET_ARG_REQUIRED, CMD_SID_FACTORY_RESET_ARG_OPTIONAL);

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

int cmd_sid_set_option(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_OPTION_ARG_REQUIRED, CMD_SID_SET_OPTION_ARG_OPTIONAL);
	enum sid_option opt = SID_OPTION_BLE_BATTERY_LEVEL;
	sid_error_t ret = SID_ERROR_NONE;
	static uint8_t arg1;

	arg1 = 0;

	if (argc == 2) {
		if (strcmp(argv[1], "-lp_get_l3") == 0) {
			opt = SID_OPTION_900MHZ_GET_DEVICE_PROFILE;
			arg1 = SID_LINK3_PROFILE_A;
		} else if (strcmp(argv[1], "-lp_get_l2") == 0) {
			opt = SID_OPTION_900MHZ_GET_DEVICE_PROFILE;
			arg1 = SID_LINK2_PROFILE_1;
		} else {
			return -EINVAL;
		}
	} else {
		int arg_val_raw = atoi(argv[2]);
		if (arg_val_raw > UINT8_MAX) {
			return -EINVAL;
		}
		arg1 = (uint8_t)arg_val_raw;
		if (strcmp(argv[1], "-lp_set") == 0) {
			opt = SID_OPTION_900MHZ_SET_DEVICE_PROFILE;
		} else if (strcmp(argv[1], "-b") == 0) {
			opt = SID_OPTION_BLE_BATTERY_LEVEL;
		} else {
			return -EINVAL;
		}
	}

	switch (opt) {
	case SID_OPTION_BLE_BATTERY_LEVEL: {
		ret = sid_option_delegated(*cli_cfg.app_cxt->sidewalk_handle, SID_OPTION_BLE_BATTERY_LEVEL, &arg1,
					   sizeof(arg1));
		shell_info(shell, "sid_option returned %d", ret);
	}
	break;
	case SID_OPTION_900MHZ_SET_DEVICE_PROFILE: {
		if (argc > 4) {
			return -EINVAL;
		}

		static struct sid_device_profile dev_cfg;
		dev_cfg = (struct sid_device_profile){ .unicast_params = { .device_profile_id = arg1 } };
		if (IS_LINK3_PROFILE_ID(arg1) && argc == 4) {
			dev_cfg.unicast_params.rx_window_count = atoi(argv[3]);
			dev_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms =
				SID_LINK3_RX_WINDOW_SEPARATION_3;
		} else if (arg1 == SID_LINK2_PROFILE_1 && argc == 3) {
			dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
		} else if (arg1 == SID_LINK2_PROFILE_2 && argc <= 4) {
			dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
			dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms =
				(argc < 4) ? SID_LINK2_RX_WINDOW_SEPARATION_1 :
				(enum
				 sid_link2_rx_window_separation_ms)atoi(argv[3]);
		} else {
			return -EINVAL;
		}
		ret = sid_option_delegated(*cli_cfg.app_cxt->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
					   &dev_cfg,
					   sizeof(dev_cfg));
		shell_info(shell, "sid_option returned %d", ret);

	}
	break;
	case SID_OPTION_900MHZ_GET_DEVICE_PROFILE: {
		static struct sid_device_profile dev_cfg;
		dev_cfg = (struct sid_device_profile){ .unicast_params = { .device_profile_id = arg1 } };
		ret = sid_option_delegated(*cli_cfg.app_cxt->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
					   &dev_cfg,
					   sizeof(dev_cfg));
		if (IS_LINK2_PROFILE_ID(dev_cfg.unicast_params.device_profile_id)
		    || IS_LINK3_PROFILE_ID(dev_cfg.unicast_params.device_profile_id)) {
			char rx_int_output[32] = {};
			if (dev_cfg.unicast_params.device_profile_id == SID_LINK2_PROFILE_2) {
				snprintf(rx_int_output, sizeof(rx_int_output), " Rx_Int = %d",
					 dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms);
			}
			shell_info(shell, "sid_option returned %d; Link_profile ID: %d Wndw_cnt: %d%s", ret,
				   dev_cfg.unicast_params.device_profile_id,
				   dev_cfg.unicast_params.rx_window_count, rx_int_output);
		}
	}
	break;
	default:
		return -EINVAL;
	}

	return 0;

}

int cmd_sid_last_status(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_LAST_STATUS_ARG_REQUIRED, CMD_SID_LAST_STATUS_ARG_OPTIONAL);
	static struct sid_status status = {};

	memset(&status, 0, sizeof(status));
	sid_error_t ret = sid_get_status_delegated(*cli_cfg.app_cxt->sidewalk_handle, &status);

	if (ret == SID_ERROR_NONE) {

		shell_info(shell, "EVENT SID STATUS: State: %d, Reg: %d, Time: %d, Link_Mask: %x",
			   status.state,
			   status.detail.registration_status,
			   status.detail.time_sync_status,
			   status.detail.link_status_mask);
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
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_CONN_REQUEST_ARG_REQUIRED, CMD_SID_CONN_REQUEST_ARG_OPTIONAL);

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

	sid_error_t ret = sid_ble_bcn_connection_request_delegated(*cli_cfg.app_cxt->sidewalk_handle, conn_req);

	shell_info(shell, "sid_ble_bcn_connection_request returned %d", ret);
	return 0;
}

int cmd_sid_get_time(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_GET_TIME_ARG_REQUIRED, CMD_SID_GET_TIME_ARG_OPTIONAL);
	enum sid_time_format type = 0;
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

	sid_error_t ret = sid_get_time_delegated(*cli_cfg.app_cxt->sidewalk_handle, type, &curr_time);

	shell_info(shell, "sid_get_time returned %d, SEC: %d NSEC: %d", ret, curr_time.tv_sec, curr_time.tv_nsec);
	return 0;
}

int cmd_sid_set_dst_id(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_DST_ID_ARG_REQUIRED, CMD_SID_SET_DST_ID_ARG_OPTIONAL);
	uint32_t dst_id = atoi(argv[1]);
	sid_error_t ret = sid_set_msg_dest_id_delegated(*cli_cfg.app_cxt->sidewalk_handle, dst_id);

	shell_info(shell, "sid_set_msg_dest_id returned %d", ret);
	return 0;
}

int cmd_sid_set_send_link(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_SHELL_INITIALIZED(shell, cli_cfg);
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_SEND_LINK_ARG_REQUIRED, CMD_SID_SET_SEND_LINK_ARG_OPTIONAL);

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
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_RSP_ID_ARG_REQUIRED, CMD_SID_SET_RSP_ID_ARG_OPTIONAL);
	cli_cfg.rsp_msg_id = atoi(argv[1]);
	return 0;
}
