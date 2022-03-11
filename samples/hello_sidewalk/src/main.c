/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_log_ifc.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);

void main(void)
{
	LOG_INF("Hello Sidewalk World! %s\n", CONFIG_BOARD);

	if (SID_ERROR_NONE == sid_pal_storage_kv_init()) {
		LOG_INF("NVM memory is ready!\n");
	}

	
	SID_PAL_LOG_INFO("Hello Sidewalk Log! %d", 2022);
}
