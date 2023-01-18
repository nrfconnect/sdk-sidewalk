/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <board_config.h>
#include <sm_task.h>
#include <sidewalk_version.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_main, CONFIG_SIDEWALK_LOG_LEVEL);

void main(void)
{
	PRINT_SIDEWALK_VERSION();

	if (board_config()) {
		LOG_INF("Board init failed.");
		return;
	}

	LOG_INF("Sidewalk demo started!");

	sm_task_start();
}
