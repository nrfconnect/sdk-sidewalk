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
#include <sid_hal_memory_ifc.h>

#include <cli/app_shell.h>
#include <cli/app_dut.h>
#include <sid_sdk_version.h>
#include <sidewalk_version.h>

#define CLI_CMD_OPT_LINK_BLE 1
#define CLI_CMD_OPT_LINK_FSK 2
#define CLI_CMD_OPT_LINK_LORA 3
#define CLI_CMD_OPT_LINK_BLE_LORA 4
#define CLI_CMD_OPT_LINK_BLE_FSK 5
#define CLI_CMD_OPT_LINK_FSK_LORA 6
#define CLI_CMD_OPT_LINK_BLE_FSK_LORA 7
#define CLI_CMD_OPT_LINK_ANY 8

#define CLI_MAX_DATA_LEN (CONFIG_SHELL_CMD_BUFF_SIZE / 2)
#define CLI_MAX_HEX_STR_LEN CONFIG_SHELL_CMD_BUFF_SIZE

#define CHECK_ARGUMENT_COUNT(argc, required, optional)                                             \
	if ((argc < required) || (argc > (required + optional))) {                                 \
		return -EINVAL;                                                                    \
	}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_sid_option,
	SHELL_CMD_ARG(-lp_get_l2, NULL,
		      "Gets link profile and associated parameters for SID_LINK_TYPE_2",
		      cmd_sid_option_lp_get_l2, 1, 0),
	SHELL_CMD_ARG(-lp_get_l3, NULL,
		      "Gets link profile and associated parameters for SID_LINK_TYPE_3",
		      cmd_sid_option_lp_get_l3, 1, 0),
	SHELL_CMD_ARG(-gd, NULL, "Get filter duplicates configuration.", cmd_sid_option_gd, 1, 0),
	SHELL_CMD_ARG(-gm, NULL, "Get current link connection policy", cmd_sid_option_gm, 1, 0),
	SHELL_CMD_ARG(-gml, NULL, "Get current configured Multi link policy.", cmd_sid_option_gml,
		      1, 0),
	SHELL_CMD_ARG(-st_get, NULL, CMD_SID_SET_OPTION_ST_GET_DESCRIPTION, cmd_sid_option_st_get,
		      1, 0),
	SHELL_CMD_ARG(-st_clear, NULL, "Clear statistics", cmd_sid_option_st_clear, 1, 0),

	SHELL_CMD_ARG(-lp_set, NULL, CMD_SID_SET_OPTION_LP_SET_DESCRIPTION, cmd_sid_option_lp_set,
		      CMD_SID_SET_OPTION_LP_SET_ARG_REQUIRED,
		      CMD_SID_SET_OPTION_LP_SET_ARG_OPTIONAL),
	SHELL_CMD_ARG(-b, NULL, CMD_SID_SET_OPTION_B_DESCRIPTION, cmd_sid_option_battery,
		      CMD_SID_SET_OPTION_B_ARG_REQUIRED, CMD_SID_SET_OPTION_B_ARG_OPTIONAL),
	SHELL_CMD_ARG(-d, NULL, CMD_SID_SET_OPTION_D_DESCRIPTION, cmd_sid_option_d,
		      CMD_SID_SET_OPTION_D_ARG_REQUIRED, CMD_SID_SET_OPTION_D_ARG_OPTIONAL),
	SHELL_CMD_ARG(-m, NULL, CMD_SID_SET_OPTION_M_DESCRIPTION, cmd_sid_option_m,
		      CMD_SID_SET_OPTION_M_ARG_REQUIRED, CMD_SID_SET_OPTION_M_ARG_OPTIONAL),
	SHELL_CMD_ARG(-c, NULL, CMD_SID_SET_OPION_C_DESCRIPTION, cmd_sid_option_c,
		      CMD_SID_SET_OPTION_C_ARG_REQUIRED, CMD_SID_SET_OPTION_C_ARG_OPTIONAL),
	SHELL_CMD_ARG(-ml, NULL, CMD_SID_SET_OPTION_ML_DESCRIPTION, cmd_sid_option_ml,
		      CMD_SID_SET_OPTION_ML_ARG_REQUIRED, CMD_SID_SET_OPTION_ML_ARG_OPTIONAL),
	SHELL_CMD_ARG(-gc, NULL, CMD_SID_SET_OPTION_GC_DESCRIPTION, cmd_sid_option_gc,
		      CMD_SID_SET_OPTION_GC_ARG_REQUIRED, CMD_SID_SET_OPTION_GC_ARG_OPTIONAL),

	SHELL_SUBCMD_SET_END);

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
	case CLI_CMD_OPT_LINK_BLE_LORA:
		*link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_3;
		break;
	case CLI_CMD_OPT_LINK_BLE_FSK:
		*link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_2;
		break;
	case CLI_CMD_OPT_LINK_FSK_LORA:
		*link_mask = SID_LINK_TYPE_2 | SID_LINK_TYPE_3;
		break;
	case CLI_CMD_OPT_LINK_BLE_FSK_LORA:
		*link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_2 | SID_LINK_TYPE_3;
		break;
	case CLI_CMD_OPT_LINK_ANY:
		*link_mask = SID_LINK_TYPE_ANY;
		break;
	default:
		return false;
	}
	return true;
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

static int cmd_sid_option(cli_event_t event, enum sid_option option, void *data, size_t len)
{
	sidewalk_option_t *p_opt = sid_hal_malloc(sizeof(sidewalk_option_t));
	if (!p_opt) {
		return -ENOMEM;
	}
	memset(p_opt, 0x0, sizeof(*p_opt));
	p_opt->option = option;
	p_opt->data_len = len;
	if (data) {
		p_opt->data = sid_hal_malloc(p_opt->data_len);
		if (!p_opt->data) {
			return -ENOMEM;
		}
		memcpy(p_opt->data, data, p_opt->data_len);
	} else {
		p_opt->data = NULL;
	}

	int err = sidewalk_event_send((sidewalk_event_t)event, p_opt);
	if (err) {
		if (p_opt->data) {
			sid_hal_free(p_opt->data);
		}
		sid_hal_free(p_opt);
		err = -ENOMSG;
	}

	return err;
}

static int cmd_sid_option_set(enum sid_option option, void *data, size_t len)
{
	return cmd_sid_option(DUT_EVENT_SET_OPTION, option, data, len);
}

static int cmd_sid_option_get(enum sid_option option)
{
	return cmd_sid_option(DUT_EVENT_GET_OPTION, option, NULL, 0);
}

static int cmd_sid_option_get_input_data(enum sid_option option, void *data, size_t len)
{
	return cmd_sid_option(DUT_EVENT_GET_OPTION, option, data, len);
}

static int cmd_sid_simple_param(cli_event_t event, uint32_t *data)
{
	uint32_t *event_ctx = sid_hal_malloc(sizeof(uint32_t));
	if (!event_ctx) {
		return -ENOMEM;
	}
	memcpy(event_ctx, data, sizeof(uint32_t));

	int err = sidewalk_event_send((sidewalk_event_t)event, event_ctx);
	if (err) {
		sid_hal_free(event_ctx);
		return -ENOMSG;
	}
	return 0;
}
// shell handlers

int cmd_sid_init(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_INIT_ARG_REQUIRED, CMD_SID_INIT_ARG_OPTIONAL);

	cli_cfg.shell = shell;

	const char *connection_type_arg = argv[1];
	uint8_t connection_type = atoi(connection_type_arg);
	uint32_t link_type = 0;

	if (!IN_RANGE(connection_type, CLI_CMD_OPT_LINK_BLE, CLI_CMD_OPT_LINK_ANY)) {
		shell_error(shell, "invalid value");
		return -EINVAL;
	}

	if (!cli_parse_link_mask_opt(connection_type, &link_type)) {
		shell_error(shell, "invalid value");
		return -EINVAL;
	}
	cli_cfg.send_link_type = link_type;

	return cmd_sid_simple_param(DUT_EVENT_INIT, &link_type);
}

int cmd_sid_deinit(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_DEINIT_ARG_REQUIRED, CMD_SID_DEINIT_ARG_OPTIONAL);

	return sidewalk_event_send((sidewalk_event_t)DUT_EVENT_DEINIT, NULL);
}

int cmd_sid_start(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_START_ARG_REQUIRED, CMD_SID_START_ARG_OPTIONAL);
	uint32_t link_type = 0;

	if (argc == 1) {
		link_type = cli_cfg.send_link_type;
	} else {
		if (!cli_parse_link_mask_opt(atoi(argv[1]), &link_type)) {
			shell_error(shell, "invalid value");
			return -EINVAL;
		}
	}

	return cmd_sid_simple_param(DUT_EVENT_START, &link_type);
}

int cmd_sid_stop(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_STOP_ARG_REQUIRED, CMD_SID_STOP_ARG_OPTIONAL);
	uint32_t link_type = 0;

	if (argc == 1) {
		link_type = cli_cfg.send_link_type;
	} else {
		if (!cli_parse_link_mask_opt(atoi(argv[1]), &link_type)) {
			shell_error(shell, "invalid value");
			return -EINVAL;
		}
	}

	return cmd_sid_simple_param(DUT_EVENT_STOP, &link_type);
}

int cmd_sid_send(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SEND_ARG_REQUIRED, CMD_SID_SEND_ARG_OPTIONAL);

	struct sid_msg msg = (struct sid_msg){ .size = 0, .data = NULL };
	struct sid_msg_desc desc = (struct sid_msg_desc){
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
			uint8_t msg_buffer[strlen(argv[opt]) / 2];
			memset(msg_buffer, 0, sizeof(msg_buffer));
			msg.size = hex2bin(argv[opt], strlen(argv[opt]), msg_buffer,
					   sizeof(msg_buffer));
			if (!msg.size) {
				shell_error(shell, "failed to parse value as hexstring");
				return -EINVAL;
			}
			msg.data = sid_hal_malloc(msg.size);
			if (!msg.data) {
				return -ENOMEM;
			}
			memcpy(msg.data, msg_buffer, msg.size);
			continue;
		}
		if (strcmp("-l", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-l need a value");
				return -EINVAL;
			}

			if (!cli_parse_link_mask_opt(atoi(argv[opt]), &desc.link_type)) {
				shell_error(shell, "invalid value");
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

	if (!msg.data) {
		msg.size = strlen(argv[argc - 1]);
		msg.data = sid_hal_malloc(msg.size);
		if (!msg.data) {
			return -ENOMEM;
		}
		memcpy(msg.data, argv[argc - 1], msg.size);
	}

	sidewalk_msg_t *send = sid_hal_malloc(sizeof(sidewalk_msg_t));
	if (!send) {
		return -ENOMEM;
	}
	memset(send, 0x0, sizeof(*send));
	memcpy(&send->msg, &msg, sizeof(struct sid_msg));
	memcpy(&send->desc, &desc, sizeof(struct sid_msg_desc));

	int err = sidewalk_event_send((sidewalk_event_t)SID_EVENT_SEND_MSG, send);
	if (err) {
		sid_hal_free(send->msg.data);
		sid_hal_free(send);
		return -ENOMSG;
	}

	return 0;
}

int cmd_sid_factory_reset(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_FACTORY_RESET_ARG_REQUIRED,
			     CMD_SID_FACTORY_RESET_ARG_OPTIONAL);

	int err = sidewalk_event_send((sidewalk_event_t)SID_EVENT_FACTORY_RESET, NULL);
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_get_mtu(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_GET_MTU_ARG_REQUIRED, CMD_SID_GET_MTU_ARG_OPTIONAL);

	uint32_t link_mask = 0;

	switch (argv[1][0]) {
	case '1':
		link_mask = SID_LINK_TYPE_1;
		break;
	case '2':
		link_mask = SID_LINK_TYPE_2;
		break;
	case '3':
		link_mask = SID_LINK_TYPE_3;
		break;
	default:
		shell_error(shell, "invalid value");
		return -EINVAL;
	}

	return cmd_sid_simple_param(DUT_EVENT_GET_MTU, &link_mask);
}

int cmd_sid_option_battery(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_OPTION_B_ARG_REQUIRED,
			     CMD_SID_SET_OPTION_B_ARG_OPTIONAL);

	long data_raw = 0l;
	uint8_t data;
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

	int err = cmd_sid_option_set(SID_OPTION_BLE_BATTERY_LEVEL, &data, sizeof(data));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_lp_set(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_OPTION_LP_SET_ARG_REQUIRED,
			     CMD_SID_SET_OPTION_LP_SET_ARG_OPTIONAL);

	static enum sid_device_profile_id dev_profile = SID_LINK3_PROFILE_LAST;
	long data_raw = 0l;
	char *end = NULL;

	data_raw = strtol(argv[1], &end, 0);
	if (end == argv[1]) {
		shell_error(shell, "invalid value");
		return -EINVAL;
	}
	if (!IN_RANGE(data_raw, 0, UINT8_MAX)) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>", argv[1],
			    (unsigned int)UINT8_MAX);
		return -EINVAL;
	}

	dev_profile = (enum sid_device_profile_id)data_raw;

	struct sid_device_profile dev_cfg = { 0 };
	dev_cfg.unicast_params.device_profile_id = dev_profile;
	dev_cfg.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP;

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
	} break;
	case SID_LINK2_PROFILE_1: {
		CHECK_ARGUMENT_COUNT(argc, 2, 0);
		dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
	} break;
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
	} break;
	default: {
		shell_error(
			shell,
			"Invalid argument: [%s]\n valid values for profile are: [0x80, 0x81, 0x83, 0x01, 0x02]",
			argv[1]);
		return -EINVAL;
	} break;
	}

	int err =
		cmd_sid_option_set(SID_OPTION_900MHZ_SET_DEVICE_PROFILE, &dev_cfg, sizeof(dev_cfg));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_lp_get_l2(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	enum sid_device_profile_id profile = SID_LINK2_PROFILE_1;
	int err = cmd_sid_option_get_input_data(SID_OPTION_900MHZ_GET_DEVICE_PROFILE, &profile,
						sizeof(profile));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_lp_get_l3(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	enum sid_device_profile_id profile = SID_LINK3_PROFILE_A;
	int err = cmd_sid_option_get_input_data(SID_OPTION_900MHZ_GET_DEVICE_PROFILE, &profile,
						sizeof(profile));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_d(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_OPTION_D_ARG_REQUIRED,
			     CMD_SID_SET_OPTION_D_ARG_OPTIONAL);

	long data_raw = 0l;
	uint8_t data = 0;
	char *end = NULL;

	data_raw = strtol(argv[1], &end, 0);
	if (end == argv[1] || !IN_RANGE(data_raw, 0, 1)) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
			    argv[1] == NULL ? "NULL" : argv[1], (unsigned int)1);
		return -EINVAL;
	}
	data = (uint8_t)data_raw;

	int err = cmd_sid_option_set(SID_OPTION_SET_MSG_POLICY_FILTER_DUPLICATES, &data,
				     sizeof(data));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_gd(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	int err = cmd_sid_option_get(SID_OPTION_GET_MSG_POLICY_FILTER_DUPLICATES);
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_gm(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	int err = cmd_sid_option_get(SID_OPTION_GET_LINK_CONNECTION_POLICY);
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_gml(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	int err = cmd_sid_option_get(SID_OPTION_GET_LINK_POLICY_MULTI_LINK_POLICY);
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_st_get(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	int err = cmd_sid_option_get(SID_OPTION_GET_STATISTICS);
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_st_clear(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, 1, 0);

	int err = cmd_sid_option_set(SID_OPTION_CLEAR_STATISTICS, NULL, 0);
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_m(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_OPTION_M_ARG_REQUIRED,
			     CMD_SID_SET_OPTION_M_ARG_OPTIONAL);
	long data_raw = 0l;
	uint8_t data = 0;
	char *end = NULL;

	data_raw = strtol(argv[1], &end, 0);
	if (end == argv[1] || !IN_RANGE(data_raw, 0, 2)) {
		shell_error(shell, "Invalid argument [%s], must be value <0, %x>",
			    argv[1] == NULL ? "NULL" : argv[1], (unsigned int)2);
		return -EINVAL;
	}
	data = (uint8_t)data_raw;

	int err = cmd_sid_option_set(SID_OPTION_SET_LINK_CONNECTION_POLICY, &data, sizeof(data));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_c(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_OPTION_C_ARG_REQUIRED,
			     CMD_SID_SET_OPTION_C_ARG_OPTIONAL);
	struct sid_link_auto_connect_params params = {};
	params.priority = UINT8_MAX;
	params.connection_attempt_timeout_seconds = UINT16_MAX;

	long link_type = 0l;
	char *end = NULL;
	link_type = strtol(argv[1], &end, 0);
	if (end == argv[1] || !IN_RANGE(link_type, CLI_CMD_OPT_LINK_BLE, CLI_CMD_OPT_LINK_ANY)) {
		shell_error(shell, "parameter link type invalid");
		return -EINVAL;
	}
	cli_parse_link_mask_opt((uint8_t)link_type, (uint32_t *)&params.link_type);

	long enable = strtol(argv[2], &end, 0);
	if (end == argv[2] || !IN_RANGE(enable, 0, 1)) {
		shell_error(shell, "parameter enable invalid");
		return -EINVAL;
	}
	params.enable = enable;
	if (argc >= 4) {
		long priority = strtol(argv[3], &end, 0);
		if (end == argv[3] || !IN_RANGE(priority, 0, 255)) {
			shell_error(shell, "parameter priority invalid");
			return -EINVAL;
		}
		params.priority = priority;
	}

	if (argc >= 5) {
		long timeout = strtol(argv[4], &end, 0);
		if (end == argv[4] || !IN_RANGE(timeout, 0, UINT16_MAX)) {
			shell_error(shell, "parameter timeout invalid");
			return -EINVAL;
		}
		params.connection_attempt_timeout_seconds = timeout;
	}

	int err = cmd_sid_option_set(SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS, &params,
				     sizeof(params));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_ml(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_OPTION_ML_ARG_REQUIRED,
			     CMD_SID_SET_OPTION_ML_ARG_OPTIONAL);
	long policy_raw = 0l;
	uint8_t policy;
	char *end = NULL;
	policy_raw = strtol(argv[1], &end, 0);
	if (end == argv[1] || !IN_RANGE(policy_raw, 0, 4)) {
		shell_error(shell, "parameter policy invalid");
		return -EINVAL;
	}
	policy = policy_raw;

	int err = cmd_sid_option_set(SID_OPTION_SET_LINK_POLICY_MULTI_LINK_POLICY, &policy,
				     sizeof(policy));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_option_gc(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_OPTION_GC_ARG_REQUIRED,
			     CMD_SID_SET_OPTION_GC_ARG_OPTIONAL);
	long link_type = 0l;
	char *end = NULL;
	link_type = strtol(argv[1], &end, 0);
	if (end == argv[1] || !IN_RANGE(link_type, CLI_CMD_OPT_LINK_BLE, CLI_CMD_OPT_LINK_ANY)) {
		shell_error(shell, "parameter link type invalid");
		return -EINVAL;
	}
	uint32_t *p_link_mask = sid_hal_malloc(sizeof(uint32_t));
	if (!p_link_mask) {
		return -ENOMEM;
	}

	memset(p_link_mask, 0x0, sizeof(*p_link_mask));
	if (!cli_parse_link_mask_opt((uint8_t)link_type, p_link_mask)) {
		shell_error(shell, "Can not parse link mask");
		return -EINVAL;
	}

	int err = cmd_sid_option_get_input_data(SID_OPTION_GET_LINK_POLICY_AUTO_CONNECT_PARAMS,
						p_link_mask, sizeof(uint32_t));
	if (err) {
		shell_error(shell, "event err %d", err);
	}

	return 0;
}

int cmd_sid_last_status(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_LAST_STATUS_ARG_REQUIRED,
			     CMD_SID_LAST_STATUS_ARG_OPTIONAL);

	if (0 != sidewalk_event_send((sidewalk_event_t)DUT_EVENT_GET_STATUS, NULL)) {
		shell_error(shell, "Failed to send Event");
	}
	return 0;
}

int cmd_sid_conn_request(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_CONN_REQUEST_ARG_REQUIRED,
			     CMD_SID_CONN_REQUEST_ARG_OPTIONAL);

	uint32_t conn_req = 0;
	switch (argv[1][0]) {
	case '1':
		conn_req = 1U;
		break;
	case '0':
		conn_req = 0U;
		break;
	default:
		shell_error(shell, "invalid value");
		return -EINVAL;
	}

	return cmd_sid_simple_param(DUT_EVENT_SET_CONN_REQ, &conn_req);
}

int cmd_sid_get_time(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_GET_TIME_ARG_REQUIRED, CMD_SID_GET_TIME_ARG_OPTIONAL);

	if (argv[1][0] != '0') {
		shell_error(shell, "invalid value");
		return -EINVAL;
	}
	uint32_t time_type = SID_GET_GPS_TIME;

	return cmd_sid_simple_param(DUT_EVENT_GET_TIME, &time_type);
}

int cmd_sid_set_dst_id(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_SID_SET_DST_ID_ARG_REQUIRED,
			     CMD_SID_SET_DST_ID_ARG_OPTIONAL);
	uint32_t dst_id = atoi(argv[1]);
	return cmd_sid_simple_param(DUT_EVENT_SET_DEST_ID, &dst_id);
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
		shell_error(shell, "invalid value");
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
