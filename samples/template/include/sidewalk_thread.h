/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef SIDEWALK_THREAD_H
#define SIDEWALK_THREAD_H

enum event_type {
	EVENT_TYPE_SIDEWALK,
	EVENT_TYPE_SEND_HELLO,
	EVENT_TYPE_SET_BATTERY_LEVEL,
	EVENT_TYPE_FACTORY_RESET,
	EVENT_TYPE_CONNECTION_REQUEST,
};

/**
 * @brief Function for starting sidewalk thread.
 *
 */
void sidewalk_thread_enable(void);

/**
 * @brief Add message to thread queue.
 *
 * @param event
 */
void sidewalk_thread_message_q_write(enum event_type event);

#endif /* SIDEWALK_THREAD_H */
