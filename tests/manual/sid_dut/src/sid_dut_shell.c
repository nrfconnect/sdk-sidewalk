/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>

#include "zephyr/sys/util.h"
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

#include <sid_api.h>

#include <sid_dut_shell.h>
#include <sid_api_delegated.h>

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_services,
	SHELL_CMD_ARG(init, NULL, CMD_SID_INIT_DESCRIPTION, cmd_sid_init, 2, 0),
	SHELL_CMD_ARG(de_init, NULL, CMD_SID_DEINIT_DESCRIPTION, cmd_sid_deinit, 1, 0),
	SHELL_CMD_ARG(start, NULL, CMD_SID_START_DESCRIPTION, cmd_sid_start, 1, 1),
	SHELL_CMD_ARG(stop, NULL, CMD_SID_STOP_DESCRIPTION, cmd_sid_stop, 1, 1),
	SHELL_CMD_ARG(send, NULL, CMD_SID_SEND_DESCRIPTION, cmd_sid_send, 2, 5),
	SHELL_CMD_ARG(factory_reset, NULL, CMD_SID_FACTORY_RESET_DESCRIPTION, cmd_sid_factory_reset, 1, 0),
	SHELL_CMD_ARG(get_mtu, NULL, CMD_SID_GET_MTU_DESCRIPTION, cmd_sid_get_mtu, 2, 0),
	SHELL_CMD_ARG(option, NULL, CMD_SID_SET_OPTION_DESCRIPTION, cmd_sid_set_option, 1, 10),
	SHELL_CMD_ARG(last_status, NULL, CMD_SID_LAST_STATUS_DESCRIPTION, cmd_sid_last_status, 1, 0),
	SHELL_CMD_ARG(conn_req, NULL, CMD_SID_CONN_REQUEST_DESCRIPTION, cmd_sid_conn_request, 2, 0),
	SHELL_CMD_ARG(get_time, NULL, CMD_SID_GET_TIME_DESCRIPTION, cmd_sid_get_time, 2, 0),
	SHELL_CMD_ARG(set_dst_id, NULL, CMD_SID_SET_DST_ID_DESCRIPTION, cmd_sid_set_dst_id, 2, 0),
	SHELL_CMD_ARG(set_send_link, NULL, CMD_SID_SET_SEND_LINK_DESCRIPTION, cmd_sid_set_send_link, 2, 0),
	SHELL_CMD_ARG(set_rsp_id, NULL, CMD_SID_SET_RSP_ID_DESCRIPTION, cmd_sid_set_rsp_id, 2, 0),
	SHELL_SUBCMD_SET_END);

// command, subcommands, help, handler
SHELL_CMD_REGISTER(sid, &sub_services, "sidewalk testing CLI", NULL);

struct cli_config cli_cfg = {
	.send_link_type = SID_LINK_TYPE_ANY,
	.rsp_msg_id = 0,
};

void initialize_sidewalk_shell(struct sid_config *sid_cfg, struct app_context *app_cxt)
{
	cli_cfg.app_cxt = app_cxt;
	cli_cfg.sid_cfg = sid_cfg;

}

#define CLI_CMD_OPT_LINK_BLE        1
#define CLI_CMD_OPT_LINK_FSK        2
#define CLI_CMD_OPT_LINK_LORA       3
#define CLI_CMD_OPT_LINK_LORA_BLE   4

#define CLI_MAX_DATA_LEN            256
#define CLI_MAX_HEX_STR_LEN         (CLI_MAX_DATA_LEN * 2)

static uint8_t ul_buf[CLI_MAX_DATA_LEN];

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
		return -1;
	}

	size_t str_len = strlen(str_p);

	if ((str_len % 2) != 0) {
		return -1;
	}

	size_t data_len = str_len / 2;

	if (buf_len < data_len) {
		return -1;
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
			return -1;
		}
		data_p[(idx / 2)] += value << (((idx + 1) % 2) * 4);
		idx++;
	}
	return data_len;
}

// shell handlers

int cmd_sid_init(const struct shell *shell, int32_t argc, const char **argv)
{
	const char *connection_type_arg = argv[1];
	uint8_t connection_type = atoi(connection_type_arg);

	if (!IN_RANGE(connection_type, 1, 4)) {
		return -ENOEXEC;
	}
	if (!cli_parse_link_mask_opt(connection_type, &cli_cfg.sid_cfg->link_mask)) {
		return -ENOEXEC;
	}
	if (!cli_cfg.app_cxt->sidewalk_handle) {
		shell_error(shell, "Sidewalk CLI not initialized");
		return -ENOEXEC;
	}

	sid_error_t ret = sid_init_delegated(cli_cfg.sid_cfg, cli_cfg.app_cxt->sidewalk_handle);

	shell_info(shell, "sid_init returned %d", ret);
	return 0;
}

int cmd_sid_deinit(const struct shell *shell, int32_t argc, const char **argv)
{
	sid_error_t ret = sid_deinit_delegated(*cli_cfg.app_cxt->sidewalk_handle);

	shell_info(shell, "sid_deinit returned %d", ret);
	return 0;
}

int cmd_sid_start(const struct shell *shell, int32_t argc, const char **argv)
{
	uint32_t link_mask = 0;

	if (argc == 1) {
		link_mask = cli_cfg.sid_cfg->link_mask;
	} else if (argc == 2) {
		if (!cli_parse_link_mask_opt(atoi(argv[1]), &link_mask)) {
			return -ENOEXEC;
		}
	}

	sid_error_t ret = sid_start_delegated(*cli_cfg.app_cxt->sidewalk_handle,
				    link_mask);

	shell_info(shell, "sid_start returned %d", ret);
	return 0;
}

int cmd_sid_stop(const struct shell *shell, int32_t argc, const char **argv)
{
	uint32_t link_mask = 0;

	if (argc == 0) {
		link_mask = cli_cfg.sid_cfg->link_mask;
	} else if (argc == 1) {
		if (!cli_parse_link_mask_opt(atoi(argv[1]), &link_mask)) {
			return -ENOEXEC;
		}
	}

	sid_error_t ret = sid_stop_delegated(*cli_cfg.app_cxt->sidewalk_handle,
				   link_mask);

	shell_info(shell, "sid_stop returned %d", ret);
	return 0;
}

int cmd_sid_send(const struct shell *shell, int32_t argc, const char **argv)
{
	struct sid_msg msg = { .size = 0, .data = NULL };
	struct sid_msg_desc desc = {
		.type = SID_MSG_TYPE_NOTIFY,
		.link_type = cli_cfg.send_link_type,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

	if (argc == 2) {
		// only data
		int res = cli_parse_hexstr(argv[2], ul_buf, CLI_MAX_DATA_LEN);
		if (res < 0) {
			return -ENOEXEC;
		}
		msg.size = (size_t)res;
		msg.data = ul_buf;
	} else {
		// one argument is required, so if argc is not 2 it is greater than 2
		int opt;

		while ((opt = getopt(argc, (char *const *)argv, "t:d:r:"))) {
			switch (opt) {
			case 't':
				switch (optarg[0]) {
				case '0': desc.type = SID_MSG_TYPE_GET; break;
				case '1': desc.type = SID_MSG_TYPE_SET; break;
				case '2': desc.type = SID_MSG_TYPE_NOTIFY; break;
				case '3': desc.type = SID_MSG_TYPE_RESPONSE; break;
				default: return -ENOEXEC;
				}
				break;
			case 'd':
				switch (optarg[0]) {
				case '1': desc.link_mode = SID_LINK_MODE_CLOUD; break;
				case '2': desc.link_mode = SID_LINK_MODE_MOBILE; break;
				default: return -ENOEXEC;
				}
				;
				break;
			case 'r': {
				int res = cli_parse_hexstr(optarg, ul_buf, CLI_MAX_DATA_LEN);
				if (res < 0) {
					return -ENOEXEC;
				}
				msg.size = (size_t)res;
				msg.data = ul_buf;
			};
				break;
			}
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
	sid_error_t ret = sid_set_factory_reset_delegated(*cli_cfg.app_cxt->sidewalk_handle);

	shell_info(shell, "sid_set_factory_reset returned %d", ret);
	return 0;
}

int cmd_sid_get_mtu(const struct shell *shell, int32_t argc, const char **argv)
{
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
		return -ENOEXEC;
	}
	}

	size_t mtu = 0;
	sid_error_t ret = sid_get_mtu_delegated(*cli_cfg.app_cxt->sidewalk_handle, link_type, &mtu);

	shell_info(shell, "sid_get_mtu returned %d, MTU: %d", ret, mtu);
	return 0;
}

int cmd_sid_set_option(const struct shell *shell, int32_t argc, const char **argv)
{
	shell_error(shell, "NOT IMPLEMENTED");
	return 0;
}

int cmd_sid_last_status(const struct shell *shell, int32_t argc, const char **argv)
{
	struct sid_status status = {};
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
		return -ENOEXEC;
	}
	}

	sid_error_t ret = sid_ble_bcn_connection_request_delegated(*cli_cfg.app_cxt->sidewalk_handle, conn_req);

	shell_info(shell, "sid_ble_bcn_connection_request returned %d", ret);
	return 0;
}

int cmd_sid_get_time(const struct shell *shell, int32_t argc, const char **argv)
{
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
		return -ENOEXEC;
	}
	}

	sid_error_t ret = sid_get_time_delegated(*cli_cfg.app_cxt->sidewalk_handle, type, &curr_time);

	shell_info(shell, "sid_get_time returned %d, SEC: %d NSEC: %d", ret, curr_time.tv_sec, curr_time.tv_nsec);
	return 0;
}

int cmd_sid_set_dst_id(const struct shell *shell, int32_t argc, const char **argv)
{

	uint32_t dst_id = atoi(argv[1]);
	sid_error_t ret = sid_set_msg_dest_id_delegated(*cli_cfg.app_cxt->sidewalk_handle, dst_id);

	shell_info(shell, "sid_set_msg_dest_id returned %d", ret);
	return 0;
}

int cmd_sid_set_send_link(const struct shell *shell, int32_t argc, const char **argv)
{
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
		return -ENOEXEC;
	}
	}

	return 0;
}

int cmd_sid_set_rsp_id(const struct shell *shell, int32_t argc, const char **argv)
{
	cli_cfg.rsp_msg_id = atoi(argv[1]);
	return 0;
}
