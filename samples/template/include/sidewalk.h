/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>

typedef struct sidewalk_ctx_s {
	struct sid_handle *handle;
	struct sid_config config;
} sidewalk_ctx_t;

typedef struct sidewalk_msg_s {
	struct sid_msg msg;
	struct sid_msg_desc desc;
} sidewalk_msg_t;

typedef enum sidewalk_events {
	SID_EVENT_SIDEWALK,
	SID_EVENT_STATE_NOT_READY,
	SID_EVENT_STATE_READY,
	SID_EVENT_STATE_ERROR,
	SID_EVENT_FACTORY_RESET,
	SID_EVENT_SEND_MSG,
	SID_EVENT_CONNECT,
	SID_EVENT_LINK_SWITCH,
	SID_EVENT_NORDIC_DFU,
} sidewalk_event_t;

void sidewalk_start(sidewalk_ctx_t *context);

int sidewalk_msg_set(sidewalk_msg_t *msg);

int sidewalk_event_send(sidewalk_event_t event);
