/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/logging/log.h>

#include <sidewalk_version.h>

#include <sid_error.h>
#include <sid_dut_shell.h>
#include <sid_thread.h>

LOG_MODULE_REGISTER(sid_main, LOG_LEVEL_INF);

int main(void)
{
	PRINT_SIDEWALK_VERSION();

	initialize_sidewalk_shell(sid_thread_get_config(), sid_thread_get_app_context());

	sid_error_t e = sid_thread_init();

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sidwalk failed to initialize with error (sid_error_t) %d", e);
	}

	return 0;
}
