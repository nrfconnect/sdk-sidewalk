#include "kernel.h"
#include "sys/atomic.h"
#include "sys/atomic_builtin.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/shell/shell.h>
#include <logging/log.h>

#include <sid_api.h>
#include <sidewalk_thread.h>

#include <json_printer.h>
#include <sid_shell.h>

LOG_MODULE_REGISTER(sid_cli, CONFIG_SIDEWALK_LOG_LEVEL);

#define CMD_RETURN_OK 0
#define CMD_RETURN_HELP 1
#define CMD_RETURN_ARGUMENT_INVALID -EINVAL
#define CMD_RETURN_NOT_EXECUTED -ENOEXEC

#define BUTTON_ID_TO_BIT(button_id) ((button_id) - 1)
#define BOOL_TO_STR(val) (val) ? "true" : "false"

typedef enum { Ok, Err } result_t;

#define Result(value_t, error_e) struct {			\
		result_t result;				\
		union { value_t val; error_e err; } val_or_err;	\
}

#define Result_Ok(value) { .result = Ok, .val_or_err.val = value }
#define Result_Err(error_code) { .result = Err, .val_or_err.err = error_code }

typedef enum str_to_hex_err {
	OUT_TOO_SMALL, ARG_NOT_HEX, ARG_EMPTY
} str_to_hex_err;

// TODO: optimize the stack value 512 is to small to send a message.
#define MY_STACK_SIZE 5120
#define SHELL_WORK_PRIO 5

K_THREAD_STACK_DEFINE(shell_work_stack, MY_STACK_SIZE);

static struct k_work_q shell_work_queue;

struct messages_stats {
	uint32_t tx_successfull;
	uint32_t tx_failed;
	uint32_t rx_successfull;
};

static const struct sid_status dummy = {
	.state = SID_STATE_SECURE_CHANNEL_READY + 1,
	.detail = { .link_status_mask = 0,
		    .registration_status = SID_STATUS_NOT_REGISTERED,
		    .time_sync_status = SID_STATUS_NO_TIME }

};

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

void CLI_register_message_received()
{
	sidewalk_messages.rx_successfull++;
}

void CLI_init(app_context_t *ctx)
{
	sid_app_ctx = ctx;

	static atomic_t queue_init = ATOMIC_INIT(0);

	if (atomic_and(&queue_init, 1) == 0) {
		k_work_queue_init(&shell_work_queue);

		k_work_queue_start(&shell_work_queue, shell_work_stack,
				   K_THREAD_STACK_SIZEOF(shell_work_stack), SHELL_WORK_PRIO,
				   NULL);
		k_thread_name_set(k_work_queue_thread_get(&shell_work_queue), "CLI work queue");
	}
}

void CLI_register_sid_status(const struct sid_status *status)
{
	CLI_status = status;
}

static int parse_ul(const char *str, unsigned long *result)
{
	char *end;
	unsigned long val;

	val = strtoul(str, &end, 0);

	if (*str == '\0' || *end != '\0') {
		return -EINVAL;
	}

	*result = val;
	return 0;
}

void sidewalk_button_pressed(uint32_t button_bit);

// --------------------------------------------
// start cmd handlers
// --------------------------------------------
static int cmd_press_button(const struct shell *shell, size_t argc,
			    char **argv)
{
	if (argc != 2) {
		shell_error(shell, "select button to press:\nexample: sidewalk press_button 4");
		return CMD_RETURN_HELP;
	}

	unsigned long button_id;

	if (parse_ul(argv[1], &button_id) != 0) {
		return CMD_RETURN_ARGUMENT_INVALID;
	}
	sidewalk_button_pressed(BUTTON_ID_TO_BIT(button_id));
	shell_info(shell, "sidewalk cli: button pressed");
	return CMD_RETURN_OK;
}

typedef struct send_message_work {
	struct k_work work;
	struct sid_msg msg;
	uint16_t id;
	struct k_sem sem;
} send_message_work_t;

void sidewalk_send_message_work(struct k_work *work)
{

	send_message_work_t *message = CONTAINER_OF(work, send_message_work_t, work);

	if (STATE_SIDEWALK_READY == sid_app_ctx->state ||
	    STATE_SIDEWALK_SECURE_CONNECTION == sid_app_ctx->state) {
		LOG_HEXDUMP_DBG(message->msg.data, message->msg.size, "sending message");

		struct sid_msg_desc desc;
		desc.type = SID_MSG_TYPE_NOTIFY;
		desc.link_type = SID_LINK_TYPE_ANY;
		desc.link_mode = SID_LINK_MODE_CLOUD;
		desc.id = message->id;

		sid_error_t ret = sid_put_msg(sid_app_ctx->sidewalk_handle, &(message->msg), &desc);
		if (SID_ERROR_NONE != ret) {
			LOG_ERR("failed sending message err:%d", (int) ret);
		} else {
			LOG_DBG("queued data message id:%u", desc.id);
		}
	} else {
		LOG_ERR("sidewalk is not ready yet!");
	}
	k_sem_give(&message->sem);
}

static bool is_hex_digit(char hex)
{
	return (hex >= '0' && hex <= '9') || (hex >= 'a' && hex <= 'f') || (hex >= 'A' && hex <= 'F');
}

typedef Result(size_t, str_to_hex_err) StrToHexRet;

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

	if (out_limit < in_size / 2) {
		return (StrToHexRet) Result_Err(OUT_TOO_SMALL);
	}

	uint8_t payload_size = 0;

	for (; *working_ptr != '\0'; working_ptr = working_ptr + 2) {
		if (is_hex_digit(*working_ptr) && is_hex_digit(*(working_ptr + 1))) {
			char byte[2] = { *working_ptr, *(working_ptr + 1) };
			out[payload_size] = strtol(byte, NULL, 16);
			payload_size++;
		} else {
			return (StrToHexRet) Result_Err(ARG_NOT_HEX);
		}
	}
	return (StrToHexRet) Result_Ok(payload_size);
}

static int cmd_send(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 3) {
		shell_error(shell, "Add payload to send in hex form\nexample: $ sidewalk send deadbeefCAFE 123");
		return CMD_RETURN_HELP;
	}
	send_message_work_t message;

	if (strlen(argv[1]) % 2) {
		shell_error(shell,
			    "Payload have to have even number of hex characters\nexample: $ sidewalk send deadbeefCAFE 123");
		return CMD_RETURN_HELP;
	}

	int32_t id_raw = atoi(argv[2]);

	if (id_raw == 0 && argv[2][0] != '0') {
		shell_error(shell,
			    "Invalid sequence id of message valid range [0:16383]\nexample: $ sidewalk send deadbeefCAFE 123");
		return CMD_RETURN_HELP;
	}
	message.id = id_raw % 0x3FFF;

	k_work_init(&message.work, sidewalk_send_message_work);
	k_sem_init(&message.sem, 0, 1);
	uint8_t payload[512];
	StrToHexRet ret = convert_hex_str_to_data(payload, sizeof(payload), argv[1]);

	if (ret.result == Err) {
		switch (ret.val_or_err.err) {
		case OUT_TOO_SMALL:
			shell_error(shell,
				    "Payload too big to send! max size of payload is %d bytes", sizeof(payload));
			return CMD_RETURN_HELP;
		case ARG_NOT_HEX:
			shell_error(shell,
				    "Payload is not hex string\nPayload have to have even number of hex characters\nexample: $ sidewalk send deadbeefCAFE 123");
			return CMD_RETURN_HELP;
		case ARG_EMPTY:
			shell_error(shell,
				    "Payload is empty\nPayload have to have even number of hex characters\nexample: $ sidewalk send deadbeefCAFE 123");
			return CMD_RETURN_HELP;
		default:
			shell_error(shell,
				    "Invalid payload");
			return CMD_RETURN_HELP;
		}
	}
	size_t payload_size = ret.val_or_err.val;

	if (payload_size == 0) {
		shell_error(shell,
			    "Payload have to consist only from hex characters\nexample: $ sidewalk send deadbeefCAFE 123");
		return CMD_RETURN_HELP;
	}

	message.msg = (struct sid_msg){ .data = payload, .size = payload_size };
	k_work_submit_to_queue(&shell_work_queue, &message.work);
	k_sem_take(&message.sem, K_FOREVER);
	shell_info(shell, "sidewalk cli: command send");
	return CMD_RETURN_OK;
}

static int cmd_report(const struct shell *shell, size_t argc, char **argv)
{
	if (argc > 2) {
		shell_error(shell,
			    "This command do not support more than 1 argument\nexample: sidewalk report [--oneline]");
		return CMD_RETURN_HELP;
	}

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
		JSON_VAL("rx_successfull", sidewalk_messages.rx_successfull, JSON_LAST);
	});
	return CMD_RETURN_OK;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_services,
	SHELL_CMD_ARG(press_button, NULL, "{1,2,3,4}", cmd_press_button, 2, 0),
	SHELL_CMD_ARG(send, NULL,
		      "<hex payload> <sequence id>\n\thex payload have to have even number of hex characters, sequence id is represented in decimal",
		      cmd_send, 3, 0),
	SHELL_CMD_ARG(report, NULL, "[--oneline] get state of the application", cmd_report, 1, 1),
	SHELL_SUBCMD_SET_END);

// command, subcommands, help, handler
SHELL_CMD_REGISTER(sidewalk, &sub_services, "sidewalk testing CLI", NULL);
