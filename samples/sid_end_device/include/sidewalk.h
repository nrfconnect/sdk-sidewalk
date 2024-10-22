/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef SIDEWALK_APP_H
#define SIDEWALK_APP_H

#include <sid_api.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/slist.h>

typedef struct {
	struct sid_handle *handle;
	struct sid_config config;
	struct sid_status last_status;
} sidewalk_ctx_t;

typedef void (*event_handler_t)(sidewalk_ctx_t *sid, void *ctx);
typedef void (*ctx_free)(void *ctx);
typedef struct {
	event_handler_t handler;
	void *ctx;
	ctx_free ctx_free;
} sidewalk_ctx_event_t;

typedef struct {
	sys_snode_t node;
	struct sid_msg msg;
	struct sid_msg_desc desc;
} sidewalk_msg_t;

typedef struct {
	enum sid_option option;
	void *data;
	size_t data_len;
} sidewalk_option_t;

typedef struct {
	uint32_t file_id;
	size_t file_offset;
	void *data;
	size_t data_size;
} sidewalk_transfer_t;

void sidewalk_start(sidewalk_ctx_t *context);

int sidewalk_event_send(event_handler_t event, void *ctx, ctx_free free);

#ifdef CONFIG_SIDEWALK_LINK_MASK_BLE
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1)
#elif CONFIG_SIDEWALK_LINK_MASK_FSK
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_2)
#elif CONFIG_SIDEWALK_LINK_MASK_LORA
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1 | SID_LINK_TYPE_3)
#else
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1)
#endif

void sidewalk_event_process(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_autostart(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_factory_reset(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_new_status(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_send_msg(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_connect(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_link_switch(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_exit(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_reboot(sidewalk_ctx_t *sid, void *ctx);
void sidewalk_event_platform_init(sidewalk_ctx_t *sid, void *ctx);
#endif /* SIDEWALK_APP_H */
