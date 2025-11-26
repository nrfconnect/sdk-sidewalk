/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
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
#include <sid_hal_memory_ifc.h>

#include <cli/location_shell.h>
#include <cli/location_shell_events.h>
#include <sidewalk.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(location_shell, CONFIG_SIDEWALK_LOG_LEVEL);

#define CHECK_ARGUMENT_COUNT(argc, required, optional)                                             \
	if ((argc < required) || (argc > (required + optional))) {                                 \
		return -EINVAL;                                                                    \
	}

/* Location Subcommands */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_location_services,
	SHELL_CMD_ARG(init, NULL, CMD_LOCATION_INIT_DESCRIPTION, cmd_location_init,
		      CMD_LOCATION_INIT_ARG_REQUIRED, CMD_LOCATION_INIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(deinit, NULL, CMD_LOCATION_DEINIT_DESCRIPTION, cmd_location_deinit,
		      CMD_LOCATION_DEINIT_ARG_REQUIRED, CMD_LOCATION_DEINIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(send, NULL, CMD_LOCATION_SEND_DESCRIPTION, cmd_location_send,
		      CMD_LOCATION_SEND_ARG_REQUIRED, CMD_LOCATION_SEND_ARG_OPTIONAL),
	SHELL_CMD_ARG(scan, NULL, CMD_LOCATION_SCAN_DESCRIPTION, cmd_location_scan,
		      CMD_LOCATION_SEND_ARG_REQUIRED, CMD_LOCATION_SEND_ARG_OPTIONAL),
	SHELL_CMD_ARG(alm_start, NULL, CMD_LOCATION_ALM_START_DESCRIPTION, cmd_location_alm_start,
		      CMD_LOCATION_ALM_START_ARG_REQUIRED, CMD_LOCATION_ALM_START_ARG_OPTIONAL),
	SHELL_SUBCMD_SET_END);

/* Main command registration */
SHELL_CMD_REGISTER(location, &sub_location_services, "Sidewalk Location CLI", NULL);

/* Utility function for simple parameter commands */
static int location_shell_simple_param(event_handler_t event, uint32_t *data)
{
	uint32_t *event_ctx = sid_hal_malloc(sizeof(uint32_t));
	if (!event_ctx) {
		return -ENOMEM;
	}
	memcpy(event_ctx, data, sizeof(uint32_t));

	int err = sidewalk_event_send(event, event_ctx, sid_hal_free);
	if (err) {
		sid_hal_free(event_ctx);
		return -ENOMSG;
	}
	return 0;
}

/* Location Command Handlers */
int cmd_location_init(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_INIT_ARG_REQUIRED, CMD_LOCATION_INIT_ARG_OPTIONAL);

	return sidewalk_event_send(location_event_init, NULL, NULL);
}

int cmd_location_deinit(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_DEINIT_ARG_REQUIRED,
			     CMD_LOCATION_DEINIT_ARG_OPTIONAL);

	return sidewalk_event_send(location_event_deinit, NULL, NULL);
}

int cmd_location_send(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_SEND_ARG_REQUIRED, CMD_LOCATION_SEND_ARG_OPTIONAL);

	uint32_t location_level = 0; /* Default to automatic mode */

	if (argc == 2) {
		char *end = NULL;
		long level_val = strtol(argv[1], &end, 0);
		if (end == argv[1] || !IN_RANGE(level_val, 1, 4)) {
			shell_error(
				shell,
				"Invalid location level [%s], must be 1-4 or no argument for automatic mode",
				argv[1]);
			return -EINVAL;
		}
		location_level = (uint32_t)level_val;
	}

	return location_shell_simple_param(location_event_send, &location_level);
}

int cmd_location_scan(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_SEND_ARG_REQUIRED, CMD_LOCATION_SEND_ARG_OPTIONAL);

	uint32_t location_level = 0; /* Default to automatic mode */

	if (argc == 2) {
		char *end = NULL;
		long level_val = strtol(argv[1], &end, 0);
		if (end == argv[1] || !IN_RANGE(level_val, 1, 4)) {
			shell_error(
				shell,
				"Invalid location level [%s], must be 1-4 or no argument for automatic mode",
				argv[1]);
			return -EINVAL;
		}
		location_level = (uint32_t)level_val;
	}

	return location_shell_simple_param(location_event_scan, &location_level);
}

int cmd_location_alm_start(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_ALM_START_ARG_REQUIRED,
			     CMD_LOCATION_ALM_START_ARG_OPTIONAL);

	shell_info(shell, "Location alm_start command - function not implemented");
	return sidewalk_event_send(location_event_alm_start, NULL, NULL);
}
