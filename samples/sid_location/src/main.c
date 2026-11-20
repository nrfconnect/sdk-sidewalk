/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app.h>
#include <sidewalk_version.h>
#include <zephyr/logging/log.h>

int main(void)
{
	PRINT_SIDEWALK_VERSION();
	app_start();
	return 0;
}
