/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_DUT_SHELL_H
#define SID_DUT_SHELL_H

#include <stdint.h>
#include <zephyr/shell/shell.h>
#include <sid_thread.h>
#include <sid_api.h>

#define CMD_SID_INIT_DESCRIPTION                                                                                                                                                                                                         \
	"<1,2,3,4,5>\n"                                                                                                                                                                                                                  \
	"initialize the sidewalk stack, 1 is SID_LINK_TYPE_1 (BLE), 2 is SID_LINK_TYPE_2 (FSK), 3 is SID_LINK_TYPE_3 (LORA) 4 is SID_LINK_TYPE_1 | SID_LINK_TYPE_3 (BLE + LORA),  5 is SID_LINK_TYPE_1 | SID_LINK_TYPE_2 (BLE + FSK).\n" \
	"This calls the sid_init() API."

#define CMD_SID_DEINIT_DESCRIPTION                                                                 \
	"\n"                                                                                       \
	"deinitialize the sidewalk stack, calls sid_deinit() API"

#define CMD_SID_START_DESCRIPTION                                                                  \
	"<link>\n"                                                                                 \
	"start the sidewalk stack, calls sid_start() API.\n"                                       \
	"link value is optional, it can take the same values as for sid init command. If link value is not present the one set with sid init will be used to call sid_start api."

#define CMD_SID_STOP_DESCRIPTION                                                                   \
	"<link>\n"                                                                                 \
	"stop the sidewalk stack, calls sid_stop() API.\n"                                         \
	"link value is optional, it can take the same values as for sid init command. If link value is not present the one set with sid init will be used to call sid_stop api."

#define CMD_SID_SEND_DESCRIPTION                                                                                                                                                                     \
	"-t <tv> -d <dv> -l <lm> -i <id> -a <ack> <retry> <ttl> -r <data>\n"                                                                                                                         \
	"send data over the SID_LINK_TYPE selected, calls the sid_put_msg()API.\n"                                                                                                                   \
	"Data field must always be placed at the end of command patametrs. If -r parameter is not preset data filed is treated as ascii. Example usage:\n"                                           \
	"    - sid send TEST\n"                                                                                                                                                                      \
	"    - sid send -t 0 -d 2 TEST\n"                                                                                                                                                            \
	"    - sid send -t 1 -d 1 -r 01AAFF02\n"                                                                                                                                                     \
	"Optional parametrs:\n"                                                                                                                                                                      \
	"- t option force specific message type to be set in message descriptor possible <tv> values:\n"                                                                                             \
	"0 - SID_MSG_TYPE_GET\n"                                                                                                                                                                     \
	"1 - SID_MSG_TYPE_SET\n"                                                                                                                                                                     \
	"2 - SID_MSG_TYPE_NOTIFY\n"                                                                                                                                                                  \
	"3 - SID_MSG_TYPE_RESPONSE\n"                                                                                                                                                                \
	"If -t option is not used message type in message descriptor will be set to SID_MSG_TYPE_NOTIFY.\n"                                                                                          \
	"If message type is set to RESPONSE type ID for outgoing message will be set with value that was previously received with GET type message. Or it can be set with sid set_rsp_id command.\n" \
	"- d specifies the destination\n"                                                                                                                                                            \
	"possible <dv> values:\n"                                                                                                                                                                    \
	"1 - SID_LINK_MODE_CLOUD\n"                                                                                                                                                                  \
	"2 - SID_LINK_MODE_MOBILE\n"                                                                                                                                                                 \
	"If -d option is not used link_mode in message descriptor will be set to SID_LINK_MODE_CLOUD.\n"                                                                                             \
	"- l link mask on which message should be sent, if not set LINK_TYPE_ANY will be used\n"                                                                                                     \
	"	<lm> link mask - for possible values see 'sid init' command\n"                                                                                                                             \
	"- i message id that needs to be used to send response. Valid only for messages of type response\n"                                                                                          \
	"	<id> response id\n"                                                                                                                                                                        \
	"- r data is interpreted hex string e.g. 010203AAFF\n"                                                                                                                                       \
	"- a configure parameters for transport ack:\n"                                                                                                                                              \
	"	<ack> - enable/disable ACK\n"                                                                                                                                                              \
	"		1 - enable ACK\n"                                                                                                                                                                         \
	"		0 - disable ACK\n"                                                                                                                                                                        \
	"	<retry> - number of retry. 0 ~ 255\n"                                                                                                                                                      \
	"	<ttl> - total seconds the stack holds the message in its queue. 0 ~ 65535"

#define CMD_SID_FACTORY_RESET_DESCRIPTION                                                          \
	"\n"                                                                                       \
	"factory reset the board, deleting all registration status. This calls the sid_set_factory_reset() API."

#define CMD_SID_GET_MTU_DESCRIPTION                                                                \
	"<1,2,3>\n"                                                                                \
	"get the MTU for the selected link type, 1 is SID_LINK_TYPE_1 (BLE), 2 is SID_LINK_TYPE_2 (FSK), 3 is SID_LINK_TYPE_3 (LORA). This calls the sid_get_mtu() API."

#define CMD_SID_SET_OPTION_DESCRIPTION                                                                                                                                      \
	"<option> <val1>...<valN>\n"                                                                                                                                        \
	"Set link options. This calls the sid_option() API. Possible inputs for  \"<option> <val1>...<valN>\" are as follows:\n"                                            \
	"\"-b <batlevel>\" - for setting battery level for SID_LINK_TYPE_1. Not supported for other link types.\n"                                                          \
	"	<batlevel> - (uint8) battery level.\n"                                                                                                                            \
	"\"-lp_set <val1> .. <valN>\" -  sets SID_LINK_TYPE_2 and SID_LINK_TYPE_3 (900 MHz) link profile and related parameters.\n"                                         \
	"\"-lp_set 1\" - for SID_LINK2_PROFILE_1\".\n"                                                                                                                      \
	"\"-lp_set 2 <rx_int>\" - for SID_LINK2_PROFILE_2 with optional rx_int parameter\".\n"                                                                              \
	"	<rx_int> Specifies DL interval between rx opportunities in units of ms. The value must be a multiple of 63ms. When ommitted the default value of 63ms is used.\n" \
	"\"-lp_set 0x80 <rxwc>\" - for SID_LINK3_PROFILE_A, where <rxwc> is the rx_window count parameter.\n"                                                               \
	"\"-lp_set 0x81 <rxwc>\" - for SID_LINK3_PROFILE_B, where <rxwc> is the rx_window count parameter.\n"                                                               \
	"\"-lp_set 0x83 <rxwc>\" - for SID_LINK3_PROFILE_D, where <rxwc> is the rx_window count parameter.\n"                                                               \
	"	<rxwc> - (uint8) rx window count. 0 represents infinite windows.\n"                                                                                               \
	"\"-lp_get_l2\" - Gets link profile and associated parameters for SID_LINK_TYPE_2. Ex: \"app: CMD: ERR: 0 Link_profile ID: 1 Wndw_cnt: 0\".\n"                      \
	"\"-lp_get_l3\" - Gets link profile and associated parameters for SID_LINK_TYPE_3. Ex: \"app: CMD: ERR: 0 Link_profile ID: 128 Wndw_cnt: 5\"."                      \
	"\"-d <0,1>\" - filter duplicate message.\n"                                                                                                                        \
	"   0 - filter duplicate message.\n"                                                                                                                                \
	"   1 - don't filter duplicate message and notify to user."                                                                                                         \
	"\"-gd\" - Get filter duplicates configuration. Ex: \"app: CMD: ERR: 0 Filter Duplicates: 0\""

#define CMD_SID_LAST_STATUS_DESCRIPTION                                                            \
	"\n"                                                                                       \
	"get last status of Sidewalk library. Result is printed in the same format as EVENT SID STATUS. This call the sid_get_status() API."

#define CMD_SID_CONN_REQUEST_DESCRIPTION                                                           \
	"<0,1>\n"                                                                                  \
	"set the connection request bit in BLE beacon,\n"                                          \
	"1 - set connection request bit\n"                                                         \
	"0 - clear connection request bit\n"                                                       \
	"This calls the sid_ble_bcn_connection_request() API.\n"

#define CMD_SID_GET_TIME_DESCRIPTION                                                               \
	"<0-2>\n"                                                                                  \
	"get time from Sidewalk library,\n"                                                        \
	"0 - GPS time\n"                                                                           \
	"1 - UTC time (not supported by sid api)\n"                                                \
	"2 - Local time (not supported by sid api)\n"                                              \
	"This calls the sid_get_time() API."

#define CMD_SID_SET_DST_ID_DESCRIPTION                                                             \
	"<id>\n"                                                                                   \
	"set message destination ID. This calls the sid_set_msg_dest_id() API."

#define CMD_SID_SET_SEND_LINK_DESCRIPTION                                                                                             \
	"<link type>\n"                                                                                                               \
	"set the link type that the message should be sent over. This is optional and only applies when multiple links are enabled\n" \
	"0 - SID_LINK_TYPE_ANY\n"                                                                                                     \
	"1 - SID_LINK_TYPE_1 (BLE)\n"                                                                                                 \
	"2 - SID_LINK_TYPE_2 (FSK)\n"                                                                                                 \
	"3 - SID_LINK_TYPE_3 (LORA)"

#define CMD_SID_SET_RSP_ID_DESCRIPTION                                                             \
	"<value>\n"                                                                                \
	"allow the set ID for RESPONSE type message, this can be use before calling sid send command with message type set as RESPONSE."

#define CMD_SID_SDK_VERSION_DESCRIPTION "Print sid sdk version"

#define CMD_SID_SDK_CONFIG_DESCRIPTION "Print sid sdk config"

#define CMD_SID_INIT_ARG_REQUIRED 2
#define CMD_SID_INIT_ARG_OPTIONAL 0
#define CMD_SID_DEINIT_ARG_REQUIRED 1
#define CMD_SID_DEINIT_ARG_OPTIONAL 0
#define CMD_SID_START_ARG_REQUIRED 1
#define CMD_SID_START_ARG_OPTIONAL 1
#define CMD_SID_STOP_ARG_REQUIRED 1
#define CMD_SID_STOP_ARG_OPTIONAL 1
#define CMD_SID_SEND_ARG_REQUIRED 2
#define CMD_SID_SEND_ARG_OPTIONAL 13
#define CMD_SID_FACTORY_RESET_ARG_REQUIRED 1
#define CMD_SID_FACTORY_RESET_ARG_OPTIONAL 0
#define CMD_SID_GET_MTU_ARG_REQUIRED 2
#define CMD_SID_GET_MTU_ARG_OPTIONAL 0
#define CMD_SID_SET_OPTION_ARG_REQUIRED 2
#define CMD_SID_SET_OPTION_ARG_OPTIONAL 2
#define CMD_SID_LAST_STATUS_ARG_REQUIRED 1
#define CMD_SID_LAST_STATUS_ARG_OPTIONAL 0
#define CMD_SID_CONN_REQUEST_ARG_REQUIRED 2
#define CMD_SID_CONN_REQUEST_ARG_OPTIONAL 0
#define CMD_SID_GET_TIME_ARG_REQUIRED 2
#define CMD_SID_GET_TIME_ARG_OPTIONAL 0
#define CMD_SID_SET_DST_ID_ARG_REQUIRED 2
#define CMD_SID_SET_DST_ID_ARG_OPTIONAL 0
#define CMD_SID_SET_SEND_LINK_ARG_REQUIRED 2
#define CMD_SID_SET_SEND_LINK_ARG_OPTIONAL 0
#define CMD_SID_SET_RSP_ID_ARG_REQUIRED 2
#define CMD_SID_SET_RSP_ID_ARG_OPTIONAL 0
#define CMD_SID_SDK_VERSION_DESCRIPTION_ARG_REQUIRED 1
#define CMD_SID_SDK_VERSION_DESCRIPTION_ARG_OPTIONAL 0
#define CMD_SID_SDK_CONFIG_DESCRIPTION_ARG_REQUIRED 1
#define CMD_SID_SDK_CONFIG_DESCRIPTION_ARG_OPTIONAL 0

int cmd_sid_init(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_deinit(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_start(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_stop(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_send(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_factory_reset(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_get_mtu(const struct shell *shell, int32_t argc, const char **argv);

int cmd_sid_option_battery(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_option_lp_set(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_option_lp_get_l2(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_option_lp_get_l3(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_option_d(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_option_gd(const struct shell *shell, int32_t argc, const char **argv);

int cmd_sid_last_status(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_conn_request(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_get_time(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_set_dst_id(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_set_send_link(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_set_rsp_id(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_sdk_version(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sid_sdk_config(const struct shell *shell, int32_t argc, const char **argv);

struct cli_config {
	struct sid_config *sid_cfg;
	struct app_context *app_cxt;
	enum sid_link_type send_link_type;
	uint16_t rsp_msg_id;
};

void initialize_sidewalk_shell(struct sid_config *sid_cfg, struct app_context *app_cxt);

#endif /* SID_DUT_SHELL_H */
