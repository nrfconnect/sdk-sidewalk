/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sm_task.h>
#include <sidewalk_version.h>
#include <zephyr/logging/log.h>
#include <board_init.h>

#include <sm_buttons.h>
#include <sm_leds.h>

LOG_MODULE_REGISTER(sm_main, CONFIG_SIDEWALK_LOG_LEVEL);

static int application_board_init(){
	int err = sm_buttons_init();

	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
		return err;
	}

	err = sm_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return err;
	}
	return 0;
}

void main(void)
{
	PRINT_SIDEWALK_VERSION();

	if (sidewalk_board_init()) {
		LOG_INF("Board init failed.");
		return;
	}
	if (application_board_init()) {
		LOG_INF("Board init failed.");
		return;
	}

	LOG_INF("Sidewalk demo started!");

	sm_task_start();
}
