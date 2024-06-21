/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef SIDEWALK_APP_H
#define SIDEWALK_APP_H

#include <sid_api.h>

#include <zephyr/kernel.h>
#include <zephyr/smf.h>

enum sidewalk_fsm_states {
	STATE_SIDEWALK,
	STATE_DFU,

	STATE_LAST
};
typedef struct {
	struct sid_handle *handle;
	struct sid_config config;
	struct sid_status last_status;
} sidewalk_ctx_t;

typedef struct {
	const char *name;
	void (*call[STATE_LAST])(void *ctx, void *state_machine);
} sidewalk_event_t;
typedef struct {
	sidewalk_event_t event_handler;
	void *ctx;
} sidewalk_ctx_event_t;

typedef struct sm_s {
	struct smf_ctx ctx;
	struct smf_state sid_states[STATE_LAST];
	struct k_msgq msgq;
	sidewalk_ctx_event_t application_event;
	sidewalk_ctx_t *sid;
} sm_t;

#define APP_SIDEWALK_EVENT(event_name, event_handlers...)                                          \
	(sidewalk_event_t)                                                                         \
	{                                                                                          \
		.name = #event_name, .call = event_handlers                                        \
	}

void sid_sidewalk_event_process_handler(void *ctx, void *state_machine);
void sid_sidewalk_event_autoconnect_handler(void *ctx, void *state_machine);
void sid_sidewalk_event_factory_reset_handler(void *ctx, void *state_machine);
void sid_sidewalk_event_link_switch_handler(void *ctx, void *state_machine);
void sid_sidewalk_event_nordic_dfu_handler(void *ctx, void *state_machine);
void sid_dfu_event_nordic_dfu_handler(void *ctx, void *state_machine);
void sid_sidewalk_event_new_status_handler(void *ctx, void *state_machine);
void sid_dfu_event_new_status_handler(void *ctx, void *state_machine);
void sid_sidewalk_event_send_message_handler(void *ctx, void *state_machine);
void sid_dfu_event_send_message_handler(void *ctx, void *state_machine);
void sid_sidewalk_event_connect_handler(void *ctx, void *state_machine);

#define SID_EVENT_SIDEWALK                                                                         \
	APP_SIDEWALK_EVENT(SID_EVENT_SIDEWALK,                                                     \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_process_handler })
#define SID_EVENT_AUTOCONNECT                                                                      \
	APP_SIDEWALK_EVENT(SID_EVENT_AUTOCONNECT,                                                  \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_autoconnect_handler })
#define SID_EVENT_FACTORY_RESET                                                                    \
	APP_SIDEWALK_EVENT(SID_EVENT_FACTORY_RESET,                                                \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_factory_reset_handler })
#define SID_EVENT_LINK_SWITCH                                                                      \
	APP_SIDEWALK_EVENT(SID_EVENT_LINK_SWITCH,                                                  \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_link_switch_handler })
#define SID_EVENT_NORDIC_DFU                                                                       \
	APP_SIDEWALK_EVENT(SID_EVENT_NORDIC_DFU,                                                   \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_nordic_dfu_handler,             \
			     [STATE_DFU] = sid_dfu_event_nordic_dfu_handler })
#define SID_EVENT_NEW_STATUS                                                                       \
	APP_SIDEWALK_EVENT(SID_EVENT_NEW_STATUS,                                                   \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_new_status_handler,             \
			     [STATE_DFU] = sid_dfu_event_new_status_handler })
#define SID_EVENT_SEND_MSG                                                                         \
	APP_SIDEWALK_EVENT(SID_EVENT_SEND_MSG,                                                     \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_send_message_handler,           \
			     [STATE_DFU] = sid_dfu_event_send_message_handler })
#define SID_EVENT_CONNECT                                                                          \
	APP_SIDEWALK_EVENT(SID_EVENT_CONNECT,                                                      \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_connect_handler })

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


#ifdef CONFIG_SIDEWALK_LINK_MASK_BLE
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1)
#elif CONFIG_SIDEWALK_LINK_MASK_FSK
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_2)
#elif CONFIG_SIDEWALK_LINK_MASK_LORA
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1 | SID_LINK_TYPE_3)
#else
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1)
#endif

void sidewalk_start(sidewalk_ctx_t *context);

int sidewalk_event_send(sidewalk_event_t event, void *ctx);

void push_message_buffer(sidewalk_msg_t *msg);
sidewalk_msg_t *get_message_buffer(uint16_t message_id);

#endif /* SIDEWALK_APP_H */
