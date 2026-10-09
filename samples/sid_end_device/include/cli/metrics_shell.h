/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef METRICS_SHELL_H
#define METRICS_SHELL_H

#include <zephyr/shell/shell.h>

#define CMD_METRICS_SEND_DESCRIPTION "Trigger metrics report to cloud"
#define CMD_METRICS_SEND_ARG_REQUIRED 1
#define CMD_METRICS_SEND_ARG_OPTIONAL 0

#define CMD_METRICS_SET_DESCRIPTION                                                                \
	"<category_id> <metric_id> <value>\n"                                                      \
	"Set metrics_fwk_v2 value (QA boundary / preconditions)."
#define CMD_METRICS_SET_ARG_REQUIRED 4
#define CMD_METRICS_SET_ARG_OPTIONAL 0

int cmd_metrics_send(const struct shell *shell, int32_t argc, const char **argv);
int cmd_metrics_set(const struct shell *shell, int32_t argc, const char **argv);

#endif /* METRICS_SHELL_H */
