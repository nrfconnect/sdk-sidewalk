/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_DUT_SHELL_H
#define SID_DUT_SHELL_H

#include <stddef.h>
#include <zephyr/shell/shell.h>
#include <sid_qa.h>

/**
 * @brief Register Nordic-specific commands in the Amazon SDK CLI and start accepting shell input.
 *
 * Must be called on the Sidewalk QA thread after sid_qa_init() and sid_config_cli_init().
 *
 * @return 0 on success, negative error code otherwise.
 */
int app_shell_init(void);

/**
 * @brief Get the Sidewalk QA platform interface.
 *
 * The interface blocks the Sidewalk QA thread in sid_qa_process() until a Sidewalk event is queued,
 * executing the Amazon SDK CLI commands received from the Zephyr shell in the meantime.
 * Commands are not executed during a power measurement.
 *
 * @return Pointer to the interface to be passed to sid_qa_set_config().
 */
struct sid_qa_pwr_meas_if *app_shell_qa_ifc(void);

/**
 * @brief Zephyr shell handler forwarding the command line to the Amazon SDK CLI.
 *
 * The command is executed on the Sidewalk QA thread and the handler waits for the result.
 * Register root commands with SHELL_OPT_ARG_RAW optional arguments, so that the
 * command line is forwarded unchanged.
 */
int app_shell_cmd_forward(const struct shell *sh, size_t argc, char **argv);

/**
 * @brief Register a root command forwarded to the Amazon SDK CLI.
 */
#define APP_SHELL_FORWARD_CMD_REGISTER(name, help)                                                 \
	SHELL_CMD_ARG_REGISTER(name, NULL, help, app_shell_cmd_forward, 1, SHELL_OPT_ARG_RAW)

#endif /* SID_DUT_SHELL_H */
