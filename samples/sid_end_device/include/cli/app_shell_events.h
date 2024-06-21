/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_DUT_H
#define APP_DUT_H

#include <sidewalk.h>

void sidewalk_event_init(void *ctx, sm_t *sm);
void sidewalk_event_deinit(void *ctx, sm_t *sm);
void sidewalk_event_start(void *ctx, sm_t *sm);
void sidewalk_event_stop(void *ctx, sm_t *sm);
void sidewalk_event_get_mtu(void *ctx, sm_t *sm);
void sidewalk_event_get_time(void *ctx, sm_t *sm);
void sidewalk_event_get_status(void *ctx, sm_t *sm);
void sidewalk_event_get_option(void *ctx, sm_t *sm);
void sidewalk_event_set_option(void *ctx, sm_t *sm);
void sidewalk_event_set_dest_id(void *ctx, sm_t *sm);
void sidewalk_event_set_conn_req(void *ctx, sm_t *sm);

#define DUT_EVENT_INIT                                                                             \
	APP_SIDEWALK_EVENT(DUT_EVENT_INIT, { [STATE_SIDEWALK] = sidewalk_event_init })
#define DUT_EVENT_DEINIT                                                                           \
	APP_SIDEWALK_EVENT(DUT_EVENT_DEINIT, { [STATE_SIDEWALK] = sidewalk_event_deinit })
#define DUT_EVENT_START                                                                            \
	APP_SIDEWALK_EVENT(DUT_EVENT_START, { [STATE_SIDEWALK] = sidewalk_event_start })
#define DUT_EVENT_STOP                                                                             \
	APP_SIDEWALK_EVENT(DUT_EVENT_STOP, { [STATE_SIDEWALK] = sidewalk_event_stop })
#define DUT_EVENT_GET_MTU                                                                          \
	APP_SIDEWALK_EVENT(DUT_EVENT_GET_MTU, { [STATE_SIDEWALK] = sidewalk_event_get_mtu })
#define DUT_EVENT_GET_TIME                                                                         \
	APP_SIDEWALK_EVENT(DUT_EVENT_GET_TIME, { [STATE_SIDEWALK] = sidewalk_event_get_time })
#define DUT_EVENT_GET_STATUS                                                                       \
	APP_SIDEWALK_EVENT(DUT_EVENT_GET_STATUS, { [STATE_SIDEWALK] = sidewalk_event_get_status })
#define DUT_EVENT_GET_OPTION                                                                       \
	APP_SIDEWALK_EVENT(DUT_EVENT_GET_OPTION, { [STATE_SIDEWALK] = sidewalk_event_get_option })
#define DUT_EVENT_SET_OPTION                                                                       \
	APP_SIDEWALK_EVENT(DUT_EVENT_SET_OPTION, { [STATE_SIDEWALK] = sidewalk_event_set_option })
#define DUT_EVENT_SET_DEST_ID                                                                      \
	APP_SIDEWALK_EVENT(DUT_EVENT_SET_DEST_ID, { [STATE_SIDEWALK] = sidewalk_event_set_dest_id })
#define DUT_EVENT_SET_CONN_REQ                                                                     \
	APP_SIDEWALK_EVENT(DUT_EVENT_SET_CONN_REQ,                                                 \
			   { [STATE_SIDEWALK] = sidewalk_event_set_conn_req })

#endif /* APP_DUT_H */
