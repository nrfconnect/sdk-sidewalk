/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sid_pal_storage_kv_ifc.h>

void main(void)
{
	printk("Hello Sidewalk World! %s\n", CONFIG_BOARD);
	if(SID_ERROR_NONE == sid_pal_storage_kv_init())
	{
		printk("NVM memory is ready!\n");
	}
}
