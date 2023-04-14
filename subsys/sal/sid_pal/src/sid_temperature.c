/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_temperature.c
 *  @brief Temperature interface implementation
 */
 #include <zephyr/kernel.h>
 #include <sid_pal_temperature_ifc.h>
 #if defined (NRF5340_XXAA)

sid_error_t sid_pal_temperature_init(void)
{
	return SID_ERROR_NONE;
}

int16_t sid_pal_temperature_get(void)
{
	return 25;
}

 #else

#include "zephyr/devicetree.h"
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

static const struct device *temp_dev;

sid_error_t sid_pal_temperature_init(void)
{
	temp_dev = device_get_binding(DT_NODE_FULL_NAME(DT_NODELABEL(temp)));

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

#endif
