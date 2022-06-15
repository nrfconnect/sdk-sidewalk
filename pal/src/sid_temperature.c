/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_temperature.c
 *  @brief Temperature interface implementation
 */

#include <sid_pal_temperature_ifc.h>
#include <stdint.h>
#include <device.h>
#include <drivers/sensor.h>

static const struct device *temp_dev;

sid_error_t sid_pal_temperature_init(void)
{
	temp_dev = device_get_binding(CONFIG_SIDEWALK_TEMPERATURE_SENSOR_NAME);

	return NULL == temp_dev ? SID_ERROR_NOSUPPORT : SID_ERROR_NONE;
}

int16_t sid_pal_temperature_get(void)
{
	struct sensor_value val = { 0, 0 };

	if (NULL == temp_dev) {
		return INT16_MIN;
	}
	int rc = sensor_sample_fetch(temp_dev);

	if (0 != rc) {
		return INT16_MIN;
	}

	rc = sensor_channel_get(temp_dev, SENSOR_CHAN_DIE_TEMP, &val);
	if (0 != rc) {
		return INT16_MIN;
	}

	return val.val1;
}
