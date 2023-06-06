/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_temperature.c
 *  @brief Temperature interface implementation
 */

#include <sid_pal_temperature_ifc.h>

sid_error_t sid_pal_temperature_init(void)
{
	return SID_ERROR_NONE;
}

int16_t sid_pal_temperature_get(void)
{
	return 20;
}
