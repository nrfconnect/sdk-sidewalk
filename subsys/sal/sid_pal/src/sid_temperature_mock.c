/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_temperature.c
 *  @brief Temperature interface implementation
 */

#include <sid_pal_temperature_ifc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_temp_mock, CONFIG_SIDEWALK_LOG_LEVEL);

#define MOCK_TEMP_VALUE (20)

sid_error_t sid_pal_temperature_init(void)
{
	return SID_ERROR_NONE;
}

int16_t sid_pal_temperature_get(void)
{
	LOG_INF("get mock temp value: %d", MOCK_TEMP_VALUE);
	return MOCK_TEMP_VALUE;
}
