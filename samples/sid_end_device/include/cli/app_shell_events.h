/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_DUT_H
#define APP_DUT_H

#include <sidewalk.h>

void dut_sidewalk_event_init_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_deinit_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_start_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_stop_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_get_mtu_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_get_time_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_get_status_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_get_option_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_set_option_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_set_dest_id_handler(void *ctx, void *state_machine);
void dut_sidewalk_event_set_conn_req_handler(void *ctx, void *state_machine);

#define DUT_EVENT_INIT                                                                             \
	APP_SIDEWALK_EVENT(DUT_EVENT_INIT, { [STATE_SIDEWALK] = dut_sidewalk_event_init_handler })
#define DUT_EVENT_DEINIT                                                                           \
	APP_SIDEWALK_EVENT(DUT_EVENT_DEINIT,                                                       \
			   { [STATE_SIDEWALK] = dut_sidewalk_event_deinit_handler })
#define DUT_EVENT_START                                                                            \
	APP_SIDEWALK_EVENT(DUT_EVENT_START, { [STATE_SIDEWALK] = dut_sidewalk_event_start_handler })
#define DUT_EVENT_STOP                                                                             \
	APP_SIDEWALK_EVENT(DUT_EVENT_STOP, { [STATE_SIDEWALK] = dut_sidewalk_event_stop_handler })
#define DUT_EVENT_GET_MTU                                                                          \
	APP_SIDEWALK_EVENT(DUT_EVENT_GET_MTU,                                                      \
			   { [STATE_SIDEWALK] = dut_sidewalk_event_get_mtu_handler })
#define DUT_EVENT_GET_TIME                                                                         \
	APP_SIDEWALK_EVENT(DUT_EVENT_GET_TIME,                                                     \
			   { [STATE_SIDEWALK] = dut_sidewalk_event_get_time_handler })
#define DUT_EVENT_GET_STATUS                                                                       \
	APP_SIDEWALK_EVENT(DUT_EVENT_GET_STATUS,                                                   \
			   { [STATE_SIDEWALK] = dut_sidewalk_event_get_status_handler })
#define DUT_EVENT_GET_OPTION                                                                       \
	APP_SIDEWALK_EVENT(DUT_EVENT_GET_OPTION,                                                   \
			   { [STATE_SIDEWALK] = dut_sidewalk_event_get_option_handler })
#define DUT_EVENT_SET_OPTION                                                                       \
	APP_SIDEWALK_EVENT(DUT_EVENT_SET_OPTION,                                                   \
			   { [STATE_SIDEWALK] = dut_sidewalk_event_set_option_handler })
#define DUT_EVENT_SET_DEST_ID                                                                      \
	APP_SIDEWALK_EVENT(DUT_EVENT_SET_DEST_ID,                                                  \
			   { [STATE_SIDEWALK] = dut_sidewalk_event_set_dest_id_handler })
#define DUT_EVENT_SET_CONN_REQ                                                                     \
	APP_SIDEWALK_EVENT(DUT_EVENT_SET_CONN_REQ,                                                 \
			   { [STATE_SIDEWALK] = dut_sidewalk_event_set_conn_req_handler })

#endif /* APP_DUT_H */
