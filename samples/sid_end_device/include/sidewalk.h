/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef SIDEWALK_APP_H
#define SIDEWALK_APP_H

#include <sid_api.h>

typedef enum {
	SID_EVENT_SIDEWALK,
	SID_EVENT_FACTORY_RESET,
	SID_EVENT_NEW_STATUS,
	SID_EVENT_SEND_MSG,
	SID_EVENT_CONNECT,
	SID_EVENT_LINK_SWITCH,
	SID_EVENT_NORDIC_DFU,
	SID_EVENT_LAST,
} sidewalk_event_t;

typedef struct {
	sidewalk_event_t id;
	void *ctx;
} sidewalk_ctx_event_t;

typedef struct {
	struct sid_handle *handle;
	struct sid_config config;
	struct sid_status last_status;
} sidewalk_ctx_t;

typedef struct {
	struct sid_msg msg;
	struct sid_msg_desc desc;
} sidewalk_msg_t;

typedef struct {
	enum sid_option option;
	void *data;
	size_t data_len;
} sidewalk_option_t;

void sidewalk_start(sidewalk_ctx_t *context);

void *sidewalk_data_alloc(size_t data_size);

void sidewalk_data_free(void *data);

int sidewalk_event_send(sidewalk_event_t event, void *ctx);

#endif /* SIDEWALK_APP_H */
