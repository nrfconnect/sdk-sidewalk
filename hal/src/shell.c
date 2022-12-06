/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "sys/util.h"
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/shell/shell.h>
#include <zephyr/kernel.h>
#include <logging/log.h>

#include <sid_api.h>
#include <sidewalk_thread.h>
#include "buttons_internal.h"

#include <sidewalk_version.h>
#include <sid_shell.h>

#include <result.h>
#include <json_printer.h>

LOG_MODULE_REGISTER(sid_cli, CONFIG_SIDEWALK_LOG_LEVEL);


#define SID_API_STACK_SIZE 5120
#define SID_API_WORKER_PRIO 5

K_THREAD_STACK_DEFINE(sid_api_work_q_stack, SID_API_STACK_SIZE);

struct k_work_q sid_api_work_q;

DECLARE_RESULT(uint32_t, ButtonParserResult, EMPTY_ARG, INVALID_INPUT);
DECLARE_RESULT(size_t, StrToHexRet, OUT_TOO_SMALL, ARG_NOT_HEX, ARG_EMPTY, ARG_NOT_ALIGNED);
DECLARE_RESULT(uint8_t, HexToByteRet, NOT_HEX);
DECLARE_RESULT(uint32_t, StrToDecRet, NOT_NUMBER);

#define MAX_MESSAGE_SIZE_BYTES 512

struct messages_stats {
	uint32_t tx_successfull;
	uint32_t tx_failed;
	uint32_t rx_successfull;
	uint16_t resp_id;
};

static const struct sid_status dummy = {
	.state = SID_STATE_SECURE_CHANNEL_READY + 1,
	.detail = { .link_status_mask = 0,
		    .registration_status = SID_STATUS_NOT_REGISTERED,
		    .time_sync_status = SID_STATUS_NO_TIME }

};

struct sid_send_work_t {
	struct k_work work;
	struct sid_msg msg;
	enum sid_msg_type type;
	uint16_t resp_id;
	const struct shell *shell;
};

struct sid_send_work_t send_work;

static const struct sid_status *CLI_status = &dummy;
struct messages_stats sidewalk_messages;
static app_context_t *sid_app_ctx;

void CLI_register_message_send()
{
	sidewalk_messages.tx_successfull++;
}

void CLI_register_message_not_send()
{
	sidewalk_messages.tx_failed++;
}

void CLI_register_message_received(uint16_t resp_id)
{
	sidewalk_messages.rx_successfull++;
	sidewalk_messages.resp_id = resp_id;
}

void CLI_init(app_context_t *ctx)
{
	sid_app_ctx = ctx;

	k_work_queue_init(&sid_api_work_q);

	k_work_queue_start(&sid_api_work_q, sid_api_work_q_stack,
                   K_THREAD_STACK_SIZEOF(sid_api_work_q_stack), SID_API_WORKER_PRIO,
                   NULL);
}

void CLI_register_sid_status(const struct sid_status *status)
{
	CLI_status = status;
}

static ButtonParserResult button_id_from_arg(const char *arg)
{
	if (NULL == arg) {
		return (ButtonParserResult) Result_Err(EMPTY_ARG);
	}

	if (strlen(arg) != 1) {
		return (ButtonParserResult) Result_Err(INVALID_INPUT);
	}

	switch (*arg) {
	case '1': return (ButtonParserResult) Result_Val(0);
	case '2': return (ButtonParserResult) Result_Val(1);
	case '3': return (ButtonParserResult) Result_Val(2);
	case '4': return (ButtonParserResult) Result_Val(3);
	default: return (ButtonParserResult) Result_Err(INVALID_INPUT);
	}
}

static int press_button(button_action_t action, const struct shell *shell, size_t argc,
			    char **argv)
{
	ButtonParserResult button_id = button_id_from_arg(argv[1]);

	if (button_id.result == Err) {
		shell_error(shell, "invalid button selectd to press:\nexample: sidewalk press_button 4");
		return CMD_RETURN_ARGUMENT_INVALID;
	}
	button_pressed(button_id.val_or_err.val, action);
	shell_info(shell, "sidewalk cli: button pressed");
	return CMD_RETURN_OK;
}

static int cmd_button_press_long(const struct shell *shell, size_t argc, char **argv)
{
	return press_button(BUTTON_ACTION_LONG_PRESS, shell, argc, argv);
}

static int cmd_button_press_short(const struct shell *shell, size_t argc, char **argv)
{
	return press_button(BUTTON_ACTION_SHORT_PRESS, shell, argc, argv);
}

static HexToByteRet hex_nib_to_byte(char hex)
{
	if (IN_RANGE(hex, '0', '9')) {
		return (HexToByteRet) Result_Val(hex - '0');
	}
	if (IN_RANGE(hex, 'A', 'F')) {
		return (HexToByteRet) Result_Val(hex - 'A' + 10);
	}
	if (IN_RANGE(hex, 'a', 'f')) {
		return (HexToByteRet) Result_Val(hex - 'a' + 10);
	}

	return (HexToByteRet) Result_Err(NOT_HEX);
}

static StrToHexRet convert_hex_str_to_data(uint8_t *out, size_t out_limit, char *in)
{
	uint8_t *working_ptr = in;

	if (strlen(in) < 2) {
		return (StrToHexRet) Result_Err(ARG_EMPTY);
	}

	if (in[0] == '0' && in[1] == 'x') {
		if (strlen(in) == 2) {
			return (StrToHexRet) Result_Err(ARG_EMPTY);
		}
		working_ptr = &in[2];
	}

	size_t in_size = strlen(working_ptr);

	if (in_size % 2 != 0) {
		return (StrToHexRet) Result_Err(ARG_NOT_ALIGNED);
	}

	uint8_t payload_size = 0;

	for (; *working_ptr != '\0' && *(working_ptr + 1) != '\0'; working_ptr = working_ptr + 2) {
		HexToByteRet hi = hex_nib_to_byte(*working_ptr);
		HexToByteRet lo = hex_nib_to_byte(*(working_ptr + 1));

		if (hi.result == Err || lo.result == Err) {
			return (StrToHexRet) Result_Err(ARG_NOT_HEX);
		}
		out[payload_size] = (hi.val_or_err.val << 4) + lo.val_or_err.val;
		payload_size++;
	}
	return (StrToHexRet) Result_Val(payload_size);

}

static StrToDecRet string_to_dec(char *string)
{
	uint32_t value = 0;

	while (*string != '\0') {
		if (IN_RANGE(*string, '0', '9') == false) {
			return (StrToDecRet) Result_Err(NOT_NUMBER);
		}
		value *= 10;
		value += (uint8_t)((*string) - '0');
		string++;
	}
	return (StrToDecRet) Result_Val(value);
}

static int cmd_set_response_id(const struct shell *shell, size_t argc, char **argv)
{
	if (strlen(argv[1]) > 5) {
		shell_error(shell,
			    "Value out of valid range [0:16383]\nexample: $ sidewalk set_response_id 123");
		return CMD_RETURN_ARGUMENT_INVALID;
	}

	StrToDecRet id_raw = string_to_dec(argv[1]);

	if (id_raw.result == Err) {
		shell_error(shell,
			    "Invalid sequence id of message valid range [0:16383]\nexample: $ sidewalk set_response_id 123");
		return CMD_RETURN_ARGUMENT_INVALID;
	}

	if (id_raw.val_or_err.val > 16383) {
		shell_error(shell,
			    "Invalid sequence id of message valid range [0:16383]\nexample: $ sidewalk set_response_id 123");
		return CMD_RETURN_ARGUMENT_INVALID;
	}

	sidewalk_messages.resp_id = id_raw.val_or_err.val;
	return CMD_RETURN_OK;
}

static void cmd_send_work(struct k_work *item){
	struct sid_send_work_t *sid_send_work =
        CONTAINER_OF(item, struct sid_send_work_t, work);

	if (!(STATE_SIDEWALK_READY == sid_app_ctx->state) &&
	    !(STATE_SIDEWALK_SECURE_CONNECTION == sid_app_ctx->state)) {
		LOG_ERR("sidewalk is not ready yet!");
		return;
	}

	LOG_HEXDUMP_DBG(sid_send_work->msg.data, sid_send_work->msg.size, "sending message");

	struct sid_msg_desc desc = {
		.type = sid_send_work->type,
		.link_type = SID_LINK_TYPE_ANY,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

	if (desc.type == SID_MSG_TYPE_RESPONSE) {
		desc.id = sid_send_work->resp_id;
	}

	sid_error_t sid_ret = sid_put_msg(sid_app_ctx->sidewalk_handle, &sid_send_work->msg, &desc);

	if (SID_ERROR_NONE != sid_ret) {
		LOG_ERR("failed sending message err:%d", (int) sid_ret);
		return;
	}
	LOG_INF("queued data message id:%u", desc.id);
	shell_info(sid_send_work->shell, "sidewalk cli: command send");
}

static int cmd_send(const struct shell *shell, size_t argc, char **argv)
{
	enum sid_msg_type type;
	uint8_t message_raw_data[MAX_MESSAGE_SIZE_BYTES];

	if (strlen(argv[2]) != 1) {
		shell_error(shell, "send type incorrect");
		return CMD_RETURN_ARGUMENT_INVALID;
	}

	switch (argv[2][0]) {
	case '0': type = SID_MSG_TYPE_GET; break;
	case '1': type = SID_MSG_TYPE_SET; break;
	case '2': type = SID_MSG_TYPE_NOTIFY; break;
	case '3': type = SID_MSG_TYPE_RESPONSE; break;
	default:
		shell_error(shell, "send type incorrect");
		return CMD_RETURN_ARGUMENT_INVALID;
	}

	StrToHexRet ret = convert_hex_str_to_data(message_raw_data, sizeof(message_raw_data), argv[1]);

	if (ret.result == Err) {
		switch (ret.val_or_err.err) {
		case OUT_TOO_SMALL:
			shell_error(shell,
				    "Payload too big to send! max size of payload is %d bytes",
				    sizeof(message_raw_data));
			return CMD_RETURN_ARGUMENT_INVALID;
		case ARG_NOT_HEX:
			shell_error(shell,
				    "Payload is not hex string\nexample: $ sidewalk send deadbeefCAFE 2");
			return CMD_RETURN_ARGUMENT_INVALID;
		case ARG_EMPTY:
			shell_error(shell,
				    "Payload is empty\nexample: $ sidewalk send deadbeefCAFE 2");
			return CMD_RETURN_ARGUMENT_INVALID;
		case ARG_NOT_ALIGNED:
			shell_error(shell,
				    "Payload is not aligned to full bytes\nexample: $ sidewalk send deadbeefCAFE 2");
			return CMD_RETURN_ARGUMENT_INVALID;
		default:
			shell_error(shell,
				    "Invalid payload");
			return CMD_RETURN_HELP;
		}
	}

	if(k_work_busy_get(&send_work.work) != 0){
		shell_error(shell, "Can not execute send command, previous send has not completed yet");
		return CMD_RETURN_NOT_EXECUTED;
	}

	send_work.msg = (struct sid_msg){ .data = message_raw_data, .size = ret.val_or_err.val};
	send_work.shell = shell;
	send_work.type = type;
	send_work.resp_id = sidewalk_messages.resp_id;
	k_work_init(&send_work.work, cmd_send_work);
	k_work_submit_to_queue(&sid_api_work_q, &send_work.work);
	
	return CMD_RETURN_OK;
}

static int cmd_report(const struct shell *shell, size_t argc, char **argv)
{
	const char *state_repo[] = {
		"STATE_INIT",
		"STATE_SIDEWALK_READY",
		"STATE_SIDEWALK_NOT_READY",
		"STATE_SIDEWALK_SECURE_CONNECTION",
		"STATE_PAL_INIT_ERROR",
		"STATE_PAL_INIT_ERROR"
	};
	const char *state;

	if (sid_app_ctx->state <= STATE_LIB_INIT_ERROR) {
		state = state_repo[sid_app_ctx->state];
	} else {
		state = "invalid";
	}

	bool in_line = (argc == 2 && strcmp("--oneline", argv[1]) == 0);

	JSON_DICT("SIDEWALK_CLI", in_line, {
		JSON_VAL_STR("state", state, JSON_NEXT);
		JSON_VAL(
			"registered",
			(CLI_status->detail.registration_status == SID_STATUS_REGISTERED), JSON_NEXT);
		JSON_VAL(
			"time_synced",
			(CLI_status->detail.time_sync_status == SID_STATUS_TIME_SYNCED), JSON_NEXT);
		JSON_VAL("link_up", (CLI_status->detail.link_status_mask), JSON_NEXT);
		JSON_VAL_DICT(
			"link_modes",
		{
			JSON_VAL("ble",
				 CLI_status->detail.supported_link_modes[SID_LINK_TYPE_1_IDX],
				 JSON_NEXT);
			JSON_VAL("fsk",
				 CLI_status->detail.supported_link_modes[SID_LINK_TYPE_2_IDX],
				 JSON_NEXT);
			JSON_VAL("lora",
				 CLI_status->detail.supported_link_modes[SID_LINK_TYPE_3_IDX],
				 JSON_LAST);
		},
			JSON_NEXT);
		JSON_VAL("tx_successfull", sidewalk_messages.tx_successfull, JSON_NEXT);
		JSON_VAL("tx_failed", sidewalk_messages.tx_failed, JSON_NEXT);
		JSON_VAL("rx_successfull", sidewalk_messages.rx_successfull, JSON_NEXT);
		JSON_VAL("response_id", sidewalk_messages.resp_id, JSON_LAST);
	});
	return CMD_RETURN_OK;
}

void cmd_print_version(const struct shell *shell, size_t argc, char **argv)
{
	bool in_line = (argc == 2 && strcmp("--oneline", argv[1]) == 0);

	JSON_DICT("COMPONENTS_VERSION", in_line, {
		JSON_VAL_STR("sidewalk_fork_point", sidewalk_version_common_commit, JSON_NEXT);
		JSON_VAL_STR("build_time", build_time_stamp, JSON_NEXT);
		JSON_VAL_DICT(
			"modules",
		{
			JSON_VAL_STR_ENUMERATE(sidewalk_version_component_name, sidewalk_version_component,
					       sidewalk_version_component_count, JSON_LAST);
		},
			JSON_LAST);
	});
}

static int cmd_factory_reset(const struct shell *shell, size_t argc, char **argv)
{
	sid_error_t ret = sid_set_factory_reset(sid_app_ctx->sidewalk_handle);

	if (SID_ERROR_NONE != ret) {
		switch (ret) {
		case SID_ERROR_INVALID_ARGS:
			shell_error(shell, "Sidewalk not initialized, can not perform factory reset");
			break;
		default:
			shell_error(shell, "Notification of factory reset to sid api failed with unhandled error %d",
				    ret);
			break;
		}
		return CMD_RETURN_NOT_EXECUTED;
	} else {
		shell_info(shell, "sidewalk cli: factory reset request registered");
	}
	return CMD_RETURN_OK;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_button,
	SHELL_CMD_ARG(short, NULL, "{1,2,3,4}", cmd_button_press_short, 2, 0),
	SHELL_CMD_ARG(long, NULL, "{1,2,3,4}", cmd_button_press_long, 2, 0),
SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_services,
	SHELL_CMD_ARG(press_button, &sub_button, "{1,2,3,4}", cmd_button_press_short, 2, 0),
	SHELL_CMD_ARG(set_response_id, NULL, "set ID of a next send message with type response", cmd_set_response_id, 2,
		      0),
	SHELL_CMD_ARG(send, NULL, "<hex payload> <type [0-3]>\n" \
		      "\thex payload have to have even number of hex characters, type id: 0 - get, 1 - set, 2 - notify, 3 - response",
		      cmd_send, 3, 0),
	SHELL_CMD_ARG(report, NULL, "[--oneline] get state of the application", cmd_report, 1, 1),
	SHELL_CMD_ARG(version, NULL, "[--oneline] print version of sidewalk and its components", cmd_print_version, 1,
		      1),
	SHELL_CMD(factory_reset, NULL, "perform factory reset of Sidewalk application", cmd_factory_reset),
	SHELL_SUBCMD_SET_END);

// command, subcommands, help, handler
SHELL_CMD_REGISTER(sidewalk, &sub_services, "sidewalk testing CLI", NULL);
