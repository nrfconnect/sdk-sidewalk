/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SM_STATES_H
#define SM_STATES_H

enum app_sidewalk_state {
	STATE_SIDEWALK_INIT,
	STATE_SIDEWALK_READY,
	STATE_SIDEWALK_NOT_READY,
	STATE_SIDEWALK_SECURE_CONNECTION,
};

enum demo_app_state {
	DEMO_APP_STATE_INIT,
	DEMO_APP_STATE_REGISTERED,
	DEMO_APP_STATE_NOTIFY_CAPABILITY,
	DEMO_APP_STATE_NOTIFY_SENSOR_DATA,
};

#endif /* SM_STATES_H */
