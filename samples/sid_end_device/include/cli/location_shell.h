/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef LOCATION_SHELL_H
#define LOCATION_SHELL_H

#include <stdint.h>
#include <zephyr/shell/shell.h>

/* Location Command Descriptions */
#define CMD_LOCATION_INIT_DESCRIPTION "Initializes sidewalk location library"

#define CMD_LOCATION_DEINIT_DESCRIPTION "Deinitializes sidewalk location library"

#define CMD_LOCATION_SEND_DESCRIPTION                                                              \
	"<LOCATION_LEVEL>\n"                                                                       \
	"Send a location uplink at the specified level 1-6. No level specified will use an "       \
	"automatic scaling mode, using lowest power mode first.\n"                                 \
	"1 - BLE gateway trigger\n"                                                                \
	"2 - FSK gateway trigger\n"                                                                \
	"3 - WiFi\n"                                                                               \
	"4 - GNSS\n"                                                                               \
	"5 - LoRa multilateration\n"                                                               \
	"6 - GNSS with GPS assist (not implemented in this sample, requires GPS-assist PAL)"

#define CMD_LOCATION_SCAN_DESCRIPTION                                                              \
	"<LOCATION_LEVEL>\n"                                                                       \
	"Scan at the specified level 3-4 without uplinking data. No level specified will use "     \
	"an automatic scaling mode, using lowest power mode first.\n"                              \
	"3 - WiFi\n"                                                                               \
	"4 - GNSS"

#define CMD_LOCATION_SEND_LEVELS_DESCRIPTION                                                       \
	"<LEVEL> [<LEVEL> ...]\n"                                                                  \
	"Send a location uplink trying the given effort levels (1-6) in order, up to 6 levels, " \
	"escalating past unsupported or insufficient levels. Level 6 (GPS assist) is not implemented "  \
	"in this sample. Example: send_levels 3 4"

#define CMD_LOCATION_SEND_PSEUDORANGE_DESCRIPTION                                                  \
	"<HEX>\n"                                                                                  \
	"Send a raw pseudorange payload (even-length hex string, up to 106 bytes) as is. "         \
	"Requires location init and time sync."

#define CMD_LOCATION_MANAGE_EFFORT_DESCRIPTION                                                     \
	"[0|1]\n"                                                                                  \
	"Show or set managed effort. Applies on the next location init."

#define CMD_LOCATION_SEND_BUF_DESCRIPTION                                                          \
	"<LOCATION_LEVEL>\n"                                                                       \
	"Send a test buffer via an available link without re-running a scan. The buffer is a "     \
	"49 byte payload that does not contain a valid location. The level is required.\n"         \
	"3 - WiFi\n"                                                                               \
	"4 - GNSS"

/* Argument counts */
#define CMD_LOCATION_INIT_ARG_REQUIRED 1
#define CMD_LOCATION_INIT_ARG_OPTIONAL 0

#define CMD_LOCATION_DEINIT_ARG_REQUIRED 1
#define CMD_LOCATION_DEINIT_ARG_OPTIONAL 0

#define CMD_LOCATION_SEND_ARG_REQUIRED 1
#define CMD_LOCATION_SEND_ARG_OPTIONAL 1

#define CMD_LOCATION_SEND_BUF_ARG_REQUIRED 2
#define CMD_LOCATION_SEND_BUF_ARG_OPTIONAL 0

#define CMD_LOCATION_SEND_LEVELS_ARG_REQUIRED 2
#define CMD_LOCATION_SEND_LEVELS_ARG_OPTIONAL 5

#define CMD_LOCATION_SEND_PSEUDORANGE_ARG_REQUIRED 2
#define CMD_LOCATION_SEND_PSEUDORANGE_ARG_OPTIONAL 0

#define CMD_LOCATION_MANAGE_EFFORT_ARG_REQUIRED 1
#define CMD_LOCATION_MANAGE_EFFORT_ARG_OPTIONAL 1

#define CMD_LOCATION_SCAN_ARG_REQUIRED 1
#define CMD_LOCATION_SCAN_ARG_OPTIONAL 1

/* Function declarations */
int cmd_location_init(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_deinit(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_send(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_scan(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_send_buf(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_send_levels(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_send_pseudorange(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_manage_effort(const struct shell *shell, int32_t argc, const char **argv);

#endif /* LOCATION_SHELL_H */
