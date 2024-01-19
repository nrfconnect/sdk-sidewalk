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

#include <cli/app_shell.h>
#include <cli/app_dut.h>
#include <sid_sdk_version.h>
#include <sidewalk_version.h>

#define CLI_CMD_OPT_LINK_BLE 1
#define CLI_CMD_OPT_LINK_FSK 2
#define CLI_CMD_OPT_LINK_LORA 3
#define CLI_CMD_OPT_LINK_LORA_BLE 4
#define CLI_CMD_OPT_LINK_FSK_BLE 5

#define CLI_MAX_DATA_LEN (CONFIG_SHELL_CMD_BUFF_SIZE / 2)
#define CLI_MAX_HEX_STR_LEN CONFIG_SHELL_CMD_BUFF_SIZE

#define CHECK_ARGUMENT_COUNT(argc, required, optional)                                             \
	if ((argc < required) || (argc > (required + optional))) {                                 \
		return -EINVAL;                                                                    \
	}

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

static size_t cli_hexstr_len(const char *str_p)
{
	if (!str_p) {
		return 0;
	}

	return strlen(str_p) / 2;
}

static int cli_hexstr_parse(uint8_t *data_p, const char *str_p, size_t data_len)
{
	if (!str_p || !data_p) {
		return -EINVAL;
	}

	memset(data_p, 0, data_len);

	for (size_t i = 0; i < data_len; i++) {
		char c = str_p[i];
		int value = 0;
		if (c >= '0' && c <= '9') {
			value = (c - '0');
		} else if (c >= 'A' && c <= 'F') {
			value = (10 + (c - 'A'));
		} else if (c >= 'a' && c <= 'f') {
			value = (10 + (c - 'a'));
		} else {
			return -EFAULT;
		}
		data_p[(i / 2)] += value << (((i + 1) % 2) * 4);
	}
	return 0;
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

// shell handlers

int cmd_sid_init(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_INIT_ARG_REQUIRED, CMD_SID_INIT_ARG_OPTIONAL);

	cli_cfg.shell = shell;

	const char *connection_type_arg = argv[1];
	uint8_t connection_type = atoi(connection_type_arg);
	uint32_t *p_link_mask = sidewalk_data_alloc(sizeof(uint32_t));

	if (!cli_parse_link_mask_opt(connection_type, p_link_mask)) {
		sidewalk_data_free(p_link_mask);
		return -EINVAL;
	}
	cli_cfg.send_link_type = *p_link_mask;

	int err = sidewalk_event_send(DUT_EVENT_INIT, p_link_mask);
	if (err) {
		sidewalk_data_free(p_link_mask);
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_deinit(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_DEINIT_ARG_REQUIRED, CMD_SID_DEINIT_ARG_OPTIONAL);

	return sidewalk_event_send(DUT_EVENT_DEINIT, NULL);
}

int cmd_sid_start(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_START_ARG_REQUIRED, CMD_SID_START_ARG_OPTIONAL);
	uint32_t *p_link_mask = sidewalk_data_alloc(sizeof(uint32_t));

	if (argc == 1) {
		*p_link_mask = cli_cfg.send_link_type;
	} else {
		if (!cli_parse_link_mask_opt(atoi(argv[1]), p_link_mask)) {
			sidewalk_data_free(p_link_mask);
			return -EINVAL;
		}
	}

	int err = sidewalk_event_send(DUT_EVENT_START, p_link_mask);
	if (err) {
		sidewalk_data_free(p_link_mask);
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_stop(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_STOP_ARG_REQUIRED, CMD_SID_STOP_ARG_OPTIONAL);
	uint32_t *p_link_mask = sidewalk_data_alloc(sizeof(uint32_t));

	if (argc == 1) {
		*p_link_mask = cli_cfg.send_link_type;
	} else {
		if (!cli_parse_link_mask_opt(atoi(argv[1]), p_link_mask)) {
			sidewalk_data_free(p_link_mask);
			return -EINVAL;
		}
	}

	int err = sidewalk_event_send(DUT_EVENT_STOP, p_link_mask);
	if (err) {
		sidewalk_data_free(p_link_mask);
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_send(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SEND_ARG_REQUIRED, CMD_SID_SEND_ARG_OPTIONAL);

	sidewalk_msg_t *send = sidewalk_data_alloc(sizeof(sidewalk_msg_t));

	bool has_msg_data = false;
	int err = 0;
	for (int opt = 1; opt < argc; opt++) {
		if (strcmp("-t", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-t need a value");
				err = -EINVAL;
				break;
			}
			switch (argv[opt][0]) {
			case '0':
				send->desc.type = SID_MSG_TYPE_GET;
				break;
			case '1':
				send->desc.type = SID_MSG_TYPE_SET;
				break;
			case '2':
				send->desc.type = SID_MSG_TYPE_NOTIFY;
				break;
			case '3':
				send->desc.type = SID_MSG_TYPE_RESPONSE;
				break;
			default: {
				shell_error(shell, "invalid type");
				err = -EINVAL;
				break;
			}
			}
			continue;
		}
		if (strcmp("-d", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-d need a value");
				err = -EINVAL;
				break;
			}
			switch (argv[opt][0]) {
			case '1':
				send->desc.link_mode = SID_LINK_MODE_CLOUD;
				break;
			case '2':
				send->desc.link_mode = SID_LINK_MODE_MOBILE;
				break;
			default: {
				shell_error(shell, "invalid mode");
				err = -EINVAL;
				break;
			}
			}
			continue;
		}
		if (strcmp("-r", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-r need a value");
				err = -EINVAL;
				break;
			}

			send->msg.size = cli_hexstr_len(argv[opt]);
			send->msg.data = sidewalk_data_alloc(send->msg.size);
			int err = cli_hexstr_parse(send->msg.data, argv[opt], send->msg.size);
			if (err) {
				sidewalk_data_free(send->msg.data);
				shell_error(shell, "failed to parse value as hexstring");
				err = -EINVAL;
				break;
			} else {
				has_msg_data = true;
			}
			continue;
		}
		if (strcmp("-l", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-l need a value");
				err = -EINVAL;
				break;
			}

			if (!cli_parse_link_mask_opt(atoi(argv[opt]), &send->desc.link_type)) {
				err = -EINVAL;
				break;
			}
			continue;
		}
		if (strcmp("-i", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-i need a value");
				err = -EINVAL;
				break;
			}
			char *end = NULL;
			long data_raw = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(data_raw, 0, UINT16_MAX) || end == argv[opt]) {
				shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
					    argv[opt], (unsigned int)UINT16_MAX);
				err = -EINVAL;
				break;
			}
			cli_cfg.rsp_msg_id = (uint16_t)data_raw;
			continue;
		}
		if (strcmp("-a", argv[opt]) == 0) {
			if (opt + 3 > argc) {
				shell_error(shell, "-a need 3 positional arguments");
				err = -EINVAL;
				break;
			}

			const char *ack = argv[++opt];
			const char *retry = argv[++opt];
			const char *ttl = argv[++opt];
			char *end = NULL;
			long ack_val = strtol(ack, &end, 0);
			if (!IN_RANGE(ack_val, 0, 1) || end == ack) {
				shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
					    ack, 1);
				err = -EINVAL;
				break;
			}
			send->desc.msg_desc_attr.tx_attr.request_ack = (bool)ack_val;

			long retry_val = strtol(retry, &end, 0);
			if (!IN_RANGE(ack_val, 0, UINT8_MAX) || end == ack) {
				shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
					    retry, (unsigned int)UINT8_MAX);
				err = -EINVAL;
				break;
			}
			send->desc.msg_desc_attr.tx_attr.num_retries = (uint8_t)retry_val;

			long ttl_val = strtol(ttl, &end, 0);
			if (!IN_RANGE(ttl_val, 0, UINT16_MAX) || end == ack) {
				shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
					    ttl, (unsigned int)UINT16_MAX);
				err = -EINVAL;
				break;
			}
			send->desc.msg_desc_attr.tx_attr.ttl_in_seconds = (uint16_t)ttl_val;
		}
	}

	if (err) {
		sidewalk_data_free(send);
		return err;
	}

	if (!has_msg_data) {
		send->msg.size = strlen(argv[argc - 1]);
		send->msg.data = sidewalk_data_alloc(send->msg.size);
		memcpy(send->msg.data, argv[argc - 1], send->msg.size);
	}

	send->desc.type = SID_MSG_TYPE_NOTIFY;
	send->desc.link_mode = SID_LINK_MODE_CLOUD;
	send->desc.link_type = cli_cfg.send_link_type;
	if (send->desc.type == SID_MSG_TYPE_RESPONSE) {
		send->desc.id = cli_cfg.rsp_msg_id;
	}

	err = sidewalk_event_send(SID_EVENT_SEND_MSG, send);
	if (err) {
		sidewalk_data_free(send->msg.data);
		sidewalk_data_free(send);
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_factory_reset(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_FACTORY_RESET_ARG_REQUIRED,
			     CMD_SID_FACTORY_RESET_ARG_OPTIONAL);

	int err = sidewalk_event_send(SID_EVENT_FACTORY_RESET, NULL);
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_get_mtu(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_GET_MTU_ARG_REQUIRED, CMD_SID_GET_MTU_ARG_OPTIONAL);

	uint32_t *p_link_mask = sidewalk_data_alloc(sizeof(uint32_t));

	switch (argv[1][0]) {
	case '1':
		*p_link_mask = SID_LINK_TYPE_1;
		break;
	case '2':
		*p_link_mask = SID_LINK_TYPE_2;
		break;
	case '3':
		*p_link_mask = SID_LINK_TYPE_3;
		break;
	default:
		sidewalk_data_free(p_link_mask);
		return -EINVAL;
	}

	int err = sidewalk_event_send(DUT_EVENT_GET_MTU, p_link_mask);
	if (err) {
		sidewalk_data_free(p_link_mask);
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_battery(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 2, 0);

	long data_raw = 0l;
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

	sidewalk_option_t *battery = sidewalk_data_alloc(sizeof(sidewalk_option_t));
	battery->option = SID_OPTION_BLE_BATTERY_LEVEL;
	battery->data_len = sizeof(uint8_t);
	battery->data = sidewalk_data_alloc(battery->data_len);
	*((uint8_t *)battery->data) = (uint8_t)data_raw;

	int err = sidewalk_event_send(SID_EVENT_OPTION_SET, battery);
	if (err) {
		sidewalk_data_free(battery->data);
		sidewalk_data_free(battery);
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_lp_set(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 2, 1);

	static enum sid_device_profile_id dev_profile = SID_LINK3_PROFILE_LAST;
	long data_raw = 0l;
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

	struct sid_device_profile *p_dev_cfg =
		sidewalk_data_alloc(sizeof(struct sid_device_profile));
	p_dev_cfg->unicast_params.device_profile_id = dev_profile;
	p_dev_cfg->unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP;

	int err = 0;
	switch (dev_profile) {
	case SID_LINK3_PROFILE_A:
	case SID_LINK3_PROFILE_B:
	case SID_LINK3_PROFILE_D: {
		CHECK_ARGUMENT_COUNT(argc, 3, 0);
		if (cmd_sid_option_handle_set_link3_profile(argv[2], p_dev_cfg) != 0) {
			shell_error(shell, "Invalid argument [%s], must be value <0, %d>", argv[2],
				    (unsigned int)UINT16_MAX);
			err = -EINVAL;
			break;
		}
		break;
	}
	case SID_LINK2_PROFILE_1: {
		CHECK_ARGUMENT_COUNT(argc, 2, 0);
		p_dev_cfg->unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
		break;
	}
	case SID_LINK2_PROFILE_2: {
		long window_separation_ms_raw = 0;
		p_dev_cfg->unicast_params.unicast_window_interval.sync_rx_interval_ms =
			SID_LINK2_RX_WINDOW_SEPARATION_1;
		if (argc == 3) {
			char *end = NULL;
			window_separation_ms_raw = strtol(argv[2], &end, 0);
			if (sid_option_get_window_separation_ms(
				    window_separation_ms_raw,
				    &p_dev_cfg->unicast_params.unicast_window_interval
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
				err = -EINVAL;
				break;
			}
		}
		p_dev_cfg->unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
		break;
	}
	default: {
		shell_error(
			shell,
			"Invalid argument: [%s]\n valid values for profile are: [0x80, 0x81, 0x83, 0x01, 0x02]",
			argv[1]);
		err = -EINVAL;
	} break;
	}

	if (err) {
		sidewalk_data_free(p_dev_cfg);
		return err;
	}

	sidewalk_option_t *lp_set = sidewalk_data_alloc(sizeof(sidewalk_option_t));
	lp_set->option = SID_OPTION_900MHZ_SET_DEVICE_PROFILE;
	lp_set->data_len = sizeof(struct sid_device_profile);
	lp_set->data = p_dev_cfg;
	err = sidewalk_event_send(SID_EVENT_OPTION_SET, lp_set);
	if (err) {
		sidewalk_data_free(lp_set->data);
		sidewalk_data_free(lp_set);
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_lp_get_l2(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	sidewalk_option_t *lp_get = sidewalk_data_alloc(sizeof(sidewalk_option_t));
	lp_get->option = SID_OPTION_900MHZ_GET_DEVICE_PROFILE;
	lp_get->data_len = sizeof(enum sid_device_profile_id);
	lp_get->data = sidewalk_data_alloc(lp_get->data_len);
	*((enum sid_device_profile_id *)lp_get->data) = SID_LINK2_PROFILE_1;

	int err = sidewalk_event_send(DUT_EVENT_GET_OPTION, lp_get);
	if (err) {
		sidewalk_data_free(lp_get->data);
		sidewalk_data_free(lp_get);
		shell_error(shell, "event err %d", err);
	}
	return 0;
}

int cmd_sid_option_lp_get_l3(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);
	sidewalk_option_t *lp_get = sidewalk_data_alloc(sizeof(sidewalk_option_t));
	lp_get->option = SID_OPTION_900MHZ_GET_DEVICE_PROFILE;
	lp_get->data_len = sizeof(enum sid_device_profile_id);
	lp_get->data = sidewalk_data_alloc(lp_get->data_len);
	*((enum sid_device_profile_id *)lp_get->data) = SID_LINK3_PROFILE_A;

	int err = sidewalk_event_send(DUT_EVENT_GET_OPTION, lp_get);
	if (err) {
		sidewalk_data_free(lp_get->data);
		sidewalk_data_free(lp_get);
		shell_error(shell, "event err %d", err);
	}
	return 0;
}

int cmd_sid_option_d(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 2, 0);

	long data_raw = 0l;
	char *end = NULL;

	data_raw = strtol(argv[1], &end, 0);
	if (end == argv[1] || !IN_RANGE(data_raw, 0, 1)) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
			    argv[1] == NULL ? "NULL" : argv[1], (unsigned int)1);
		return -EINVAL;
	}

	sidewalk_option_t *filter = sidewalk_data_alloc(sizeof(sidewalk_option_t));
	filter->option = SID_OPTION_SET_MSG_POLICY_FILTER_DUPLICATES;
	filter->data_len = sizeof(uint8_t);
	filter->data = sidewalk_data_alloc(filter->data_len);
	*((uint8_t *)filter->data) = (uint8_t)data_raw;

	int err = sidewalk_event_send(SID_EVENT_OPTION_SET, filter);
	if (err) {
		sidewalk_data_free(filter->data);
		sidewalk_data_free(filter);
		shell_error(shell, "event err %d", err);
	}
	return 0;
}

int cmd_sid_option_gd(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	sidewalk_option_t *filter = sidewalk_data_alloc(sizeof(sidewalk_option_t));
	filter->option = SID_OPTION_GET_MSG_POLICY_FILTER_DUPLICATES;
	filter->data = NULL;
	filter->data_len = 0;

	int err = sidewalk_event_send(DUT_EVENT_GET_OPTION, filter);
	if (err) {
		sidewalk_data_free(filter);
		shell_error(shell, "event err %d", err);
	}
	return 0;
}

int cmd_sid_last_status(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_LAST_STATUS_ARG_REQUIRED,
			     CMD_SID_LAST_STATUS_ARG_OPTIONAL);

	sidewalk_event_send(DUT_EVENT_GET_STATUS, NULL);
	return 0;
}

int cmd_sid_conn_request(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_CONN_REQUEST_ARG_REQUIRED,
			     CMD_SID_CONN_REQUEST_ARG_OPTIONAL);

	uint32_t *req = sidewalk_data_alloc(sizeof(uint32_t));

	switch (argv[1][0]) {
	case '1': {
		*req = 1U;
		break;
	}
	case '0': {
		*req = 0U;
		break;
	}
	default: {
		return -EINVAL;
	}
	}

	int err = sidewalk_event_send(DUT_EVENT_SET_CONN_REQ, req);
	if (err) {
		sidewalk_data_free(req);
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_get_time(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_GET_TIME_ARG_REQUIRED, CMD_SID_GET_TIME_ARG_OPTIONAL);
	uint32_t *p_type = sidewalk_data_alloc(sizeof(uint32_t));

	switch (argv[1][0]) {
	case '0':
		*p_type = SID_GET_GPS_TIME;
		break;
	case '1':
		*p_type = SID_GET_UTC_TIME;
		break;
	case '2':
		*p_type = SID_GET_LOCAL_TIME;
		break;
	default:
		sidewalk_data_free(p_type);
		return -EINVAL;
	}

	int err = sidewalk_event_send(DUT_EVENT_GET_TIME, p_type);
	if (err) {
		sidewalk_data_free(p_type);
		shell_error(shell, "event err %d", err);
	}
	return 0;
}

int cmd_sid_set_dst_id(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_DST_ID_ARG_REQUIRED,
			     CMD_SID_SET_DST_ID_ARG_OPTIONAL);
	uint32_t *p_id = sidewalk_data_alloc(sizeof(uint32_t));
	*((uint32_t *)p_id) = atoi(argv[1]);
	int err = sidewalk_event_send(DUT_EVENT_SET_DEST_ID, p_id);
	if (err) {
		sidewalk_data_free(p_id);
		shell_error(shell, "event err %d", err);
	}
	return 0;
}

int cmd_sid_set_send_link(const struct shell *shell, int32_t argc, const char **argv)
{
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
