/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <ace/cli.h>
#include <sid_asd_cli.h>
#include <sidewalk_dfu/nordic_dfu.h>

#include <cli/app_shell.h>
#include <cli/nordic_dfu_shell.h>

LOG_MODULE_REGISTER(nordic_dfu_shell, CONFIG_SIDEWALK_LOG_LEVEL);

#define CMD_NORDIC_DFU_DESCRIPTION                                                                 \
	"Start BLE SMP service for DFU\n"                                                          \
	"Make sure that sidewalk is deinitialized before use, or the DFU may fail during transfer!"

static ace_status_t cmd_nordic_dfu(int32_t argc, const char **argv)
{
	int err;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	LOG_INF("Entering into DFU mode");

	err = nordic_dfu_ble_start();
	if (err) {
		LOG_ERR("dfu start err %d", err);
		return ACE_STATUS_CLI_FUNC_ERROR;
	}

	return ACE_STATUS_OK;
}

static SID_CLI_REGISTER_COMMAND(nordic_dfu_cmds){
	SID_CLI_DEFINE_COMMAND(nordic_dfu, NULL, CMD_NORDIC_DFU_DESCRIPTION, cmd_nordic_dfu),
	SID_CLI_SUBCMD_SET_END,
};

int nordic_dfu_shell_init(void)
{
	if (SID_CLI_REGISTER_SUB_COMMAND_SET(nordic_dfu_cmds) != SID_ERROR_NONE) {
		return -ENOMEM;
	}

	return 0;
}

APP_SHELL_FORWARD_CMD_REGISTER(nordic_dfu, CMD_NORDIC_DFU_DESCRIPTION);
