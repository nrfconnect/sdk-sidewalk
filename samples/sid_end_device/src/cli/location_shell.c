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
	SHELL_CMD_ARG(send_buf, NULL, CMD_LOCATION_SEND_BUF_DESCRIPTION, cmd_location_send_buf,
		      CMD_LOCATION_SEND_BUF_ARG_REQUIRED, CMD_LOCATION_SEND_BUF_ARG_OPTIONAL),
	SHELL_CMD_ARG(send_levels, NULL, CMD_LOCATION_SEND_LEVELS_DESCRIPTION,
		      cmd_location_send_levels, CMD_LOCATION_SEND_LEVELS_ARG_REQUIRED,
		      CMD_LOCATION_SEND_LEVELS_ARG_OPTIONAL),
	SHELL_CMD_ARG(send_pseudorange, NULL, CMD_LOCATION_SEND_PSEUDORANGE_DESCRIPTION,
		      cmd_location_send_pseudorange, CMD_LOCATION_SEND_PSEUDORANGE_ARG_REQUIRED,
		      CMD_LOCATION_SEND_PSEUDORANGE_ARG_OPTIONAL),
	SHELL_CMD_ARG(scan, NULL, CMD_LOCATION_SCAN_DESCRIPTION, cmd_location_scan,
		      CMD_LOCATION_SCAN_ARG_REQUIRED, CMD_LOCATION_SCAN_ARG_OPTIONAL),
	SHELL_CMD_ARG(manage_effort, NULL, CMD_LOCATION_MANAGE_EFFORT_DESCRIPTION,
		      cmd_location_manage_effort, CMD_LOCATION_MANAGE_EFFORT_ARG_REQUIRED,
		      CMD_LOCATION_MANAGE_EFFORT_ARG_OPTIONAL),
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

/* Parse an effort level in the range min_level..max_level. */
static int location_shell_parse_level(const struct shell *shell, const char *arg,
				      enum sid_location_effort_mode min_level,
				      enum sid_location_effort_mode max_level, uint32_t *level)
{
	char *end = NULL;
	long val = strtol(arg, &end, 0);

	if (end == arg || *end != '\0' || !IN_RANGE(val, (long)min_level, (long)max_level)) {
		shell_error(shell, "Invalid location level [%s], must be %d-%d", arg, min_level,
			    max_level);
		return -EINVAL;
	}
	*level = (uint32_t)val;
	return 0;
}

static int location_shell_level_cmd(const struct shell *shell, int32_t argc, const char **argv,
				    enum sid_location_effort_mode min_level,
				    enum sid_location_effort_mode max_level, event_handler_t event)
{
	uint32_t location_level = SID_LOCATION_EFFORT_DEFAULT;

	if (argc == 2) {
		int err = location_shell_parse_level(shell, argv[1], min_level, max_level,
						     &location_level);

		if (err) {
			return err;
		}
	}

	return location_shell_simple_param(event, &location_level);
}

int cmd_location_send(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_SEND_ARG_REQUIRED, CMD_LOCATION_SEND_ARG_OPTIONAL);

	return location_shell_level_cmd(shell, argc, argv, SID_LOCATION_EFFORT_L1,
					SID_LOCATION_EFFORT_L6, location_event_send);
}

int cmd_location_scan(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_SCAN_ARG_REQUIRED, CMD_LOCATION_SCAN_ARG_OPTIONAL);

	return location_shell_level_cmd(shell, argc, argv, SID_LOCATION_EFFORT_L3,
					SID_LOCATION_EFFORT_L4, location_event_scan);
}

int cmd_location_send_buf(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_SEND_BUF_ARG_REQUIRED,
			     CMD_LOCATION_SEND_BUF_ARG_OPTIONAL);

	return location_shell_level_cmd(shell, argc, argv, SID_LOCATION_EFFORT_L3,
					SID_LOCATION_EFFORT_L4, location_event_send_buffer);
}

int cmd_location_send_levels(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_SEND_LEVELS_ARG_REQUIRED,
			     CMD_LOCATION_SEND_LEVELS_ARG_OPTIONAL);

	struct location_send_levels_params *params =
		sid_hal_malloc(sizeof(struct location_send_levels_params));

	if (!params) {
		return -ENOMEM;
	}
	memset(params, 0, sizeof(*params));

	for (int32_t i = 1; i < argc; i++) {
		uint32_t level = 0;
		int err = location_shell_parse_level(shell, argv[i], SID_LOCATION_EFFORT_L1,
						     SID_LOCATION_EFFORT_L6, &level);

		if (err) {
			sid_hal_free(params);
			return err;
		}
		params->levels[params->num_levels++] = (uint8_t)level;
	}

	int err = sidewalk_event_send(location_event_send_levels, params, sid_hal_free);

	if (err) {
		sid_hal_free(params);
		return -ENOMSG;
	}
	return 0;
}

int cmd_location_send_pseudorange(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_SEND_PSEUDORANGE_ARG_REQUIRED,
			     CMD_LOCATION_SEND_PSEUDORANGE_ARG_OPTIONAL);

	size_t hex_len = strlen(argv[1]);

	if (hex_len == 0 || (hex_len % 2) != 0 || (hex_len / 2) > SID_LOCATION_MAX_PAYLOAD_SIZE) {
		shell_error(shell, "Bad hex, need even length and at most %d bytes",
			    SID_LOCATION_MAX_PAYLOAD_SIZE);
		return -EINVAL;
	}

	struct location_send_pseudorange_params *params =
		sid_hal_malloc(sizeof(struct location_send_pseudorange_params));

	if (!params) {
		return -ENOMEM;
	}
	memset(params, 0, sizeof(*params));

	params->size = hex2bin(argv[1], hex_len, params->data, sizeof(params->data));
	if (params->size != hex_len / 2) {
		shell_error(shell, "Non-hex character in payload");
		sid_hal_free(params);
		return -EINVAL;
	}

	int err = sidewalk_event_send(location_event_send_pseudorange, params, sid_hal_free);

	if (err) {
		sid_hal_free(params);
		return -ENOMSG;
	}
	return 0;
}

int cmd_location_manage_effort(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_MANAGE_EFFORT_ARG_REQUIRED,
			     CMD_LOCATION_MANAGE_EFFORT_ARG_OPTIONAL);

	if (argc == 1) {
		return sidewalk_event_send(location_event_manage_effort, NULL, NULL);
	}

	char *end = NULL;
	long val = strtol(argv[1], &end, 0);

	if (end == argv[1] || *end != '\0' || !IN_RANGE(val, 0, 1)) {
		shell_error(shell, "Invalid value [%s], use 0 (off) or 1 (on)", argv[1]);
		return -EINVAL;
	}

	uint32_t enabled = (uint32_t)val;

	return location_shell_simple_param(location_event_manage_effort, &enabled);
}
