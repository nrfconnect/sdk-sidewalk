/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SM_TASK_H
#define SM_TASK_H

#include <sid_demo_parser.h>
#include <sm_events.h>
#include <sm_states.h>
#include <sm_utils.h>
#include <zephyr/kernel.h>

struct link_status {
	enum sid_time_sync_status time_sync_status;
	uint32_t link_mask;
	uint32_t supported_link_mode[SID_LINK_TYPE_MAX_IDX];
};

typedef struct app_context {
	struct sid_handle *sidewalk_handle;
	enum app_sidewalk_state sidewalk_state;
	enum demo_app_state app_state;
	struct link_status link_status;

	uint8_t buffer[PAYLOAD_MAX_SIZE];
} app_context_t;

/**
 * @brief Function for starting demo tasks.
 *
 */
void sm_task_start(void);

/**
 * @brief Add message to sidewalk queue.
 *
 * @param app_context application context.
 * @param desc message descriptor.
 * @param msg message payload.
 */
void sm_send_msg(const app_context_t *app_context, struct sid_msg_desc *desc, struct sid_msg *msg);

/**
 * @brief Get the active link type object.
 *
 * @param app_context application context.
 * @return link type.
 */
enum sid_link_type sm_active_link_type_get(const app_context_t *app_context);

#endif /* SM_TASK_H */
