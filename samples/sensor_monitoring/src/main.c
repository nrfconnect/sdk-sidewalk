/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sm_task.h>
#include <sm_buttons.h>
#include <sm_leds.h>
#include <pal_init.h>
#include <sidewalk_version.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_main, CONFIG_SIDEWALK_LOG_LEVEL);

static int init_leds_and_buttons()
{
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
	return err;
}

int main(void)
{
	int err;

	PRINT_SIDEWALK_VERSION();

	err = init_leds_and_buttons();
	if (err) {
		LOG_INF("Buttons or LEDs faild to initialize.");
		return err;
	}

	err = application_pal_init();
	if (err) {
		LOG_INF("Failed to initialize PAL layer.");
		return err;
	}

	LOG_INF("Sidewalk demo started!");

	sm_task_start();

	return 0;
}
