/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sid_pal_storage_kv_ifc.h>

#include "sid_pal_log_ifc.h"

#include <logging/log.h>
LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);

void main(void)
{
	printk("Hello Sidewalk World! %s\n", CONFIG_BOARD);
	if (SID_ERROR_NONE == sid_pal_storage_kv_init()) {
		printk("NVM memory is ready!\n");
	}

	LOG_INF("Hello Sidewalk World! %s\n", CONFIG_BOARD);
	SID_PAL_LOG_INFO("Hello Sidewalk Log! %d", 2022);
}
