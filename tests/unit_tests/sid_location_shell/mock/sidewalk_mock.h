/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SIDEWALK_MOCK_H
#define SIDEWALK_MOCK_H

#include <zephyr/fff.h>
#include <sidewalk.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(void *, sid_hal_malloc, size_t);
FAKE_VOID_FUNC(sid_hal_free, void *);
FAKE_VALUE_FUNC(int, sidewalk_event_send, event_handler_t, void *, ctx_free);

FAKE_VOID_FUNC(location_event_init, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(location_event_deinit, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(location_event_send, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(location_event_send_levels, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(location_event_scan, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(location_event_send_buffer, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(location_event_send_pseudorange, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(location_event_manage_effort, sidewalk_ctx_t *, void *);

#define SIDEWALK_FAKES_LIST(FAKE)                                                                  \
	FAKE(sid_hal_malloc)                                                                       \
	FAKE(sid_hal_free)                                                                         \
	FAKE(sidewalk_event_send)

#endif /* SIDEWALK_MOCK_H */
