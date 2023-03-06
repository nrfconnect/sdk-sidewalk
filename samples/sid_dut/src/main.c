/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr/logging/log.h>

#include <sidewalk_version.h>

#include <sid_error.h>
#include <sid_dut_shell.h>
#include <sid_thread.h>
#include <sid_api_delegated.h>
#include <board_init.h>

LOG_MODULE_REGISTER(sid_main, LOG_LEVEL_INF);

void main(void)
{
	PRINT_SIDEWALK_VERSION();

	if(sidewalk_board_init() != SID_ERROR_NONE)
	{return;}

	struct k_work_q * workq = sid_thread_init();
	sid_api_delegated_init(workq);
	
	initialize_sidewalk_shell(get_sidewalk_config(), get_sidewalk_handle());
}
