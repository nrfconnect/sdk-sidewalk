/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <sid_api.h>
#include <sidewalk.h>

/**
 * @brief Initilize Sidewalk Bulk Data Transfer module.
 * 
 * @param handle Sidewalk handle given by sid_init.
 */
void app_file_transfer_demo_init(struct sid_handle *handle);

/**
 * @brief Deinitilize Sidewalk Bulk Data Transfer module.
 * 
 * @param handle Sidewalk handle given by sid_init.
 */
void app_file_transfer_demo_deinit(struct sid_handle *handle);

void sid_sidewalk_event_file_transfer_handler(void *ctx, void *state_machine);
#define SID_EVENT_FILE_TRANSFER                                                                    \
	APP_SIDEWALK_EVENT(SID_EVENT_FILE_TRANSFER,                                                \
			   { [STATE_SIDEWALK] = sid_sidewalk_event_file_transfer_handler })


#endif /* FILE_TRANSFER_H */
