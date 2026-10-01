/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef PRINT_SHELL_H
#define PRINT_SHELL_H

#include <zephyr/shell/shell.h>

#define CMD_PRINT_TXID_DESCRIPTION "Print transmit UUID (TXID)"
#define CMD_PRINT_TXID_ARG_REQUIRED 1
#define CMD_PRINT_TXID_ARG_OPTIONAL 0

#define CMD_PRINT_GET_CAP_DESCRIPTION "<1|2|3>\n1 = capabilities, 2 = configuration, 3 = both"
#define CMD_PRINT_GET_CAP_ARG_REQUIRED 2
#define CMD_PRINT_GET_CAP_ARG_OPTIONAL 0

#define CMD_PRINT_CLEAR_CAP_DESCRIPTION "<1|2|3>\n1 = capabilities, 2 = configuration, 3 = both"
#define CMD_PRINT_CLEAR_CAP_ARG_REQUIRED 2
#define CMD_PRINT_CLEAR_CAP_ARG_OPTIONAL 0

#define CMD_PRINT_METRICS_DESCRIPTION "<category>\nMetrics category id (0x00-0x12, 0x36-0x39)"
#define CMD_PRINT_METRICS_ARG_REQUIRED 2
#define CMD_PRINT_METRICS_ARG_OPTIONAL 0

#define CMD_PRINT_CLEAR_METRICS_DESCRIPTION "<category> [metric_id]"
#define CMD_PRINT_CLEAR_METRICS_ARG_REQUIRED 2
#define CMD_PRINT_CLEAR_METRICS_ARG_OPTIONAL 1

#define CMD_PRINT_MFG_DESCRIPTION "Print manufacturing store values"
#define CMD_PRINT_MFG_ARG_REQUIRED 1
#define CMD_PRINT_MFG_ARG_OPTIONAL 0

#define CMD_PRINT_FWVERSION_DESCRIPTION "Print firmware / SDK version"
#define CMD_PRINT_FWVERSION_ARG_REQUIRED 1
#define CMD_PRINT_FWVERSION_ARG_OPTIONAL 0

int cmd_print_txid(const struct shell *shell, int32_t argc, const char **argv);
int cmd_print_get_cap(const struct shell *shell, int32_t argc, const char **argv);
int cmd_print_clear_cap(const struct shell *shell, int32_t argc, const char **argv);
int cmd_print_metrics(const struct shell *shell, int32_t argc, const char **argv);
int cmd_print_clear_metrics(const struct shell *shell, int32_t argc, const char **argv);
int cmd_print_mfg(const struct shell *shell, int32_t argc, const char **argv);
int cmd_print_fwversion(const struct shell *shell, int32_t argc, const char **argv);

#endif /* PRINT_SHELL_H */
