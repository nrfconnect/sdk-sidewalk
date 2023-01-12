/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef SIDEWALK_THREAD_H
#define SIDEWALK_THREAD_H
#include <sid_api.h>

enum event_type {
	EVENT_TYPE_SIDEWALK,
	EVENT_TYPE_SEND_HELLO,
	EVENT_TYPE_SET_BATTERY_LEVEL,
	EVENT_TYPE_NORDIC_DFU,
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	EVENT_TYPE_CONNECTION_REQUEST,
#else /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
	EVENT_TYPE_SET_DEVICE_PROFILE,
#endif /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
	EVENT_TYPE_FACTORY_RESET,
};

enum app_state {
	STATE_INIT,
	STATE_SIDEWALK_READY,
	STATE_SIDEWALK_NOT_READY,
	STATE_SIDEWALK_SECURE_CONNECTION,
	STATE_PAL_INIT_ERROR,
	STATE_LIB_INIT_ERROR,
	STATE_NORDIC_DFU
};

struct link_status {
	enum sid_registration_status link_mask;
	enum sid_link_mode supported_link_mode[SID_LINK_TYPE_MAX_IDX];
};

typedef struct app_context {
	struct sid_handle *sidewalk_handle;
	struct sid_config sidewalk_config;
	enum app_state state;
	struct link_status link_status;
	uint8_t counter;
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	bool connection_request;
#endif /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
} app_context_t;

/**
 * @brief Function for starting sidewalk thread.
 *
 * @return app_context_t* poiner to applicaiton context
 */
app_context_t* sidewalk_thread_enable(void);

/**
 * @brief Add message to thread queue.
 *
 * @param event
 */
void sidewalk_thread_message_q_write(enum event_type event);

#endif /* SIDEWALK_THREAD_H */
