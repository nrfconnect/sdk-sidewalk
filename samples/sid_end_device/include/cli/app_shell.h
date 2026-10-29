/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_DUT_SHELL_H
#define SID_DUT_SHELL_H

#include <sid_qa.h>
#include <sid_api.h>

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

#endif /* SID_DUT_SHELL_H */
