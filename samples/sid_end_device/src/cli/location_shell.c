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

/* Data structures for DULT commands */
struct dult_config_params {
	bool reset;
	char product_data[9]; /* 8 bytes + null terminator */
	char manufacturer_data[65]; /* 64 bytes + null terminator */
	char model_data[65]; /* 64 bytes + null terminator */
	char identifier[33]; /* 32 bytes + null terminator */
	uint16_t firmware_major;
	uint8_t firmware_minor;
	uint8_t firmware_revision;
	uint8_t category_id;
	uint8_t network_id;
	uint8_t battery_type;
	uint8_t advertising_mode;
	uint8_t initial_proximity;
};

struct dult_set_params {
	bool reset;
	uint8_t proximity_state;
	uint8_t ignore_non_owner_events;
	uint8_t non_owner_find_status;
	uint8_t identifier_read_mode;
	uint8_t battery_level;
};

#define CHECK_ARGUMENT_COUNT(argc, required, optional)                                             \
	if ((argc < required) || (argc > (required + optional))) {                                 \
		return -EINVAL;                                                                    \
	}

/* DULT Subcommands */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_dult_services,
	SHELL_CMD_ARG(init, NULL, CMD_DULT_INIT_DESCRIPTION, cmd_dult_init,
		      CMD_DULT_INIT_ARG_REQUIRED, CMD_DULT_INIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(deinit, NULL, CMD_DULT_DEINIT_DESCRIPTION, cmd_dult_deinit,
		      CMD_DULT_DEINIT_ARG_REQUIRED, CMD_DULT_DEINIT_ARG_OPTIONAL),
	SHELL_CMD_ARG(config, NULL, CMD_DULT_CONFIG_DESCRIPTION, cmd_dult_config,
		      CMD_DULT_CONFIG_ARG_REQUIRED, CMD_DULT_CONFIG_ARG_OPTIONAL),
	SHELL_CMD_ARG(set, NULL, CMD_DULT_SET_DESCRIPTION, cmd_dult_set, CMD_DULT_SET_ARG_REQUIRED,
		      CMD_DULT_SET_ARG_OPTIONAL),
	SHELL_SUBCMD_SET_END);

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
	SHELL_CMD_ARG(scan, NULL, CMD_LOCATION_SCAN_DESCRIPTION, cmd_location_scan,
		      CMD_LOCATION_SEND_ARG_REQUIRED, CMD_LOCATION_SEND_ARG_OPTIONAL),
	SHELL_CMD_ARG(alm_start, NULL, CMD_LOCATION_ALM_START_DESCRIPTION, cmd_location_alm_start,
		      CMD_LOCATION_ALM_START_ARG_REQUIRED, CMD_LOCATION_ALM_START_ARG_OPTIONAL),
	SHELL_SUBCMD_SET_END);

/* Main command registration */
SHELL_CMD_REGISTER(dult, &sub_dult_services, "Sidewalk DULT CLI", NULL);
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

/* Free functions for event contexts */
static void free_dult_config_event_ctx(void *ctx)
{
	struct dult_config_params *params = (struct dult_config_params *)ctx;
	if (params) {
		sid_hal_free(params);
	}
}

static void free_dult_set_event_ctx(void *ctx)
{
	struct dult_set_params *params = (struct dult_set_params *)ctx;
	if (params) {
		sid_hal_free(params);
	}
}

/* DULT Command Handlers */
int cmd_dult_init(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_DULT_INIT_ARG_REQUIRED, CMD_DULT_INIT_ARG_OPTIONAL);

	shell_info(shell, "DULT init command - function not implemented");
	return sidewalk_event_send(dult_event_init, NULL, NULL);
}

int cmd_dult_deinit(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_DULT_DEINIT_ARG_REQUIRED, CMD_DULT_DEINIT_ARG_OPTIONAL);

	shell_info(shell, "DULT deinit command - function not implemented");
	return sidewalk_event_send(dult_event_deinit, NULL, NULL);
}

int cmd_dult_config(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_DULT_CONFIG_ARG_REQUIRED, CMD_DULT_CONFIG_ARG_OPTIONAL);

	// If no arguments, just print current config
	if (argc == 1) {
		shell_info(shell, "DULT config command - function not implemented");
		return sidewalk_event_send(dult_event_config, NULL, NULL);
	}

	// Allocate memory for config parameters
	struct dult_config_params *config_params =
		sid_hal_malloc(sizeof(struct dult_config_params));
	if (!config_params) {
		return -ENOMEM;
	}
	memset(config_params, 0, sizeof(*config_params));

	// Parse parameters similar to cmd_sid_send pattern
	for (int opt = 1; opt < argc; opt++) {
		if (strcmp("-r", argv[opt]) == 0) {
			// Reset to default
			config_params->reset = true;
			shell_info(shell, "DULT config reset to default");
			int err = sidewalk_event_send(dult_event_config, config_params,
						      free_dult_config_event_ctx);
			if (err) {
				free_dult_config_event_ctx(config_params);
				return -ENOMSG;
			}
			return 0;
		}
		if (strcmp("-p", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-p need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			if (strlen(argv[opt]) > 8) {
				shell_error(shell, "Product data too long, maximum 8 bytes");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			strncpy(config_params->product_data, argv[opt],
				sizeof(config_params->product_data) - 1);
			shell_info(shell, "Product data: %s", argv[opt]);
			continue;
		}
		if (strcmp("-m", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-m need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			if (strlen(argv[opt]) > 64) {
				shell_error(shell, "Manufacturer data too long, maximum 64 bytes");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			strncpy(config_params->manufacturer_data, argv[opt],
				sizeof(config_params->manufacturer_data) - 1);
			shell_info(shell, "Manufacturer data: %s", argv[opt]);
			continue;
		}
		if (strcmp("-o", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-o need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			if (strlen(argv[opt]) > 64) {
				shell_error(shell, "Model data too long, maximum 64 bytes");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			strncpy(config_params->model_data, argv[opt],
				sizeof(config_params->model_data) - 1);
			shell_info(shell, "Model data: %s", argv[opt]);
			continue;
		}
		if (strcmp("-i", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-i need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			if (strlen(argv[opt]) > 32) {
				shell_error(shell, "Identifier too long, maximum 32 bytes");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			strncpy(config_params->identifier, argv[opt],
				sizeof(config_params->identifier) - 1);
			shell_info(shell, "Identifier: %s", argv[opt]);
			continue;
		}
		if (strcmp("-f", argv[opt]) == 0) {
			if (opt + 3 > argc) {
				shell_error(shell, "-f need 3 positional arguments");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			const char *major = argv[++opt];
			const char *minor = argv[++opt];
			const char *revision = argv[++opt];

			char *end = NULL;
			long major_val = strtol(major, &end, 0);
			if (!IN_RANGE(major_val, 0, UINT16_MAX) || end == major) {
				shell_error(shell, "Invalid major version [%s]", major);
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}

			long minor_val = strtol(minor, &end, 0);
			if (!IN_RANGE(minor_val, 0, UINT8_MAX) || end == minor) {
				shell_error(shell, "Invalid minor version [%s]", minor);
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}

			long revision_val = strtol(revision, &end, 0);
			if (!IN_RANGE(revision_val, 0, UINT8_MAX) || end == revision) {
				shell_error(shell, "Invalid revision [%s]", revision);
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}

			config_params->firmware_major = (uint16_t)major_val;
			config_params->firmware_minor = (uint8_t)minor_val;
			config_params->firmware_revision = (uint8_t)revision_val;
			shell_info(shell, "Firmware version: %ld.%ld.%ld", major_val, minor_val,
				   revision_val);
			continue;
		}
		if (strcmp("-c", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-c need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			char *end = NULL;
			long category_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(category_val, 1, 167) || end == argv[opt]) {
				shell_error(shell, "Invalid category ID [%s], must be 1-167",
					    argv[opt]);
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			config_params->category_id = (uint8_t)category_val;
			shell_info(shell, "Category ID: %ld", category_val);
			continue;
		}
		if (strcmp("-n", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-n need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			char *end = NULL;
			long network_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(network_val, 1, 3) || end == argv[opt]) {
				shell_error(shell, "Invalid network ID [%s], must be 1-3",
					    argv[opt]);
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			config_params->network_id = (uint8_t)network_val;
			shell_info(shell, "Network ID: %ld", network_val);
			continue;
		}
		if (strcmp("-b", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-b need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			char *end = NULL;
			long battery_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(battery_val, 0, 2) || end == argv[opt]) {
				shell_error(shell, "Invalid battery type [%s], must be 0-2",
					    argv[opt]);
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			config_params->battery_type = (uint8_t)battery_val;
			shell_info(shell, "Battery type: %ld", battery_val);
			continue;
		}
		if (strcmp("-a", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-a need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			char *end = NULL;
			long adv_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(adv_val, 0, 1) || end == argv[opt]) {
				shell_error(shell, "Invalid advertising mode [%s], must be 0-1",
					    argv[opt]);
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			config_params->advertising_mode = (uint8_t)adv_val;
			shell_info(shell, "Advertising mode: %ld", adv_val);
			continue;
		}
		if (strcmp("-im", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-im need a value");
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			char *end = NULL;
			long proximity_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(proximity_val, 0, 1) || end == argv[opt]) {
				shell_error(shell, "Invalid initial proximity [%s], must be 0-1",
					    argv[opt]);
				free_dult_config_event_ctx(config_params);
				return -EINVAL;
			}
			config_params->initial_proximity = (uint8_t)proximity_val;
			shell_info(shell, "Initial proximity: %ld", proximity_val);
			continue;
		}
	}

	shell_info(shell, "DULT config command - function not implemented");
	int err = sidewalk_event_send(dult_event_config, config_params, free_dult_config_event_ctx);
	if (err) {
		free_dult_config_event_ctx(config_params);
		return -ENOMSG;
	}
	return 0;
}

int cmd_dult_set(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_DULT_SET_ARG_REQUIRED, CMD_DULT_SET_ARG_OPTIONAL);

	// If no arguments, just print current config
	if (argc == 1) {
		shell_info(shell, "DULT set command - function not implemented");
		return sidewalk_event_send(dult_event_set, NULL, NULL);
	}

	// Allocate memory for set parameters
	struct dult_set_params *set_params = sid_hal_malloc(sizeof(struct dult_set_params));
	if (!set_params) {
		return -ENOMEM;
	}
	memset(set_params, 0, sizeof(*set_params));

	// Parse parameters similar to cmd_sid_send pattern
	for (int opt = 1; opt < argc; opt++) {
		if (strcmp("-r", argv[opt]) == 0) {
			// Reset to default
			set_params->reset = true;
			shell_info(shell, "DULT set reset to default");
			int err = sidewalk_event_send(dult_event_set, set_params,
						      free_dult_set_event_ctx);
			if (err) {
				free_dult_set_event_ctx(set_params);
				return -ENOMSG;
			}
			return 0;
		}
		if (strcmp("-m", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-m need a value");
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			char *end = NULL;
			long proximity_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(proximity_val, 0, 1) || end == argv[opt]) {
				shell_error(shell, "Invalid proximity state [%s], must be 0-1",
					    argv[opt]);
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			set_params->proximity_state = (uint8_t)proximity_val;
			shell_info(shell, "Proximity state: %ld", proximity_val);
			continue;
		}
		if (strcmp("-o", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-o need a value");
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			char *end = NULL;
			long ignore_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(ignore_val, 0, 1) || end == argv[opt]) {
				shell_error(shell,
					    "Invalid ignore non owner events [%s], must be 0-1",
					    argv[opt]);
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			set_params->ignore_non_owner_events = (uint8_t)ignore_val;
			shell_info(shell, "Ignore non owner events: %ld", ignore_val);
			continue;
		}
		if (strcmp("-s", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-s need a value");
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			char *end = NULL;
			long status_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(status_val, 0, 1) || end == argv[opt]) {
				shell_error(shell,
					    "Invalid non owner find status [%s], must be 0-1",
					    argv[opt]);
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			set_params->non_owner_find_status = (uint8_t)status_val;
			shell_info(shell, "Non owner find status: %ld", status_val);
			continue;
		}
		if (strcmp("-i", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-i need a value");
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			char *end = NULL;
			long identifier_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(identifier_val, 0, 1) || end == argv[opt]) {
				shell_error(shell, "Invalid identifier read mode [%s], must be 0-1",
					    argv[opt]);
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			set_params->identifier_read_mode = (uint8_t)identifier_val;
			shell_info(shell, "Enter identifier read mode: %ld", identifier_val);
			continue;
		}
		if (strcmp("-l", argv[opt]) == 0) {
			opt++;
			if (opt >= argc) {
				shell_error(shell, "-l need a value");
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			char *end = NULL;
			long battery_val = strtol(argv[opt], &end, 0);
			if (!IN_RANGE(battery_val, 0, 3) || end == argv[opt]) {
				shell_error(shell, "Invalid battery level [%s], must be 0-3",
					    argv[opt]);
				free_dult_set_event_ctx(set_params);
				return -EINVAL;
			}
			set_params->battery_level = (uint8_t)battery_val;
			shell_info(shell, "Battery level: %ld", battery_val);
			continue;
		}
	}

	shell_info(shell, "DULT set command - function not implemented");
	int err = sidewalk_event_send(dult_event_set, set_params, free_dult_set_event_ctx);
	if (err) {
		free_dult_set_event_ctx(set_params);
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

int cmd_location_send_buf(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_SEND_BUF_ARG_REQUIRED,
			     CMD_LOCATION_SEND_BUF_ARG_OPTIONAL);

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

	return location_shell_simple_param(location_event_send_buffer, &location_level);
}

int cmd_location_alm_start(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_LOCATION_ALM_START_ARG_REQUIRED,
			     CMD_LOCATION_ALM_START_ARG_OPTIONAL);

	shell_info(shell, "Location alm_start command - function not implemented");
	return sidewalk_event_send(location_event_alm_start, NULL, NULL);
}
