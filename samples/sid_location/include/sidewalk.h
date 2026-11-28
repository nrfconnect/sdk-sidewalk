/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_LOCATION_SIDEWALK_H
#define SID_LOCATION_SIDEWALK_H

#include <sid_api.h>
#include <sid_location.h>

#define SIDEWALK_BLE_CONNECT_TIMEOUT_S 30

/* Called from the Sidewalk stack context, must not block */
struct sidewalk_callbacks {
	void (*on_process)(void);
	void (*on_status)(const struct sid_status *status);
	void (*on_location)(const struct sid_location_result *result);
};

int sidewalk_init(const struct sidewalk_callbacks *callbacks);
void sidewalk_process(void);
int sidewalk_start(void);
int sidewalk_stop(void);
void sidewalk_ble_connect_request(void);

int sidewalk_location_init(void);
int sidewalk_location_run_auto(void);
int sidewalk_location_run_level(enum sid_location_effort_mode mode);
enum sid_location_effort_mode sidewalk_location_effort(void);
int sidewalk_location_msg_send(void);

#endif /* SID_LOCATION_SIDEWALK_H */
