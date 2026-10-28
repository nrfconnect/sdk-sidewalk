/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef LOCATION_SHELL_EVENTS_H
#define LOCATION_SHELL_EVENTS_H

#include <stddef.h>
#include <stdint.h>
#include <sid_location.h>
#include <sidewalk.h>

/* Context of the location_event_send_levels event. */
struct location_send_levels_params {
	uint8_t levels[SID_LOCATION_MAX_LEVELS];
	uint8_t num_levels;
};

/* Context of the location_event_send_pseudorange event. */
struct location_send_pseudorange_params {
	uint8_t data[SID_LOCATION_MAX_PAYLOAD_SIZE];
	size_t size;
};

/* Location Event Handlers */
void location_event_init(sidewalk_ctx_t *sid, void *ctx);
void location_event_deinit(sidewalk_ctx_t *sid, void *ctx);
void location_event_send(sidewalk_ctx_t *sid, void *ctx);
void location_event_scan(sidewalk_ctx_t *sid, void *ctx);
void location_event_send_levels(sidewalk_ctx_t *sid, void *ctx);
void location_event_send_buffer(sidewalk_ctx_t *sid, void *ctx);
void location_event_send_pseudorange(sidewalk_ctx_t *sid, void *ctx);
void location_event_manage_effort(sidewalk_ctx_t *sid, void *ctx);

#endif /* LOCATION_SHELL_EVENTS_H */
