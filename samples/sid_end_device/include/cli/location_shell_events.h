/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef LOCATION_SHELL_EVENTS_H
#define LOCATION_SHELL_EVENTS_H

#include <sidewalk.h>

/* DULT Event Handlers */
void dult_event_init(sidewalk_ctx_t *sid, void *ctx);
void dult_event_deinit(sidewalk_ctx_t *sid, void *ctx);
void dult_event_config(sidewalk_ctx_t *sid, void *ctx);
void dult_event_set(sidewalk_ctx_t *sid, void *ctx);

/* Location Event Handlers */
void location_event_init(sidewalk_ctx_t *sid, void *ctx);
void location_event_deinit(sidewalk_ctx_t *sid, void *ctx);
void location_event_send(sidewalk_ctx_t *sid, void *ctx);
void location_event_scan(sidewalk_ctx_t *sid, void *ctx);
void location_event_send_buffer(sidewalk_ctx_t *sid, void *ctx);
void location_event_alm_start(sidewalk_ctx_t *sid, void *ctx);

#endif /* LOCATION_SHELL_EVENTS_H */
