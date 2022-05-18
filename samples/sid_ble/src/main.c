/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

void main(void)
{
	LOG_INF("Hello Sidewalk World! %s\n", CONFIG_BOARD);
}
