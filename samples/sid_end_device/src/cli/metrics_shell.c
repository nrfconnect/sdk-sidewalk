/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>
#include <sys/errno.h>

#include <zephyr/shell/shell.h>

#include <sid_error.h>

#include <cli/metrics_shell.h>
#include <cli/sid_metrics_cli.h>

#define CHECK_ARGUMENT_COUNT(argc, required, optional)                                             \
	if ((argc < required) || (argc > (required + optional))) {                                 \
		return -EINVAL;                                                                    \
	}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_metrics_services,
	SHELL_CMD_ARG(send, NULL, CMD_METRICS_SEND_DESCRIPTION, cmd_metrics_send,
		      CMD_METRICS_SEND_ARG_REQUIRED, CMD_METRICS_SEND_ARG_OPTIONAL),
	SHELL_CMD_ARG(set, NULL, CMD_METRICS_SET_DESCRIPTION, cmd_metrics_set,
		      CMD_METRICS_SET_ARG_REQUIRED, CMD_METRICS_SET_ARG_OPTIONAL),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(metrics, &sub_metrics_services, "Sidewalk metrics CLI (QA)", NULL);

int cmd_metrics_send(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_METRICS_SEND_ARG_REQUIRED, CMD_METRICS_SEND_ARG_OPTIONAL);

	if (sid_metrics_core_cli_trigger_report() != SID_ERROR_NONE) {
		shell_error(shell, "metrics send failed");
		return -EIO;
	}

	return 0;
}

int cmd_metrics_set(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_METRICS_SET_ARG_REQUIRED, CMD_METRICS_SET_ARG_OPTIONAL);

	enum sid_metrics_category_ids category =
		(enum sid_metrics_category_ids)strtoul(argv[1], NULL, 0);
	uint8_t metrics_id = (uint8_t)strtoul(argv[2], NULL, 0);
	uint32_t value = (uint32_t)strtoul(argv[3], NULL, 0);

	if (sid_metrics_core_cli_set_metrics(category, metrics_id, value) != SID_ERROR_NONE) {
		shell_error(shell, "metrics set failed");
		return -EIO;
	}

	return 0;
}
