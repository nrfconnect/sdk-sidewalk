/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SIDEWALK_MOCK_H
#define SIDEWALK_MOCK_H

#include <zephyr/fff.h>
#include <sidewalk.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(sidewalk_start, sidewalk_ctx_t *);
FAKE_VALUE_FUNC(void *, sid_hal_malloc, size_t);
FAKE_VOID_FUNC(sid_hal_free, void *);
FAKE_VALUE_FUNC(int, sidewalk_event_send, sidewalk_event_t, void *, ctx_free);

#define SIDEWALK_FAKES_LIST(FAKE)                                                                  \
	FAKE(sidewalk_start)                                                                       \
	FAKE(sid_hal_malloc)                                                                       \
	FAKE(sid_hal_free)                                                                         \
	FAKE(sidewalk_event_send)

#endif /* SIDEWALK_MOCK_H */
