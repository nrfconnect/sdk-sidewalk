/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef LOCATION_SHELL_H
#define LOCATION_SHELL_H

#include <stdint.h>
#include <zephyr/shell/shell.h>

/* DULT Command Descriptions */
#define CMD_DULT_INIT_DESCRIPTION "Initializes sidewalk unwanted location tracking component"

#define CMD_DULT_DEINIT_DESCRIPTION "Deinitializes sidewalk unwanted location tracking"

#define CMD_DULT_CONFIG_DESCRIPTION                                                                                                                      \
	"-r -p <PRODUCT_DATA> -m <MANUFACTURER_DATA> -o <MODEL> -i <IDENTIFIER> "                                                                        \
	"-f <MAJOR VERSION> <MINOR VERSION> <REVISION> -c <CATEGORY ID> "                                                                                \
	"-n <NETWORK ID> -b <BATTERY_TYPE> -a <ADV MODE> -im <INITIAL PROXIMITY>\n"                                                                      \
	"Will allow for configuration of various dult config information\n"                                                                              \
	"<No Arguments> Will print the existing/default config\n"                                                                                        \
	"-r reset the config to default\n"                                                                                                               \
	"-p Set the product_data, maximum is 8 bytes\n"                                                                                                  \
	"-m Set the manufacturer name, maximum is 64 bytes\n"                                                                                            \
	"-o Set the model name, maximum is 64 bytes\n"                                                                                                   \
	"-i Set the device identifier, maximum is 32 bytes\n"                                                                                            \
	"-f Sets the major, minor, revision of the firmware, major max is 2 bytes, minor max is 1 byte and revision is 1 byte\n"                         \
	"-c Sets the category id see https://www.ietf.org/archive/id/draft-detecting-unwanted-location-trackers-01.html#name-accessory-category-value\n" \
	"-n Sets the network id, 1 - Apple, 2 - Google, 3 - Sidewalk (Tentative)\n"                                                                      \
	"-b Sets the battery type, 0 = Powered, 1 = Non-rechargeable battery, 2 = Rechargeable battery\n"                                                \
	"-a Sets the advertising mode, 0 will only advertise DULT in separated mode, 1 will advertise in both modes\n"                                   \
	"-im Sets the initial proximity state, 0 is SEPARATED, 1 is NEAR mode"

#define CMD_DULT_SET_DESCRIPTION                                                                                                                                                            \
	"-r -m <proximity state> -o <ignore non owner events> -s <non owner find status> "                                                                                                  \
	"-i <enter identifier read mode> -l <BATTERY_LEVEL>\n"                                                                                                                              \
	"Configure the behaviour of DULT\n"                                                                                                                                                 \
	"<No Arguments> Will print the existing/default config\n"                                                                                                                           \
	"-r reset to default\n"                                                                                                                                                             \
	"-m Set the proximity state, 0 - Separated mode, 1 - Near mode\n"                                                                                                                   \
	"-o Ignore if gatt requests for non owner find events such as Sound on/off etc, 0 - Do not ignore events respond with status defined by non_owner_find_status, 1 - ignore events\n" \
	"-s Status of the non owner find event, 0 - Failure, 1 - Success\n"                                                                                                                 \
	"-i Enter get identifier state of dult, if this is not set, then the GET_IDENTIFIER will fail\n"                                                                                    \
	"-l Sets the battery level, 0 = Full, 1 = Medium, 2 = Low, 3 = Critically low"

/* Location Command Descriptions */
#define CMD_LOCATION_INIT_DESCRIPTION "Initializes sidewalk location library"

#define CMD_LOCATION_DEINIT_DESCRIPTION "Deinitializes sidewalk location library"

#define CMD_LOCATION_SEND_DESCRIPTION                                                                                                                \
	"<LOCATION_LEVEL>\n"                                                                                                                         \
	"Send a location uplink at the specified level 1-4. No level specified will use an automatic scaling mode, using lowest power mode first.\n" \
	"1 - BLE Uplink using Sidewalk Network location\n"                                                                                           \
	"2 - LoRa Triangulation (not supported)\n"                                                                                                   \
	"3 - WiFi\n"                                                                                                                                 \
	"4 - GNSS"

#define CMD_LOCATION_SCAN_DESCRIPTION                                                                                                                \
	"<LOCATION_LEVEL>\n"                                                                                                                         \
	"Scan a location uplink at the specified level 1-4. No level specified will use an automatic scaling mode, using lowest power mode first.\n" \
	"1 - BLE Uplink using Sidewalk Network location\n"                                                                                           \
	"2 - LoRa Triangulation (not supported)\n"                                                                                                   \
	"3 - WiFi\n"                                                                                                                                 \
	"4 - GNSS"

#define CMD_LOCATION_ALM_START_DESCRIPTION                                                         \
	"Start the almanac demodulation service to update the GNSS almanac via satellite. The service cannot currently be stopped."

#define CMD_LOCATION_SEND_BUF_DESCRIPTION                                                                                                               \
	"<LOCATION_LEVEL>\n"                                                                                                                            \
	"Send a test buffer via an available link without re-running a scan. The buffer is a 50 byte payload that does not contain a valid location.\n" \
	"1 - BLE Uplink using Sidewalk Network location\n"                                                                                              \
	"2 - LoRa Triangulation (not supported)\n"                                                                                                      \
	"3 - WiFi\n"                                                                                                                                    \
	"4 - GNSS"

/* Argument counts */
#define CMD_DULT_INIT_ARG_REQUIRED 1
#define CMD_DULT_INIT_ARG_OPTIONAL 0

#define CMD_DULT_DEINIT_ARG_REQUIRED 1
#define CMD_DULT_DEINIT_ARG_OPTIONAL 0

#define CMD_DULT_CONFIG_ARG_REQUIRED 1
#define CMD_DULT_CONFIG_ARG_OPTIONAL 20

#define CMD_DULT_SET_ARG_REQUIRED 1
#define CMD_DULT_SET_ARG_OPTIONAL 10

#define CMD_LOCATION_INIT_ARG_REQUIRED 1
#define CMD_LOCATION_INIT_ARG_OPTIONAL 0

#define CMD_LOCATION_DEINIT_ARG_REQUIRED 1
#define CMD_LOCATION_DEINIT_ARG_OPTIONAL 0

#define CMD_LOCATION_SEND_ARG_REQUIRED 1
#define CMD_LOCATION_SEND_ARG_OPTIONAL 1

#define CMD_LOCATION_ALM_START_ARG_REQUIRED 1
#define CMD_LOCATION_ALM_START_ARG_OPTIONAL 0

#define CMD_LOCATION_SEND_BUF_ARG_REQUIRED 1
#define CMD_LOCATION_SEND_BUF_ARG_OPTIONAL 1

/* Function declarations */
int cmd_dult_init(const struct shell *shell, int32_t argc, const char **argv);
int cmd_dult_deinit(const struct shell *shell, int32_t argc, const char **argv);
int cmd_dult_config(const struct shell *shell, int32_t argc, const char **argv);
int cmd_dult_set(const struct shell *shell, int32_t argc, const char **argv);

int cmd_location_init(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_deinit(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_send(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_scan(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_send_buf(const struct shell *shell, int32_t argc, const char **argv);
int cmd_location_alm_start(const struct shell *shell, int32_t argc, const char **argv);

#endif /* LOCATION_SHELL_H */
