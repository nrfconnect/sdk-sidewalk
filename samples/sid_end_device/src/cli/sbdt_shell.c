/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <cli/sbdt_shell.h>
#include <cli/sbdt_shell_events.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <sid_hal_memory_ifc.h>
#include <sid_bulk_data_transfer_api.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/shell/shell.h>
#include <json_printer/sidTypes2str.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sid_sbdt_cli, CONFIG_SIDEWALK_LOG_LEVEL);

struct cmd_sbdt_cfg_args {
	bool print;
	bool reset;
	struct {
		bool set;
		uint16_t val;
	} fd;
	struct {
		bool set;
		uint8_t val;
	} fs;
	struct {
		bool set;
		uint8_t val;
	} tr;
	struct {
		bool set;
		enum sid_bulk_data_transfer_reject_reason val;
	} trs;
	struct {
		bool set;
		uint16_t val;
	} br;
};

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_services,
	SHELL_CMD_ARG(init, NULL, CMD_SBDT_INIT_DESCRIPTION, cmd_sbdt_init,
		      CMD_SBDT_INIT_ARG_REQUIRED, CMD_SBDT_INIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(deinit, NULL, CMD_SBDT_DEINIT_DESCRIPTION, cmd_sbdt_deinit,
		      CMD_SBDT_DEINIT_ARG_REQUIRED, CMD_SBDT_DEINIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(cancel, NULL, CMD_SBDT_CANCEL_DESCRIPTION, cmd_sbdt_cancel,
		      CMD_SBDT_CANCEL_ARG_REQUIRED, CMD_SBDT_CANCEL_ARG_OPTIONAL),
	SHELL_CMD_ARG(cfg, NULL, CMD_SBDT_CFG_DESCRIPTION, cmd_sbdt_cfg, CMD_SBDT_CFG_ARG_REQUIRED,
		      CMD_SBDT_CFG_ARG_OPTIONAL),
	SHELL_CMD_ARG(print, NULL, CMD_SBDT_STATS_DESCRIPTION, cmd_sbdt_print,
		      CMD_SBDT_STATS_ARG_REQUIRED, CMD_SBDT_STATS_ARG_OPTIONAL),
	SHELL_CMD_ARG(stats, NULL, CMD_SBDT_STATS_DESCRIPTION, cmd_sbdt_stats,
		      CMD_SBDT_STATS_ARG_REQUIRED, CMD_SBDT_STATS_ARG_OPTIONAL),
	SHELL_CMD_ARG(params, NULL, CMD_SBDT_PARAMS_DESCRIPTION, cmd_sbdt_params,
		      CMD_SBDT_PARAMS_ARG_REQUIRED, CMD_SBDT_PARAMS_ARG_OPTIONAL),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sbdt, &sub_services, "Sidewalk bulk data transfer CLI", NULL);

static struct sbdt_context sbdt_context = {
        .ft_callbacks = {
                .context = &sbdt_context,
                .on_transfer_request = on_sbdt_transfer_request,
                .on_data_received = on_sbdt_data_received,
                .on_finalize_request = on_sbdt_finalize_request,
                .on_cancel_request = on_sbdt_cancel_request,
                .on_error = on_sbdt_error,
                .on_release_scratch_buffer = on_release_scratch_buffer,
                },
        .transfer_request_action = 0,
        .transfer_request_reject_reason = 0x1,
        .finalize_response_action = 0,
        .finalize_response_delay_s = 1,
        .release_buffer_delay_ms = 30,
	.started_transfer = false,
};

int cmd_sbdt_init(const struct shell *shell, int32_t argc, const char **argv)
{
	sidewalk_event_send(sbdt_event_init, &sbdt_context.ft_callbacks, NULL);
	return 0;
}

int cmd_sbdt_deinit(const struct shell *shell, int32_t argc, const char **argv)
{
	sidewalk_event_send(sbdt_event_deinit, NULL, NULL);
	return 0;
}

static bool validate_reason(enum sid_bulk_data_transfer_reject_reason reason)
{
	switch (reason) {
	case SID_BULK_DATA_TRANSFER_REJECT_REASON_NONE:
	case SID_BULK_DATA_TRANSFER_REJECT_REASON_GENERIC:
	case SID_BULK_DATA_TRANSFER_REJECT_REASON_FILE_TOO_BIG:
	case SID_BULK_DATA_TRANSFER_REJECT_REASON_NO_SPACE:
	case SID_BULK_DATA_TRANSFER_REJECT_REASON_LOW_BATTERY:
	case SID_BULK_DATA_TRANSFER_REJECT_REASON_FILE_VERIFICATION_FAILED:
	case SID_BULK_DATA_TRANSFER_REJECT_REASON_FILE_ALREADY_EXISTS:
	case SID_BULK_DATA_TRANSFER_REJECT_REASON_INVALID_FRAGMENT_SIZE:
		return true;
	}
	return false;
}

int cmd_sbdt_cancel(const struct shell *shell, int32_t argc, const char **argv)
{
	struct sbdt_cancel_ctx *ctx = sid_hal_malloc(sizeof(struct sbdt_cancel_ctx));
	if (ctx == NULL) {
		return -ENOMEM;
	}
	char *ref = NULL;

	ctx->file_id = (int)strtol(argv[1], &ref, 0);
	if (ref == NULL || ref == argv[1] || !IN_RANGE(ctx->file_id, 0, INT32_MAX)) {
		shell_error(shell, "invalid file_id value");
		return -EINVAL;
	}
	ref = NULL;

	ctx->reason = (enum sid_bulk_data_transfer_reject_reason)strtol(argv[2], &ref, 0);
	if (ref == NULL || ref == argv[2] || !validate_reason(ctx->reason)) {
		shell_error(shell, "invalid reason value");
		return -EINVAL;
	}

	sidewalk_event_send(sbdt_event_cancel, ctx, sid_hal_free);
	return 0;
}

static void sbdt_print_config()
{
	LOG_INF("DATA_TRANSFER_ACTION: %d (%s)\n"
		"DATA_TRANSFER_REJECT_REASON: %d (%s)\n"
		"TRANSFER_FINAL_STATUS: %d (%s)\n"
		"TRANSFER_FINAL_RESP_DELAY_S: %d\n"
		"RELEASE_BUFFER_DELAY_MS: %d\n",
		sbdt_context.transfer_request_action,
		sbdt_context.transfer_request_action ? "REJECT" : "ACCEPT",
		sbdt_context.transfer_request_reject_reason,
		SID_BULK_DATA_TRANSFER_REJECT_REASON_STR(
			sbdt_context.transfer_request_reject_reason),
		sbdt_context.finalize_response_action,
		sbdt_context.finalize_response_action ? "FAILURE" : "SUCCESS",
		sbdt_context.finalize_response_delay_s, sbdt_context.release_buffer_delay_ms);
}

static void sbdt_reset_cfg()
{
	sbdt_context.transfer_request_action = 0;
	sbdt_context.transfer_request_reject_reason = 0x1;
	sbdt_context.finalize_response_action = 0;
	sbdt_context.finalize_response_delay_s = 1;
	sbdt_context.release_buffer_delay_ms = 30;
}

static bool parse_sbdt_cfg_args(const struct shell *shell, int32_t argc, const char **argv,
				struct cmd_sbdt_cfg_args *out)
{
	for (int opt = 1; opt < argc; opt++) {
		if (strcmp("-p", argv[opt]) == 0) {
			out->print = true;
			continue;
		}
		if (strcmp("-r", argv[opt]) == 0) {
			out->reset = true;
			continue;
		}
		if (strcmp("-fd", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-fd need a value");
				return false;
			}
			char *ref = NULL;
			out->fd.val = strtol(argv[opt], &ref, 0);
			if (ref == NULL || ref == argv[opt] ||
			    !IN_RANGE(out->fd.val, 0, UINT16_MAX)) {
				shell_error(shell, "failed to parse argument for -fd option");
				return false;
			}
			out->fd.set = true;

			continue;
		}
		if (strcmp("-fs", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-fs need a value");
				return false;
			}
			char *ref = NULL;
			out->fs.val = strtol(argv[opt], &ref, 0);
			if (ref == NULL || ref == argv[opt] || !IN_RANGE(out->fs.val, 0, 1)) {
				shell_error(shell, "failed to parse argument for -fs option");
				return false;
			}
			out->fs.set = true;
			continue;
		}
		if (strcmp("-tr", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-tr need a value");
				return false;
			}
			char *ref = NULL;
			out->tr.val = strtol(argv[opt], &ref, 0);
			if (ref == NULL || ref == argv[opt] || !IN_RANGE(out->tr.val, 0, 1)) {
				shell_error(shell, "failed to parse argument for -tr option");
				return false;
			}
			out->tr.set = true;
			continue;
		}
		if (strcmp("-trs", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-trs need a value");
				return false;
			}
			char *ref = NULL;
			out->trs.val = strtol(argv[opt], &ref, 0);
			if (ref == NULL || ref == argv[opt] || !validate_reason(out->trs.val)) {
				shell_error(shell, "failed to parse argument for -trs option");
				return false;
			}
			out->trs.set = true;
			continue;
		}
		if (strcmp("-br", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-br need a value");
				return false;
			}
			char *ref = NULL;
			out->br.val = strtol(argv[opt], &ref, 0);
			if (ref == NULL || ref == argv[opt] ||
			    !IN_RANGE(out->br.val, 0, UINT16_MAX)) {
				shell_error(shell, "failed to parse argument for -br option");
				return false;
			}
			out->br.set = true;
			continue;
		}
	}

	if ((out->reset) &&
	    (out->fd.set || out->fs.set || out->tr.set || out->trs.set || out->br.set)) {
		shell_error(shell, "-r flag can not be combined with any other flag");
		return false;
	}

	return true;
}

int cmd_sbdt_cfg(const struct shell *shell, int32_t argc, const char **argv)
{
	struct cmd_sbdt_cfg_args arguments = { 0 };

	if (argc == 1) {
		arguments.print = true;
	}

	if (parse_sbdt_cfg_args(shell, argc, argv, &arguments) == false) {
		return -EINVAL;
	}

	if (arguments.reset) {
		arguments.print = true;
		sbdt_reset_cfg();
	}
	if (arguments.fd.set) {
		sbdt_context.finalize_response_delay_s = arguments.fd.val;
	}
	if (arguments.fs.set) {
		sbdt_context.finalize_response_action = arguments.fs.val;
	}
	if (arguments.tr.set) {
		sbdt_context.transfer_request_action = arguments.tr.val;
	}
	if (arguments.trs.set) {
		sbdt_context.transfer_request_reject_reason = arguments.trs.val;
	}
	if (arguments.br.set) {
		sbdt_context.release_buffer_delay_ms = arguments.br.val;
	}
	if (arguments.print) {
		sbdt_print_config();
	}
	return 0;
}

static const uint16_t valid_block_sizes[] = { 1024, 2048, 3072, 4096, 5120, 6144, 7168, 8192 };

int cmd_sbdt_print(const struct shell *shell, int32_t argc, const char **argv)
{
	for (size_t i = 0; i < ARRAY_SIZE(valid_block_sizes); i++) {
		LOG_INF("BLOCK_SIZE: %d, MIN_SCRATCH_SPACE: %d", valid_block_sizes[i],
			sid_bulk_data_transfer_compute_min_scratch_buffer_size(
				valid_block_sizes[i]));
	}
	return 0;
}

int cmd_sbdt_stats(const struct shell *shell, int32_t argc, const char **argv)
{
	int *file_id = sid_hal_malloc(sizeof(int));
	if (file_id == NULL) {
		return -ENOMEM;
	}
	*file_id = 0;
	if (argc == 2) {
		char *ref = NULL;
		*file_id = strtol(argv[1], &ref, 0);
		if (ref == NULL || ref == argv[1]) {
			shell_error(shell, "failed to parse argument");
			sid_hal_free(file_id);
			return -EINVAL;
		}
	}
	sidewalk_event_send(sbdt_event_print_stats, file_id, sid_hal_free);
	return 0;
}

int cmd_sbdt_params(const struct shell *shell, int32_t argc, const char **argv)
{
	int *file_id = sid_hal_malloc(sizeof(int));
	if (file_id == NULL) {
		return -ENOMEM;
	}
	*file_id = 0;
	if (argc == 2) {
		char *ref = NULL;
		*file_id = strtol(argv[1], &ref, 0);
		if (ref == NULL || ref == argv[1]) {
			shell_error(shell, "failed to parse argument");
			sid_hal_free(file_id);
			return -EINVAL;
		}
	}
	sidewalk_event_send(sbdt_event_print_params, file_id, sid_hal_free);
	return 0;
}
