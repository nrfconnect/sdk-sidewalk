/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_DUT_H
#define APP_DUT_H

#include <sidewalk.h>

typedef enum {
	DUT_EVENT_INIT = SID_EVENT_LAST,
	DUT_EVENT_DEINIT,
	DUT_EVENT_START,
	DUT_EVENT_STOP,
	DUT_EVENT_GET_MTU,
	DUT_EVENT_GET_TIME,
	DUT_EVENT_GET_STATUS,
	DUT_EVENT_GET_OPTION,
	DUT_EVENT_SET_OPTION,
	DUT_EVENT_SET_DEST_ID,
	DUT_EVENT_SET_CONN_REQ,
} cli_event_t;

void app_dut_event_process(sidewalk_ctx_event_t event, sidewalk_ctx_t *sid);

#endif /* APP_DUT_H */
