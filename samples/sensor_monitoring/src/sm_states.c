/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sm_states.h>

const char *app_sidewalk_state_to_str(const enum app_sidewalk_state state)
{
	switch (state) {
	case STATE_SIDEWALK_INIT:
		return "STATE_SIDEWALK_INIT";
	case STATE_SIDEWALK_READY:
		return "STATE_SIDEWALK_READY";
	case STATE_SIDEWALK_NOT_READY:
		return "STATE_SIDEWALK_NOT_READY";
	case STATE_SIDEWALK_SECURE_CONNECTION:
		return "STATE_SIDEWALK_SECURE_CONNECTION";
	default:
		return "!!! Invalid Sidewalk state !!!";
	}
}

const char *demo_app_state_to_string(const enum demo_app_state state)
{
	switch (state) {
	case DEMO_APP_STATE_INIT:
		return "DEMO_APP_STATE_INIT";
	case DEMO_APP_STATE_REGISTERED:
		return "DEMO_APP_STATE_REGISTERED";
	case DEMO_APP_STATE_NOTIFY_CAPABILITY:
		return "DEMO_APP_STATE_NOTIFY_CAPABILITY";
	case DEMO_APP_STATE_NOTIFY_SENSOR_DATA:
		return "DEMO_APP_STATE_NOTIFY_SENSOR_DATA";
	default:
		return "!!! Invalid demo app state !!!";
	}
}
