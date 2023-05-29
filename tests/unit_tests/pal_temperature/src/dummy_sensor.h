/**
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * 
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __TEST_DUMMY_SENSOR_H__
#define __TEST_DUMMY_SENSOR_H__

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

#define DUMMY_SENSOR_NAME "dummy_sensor"

/**
 * @brief mock immplementation for dummy sensor
 * 
 * see sensor_sample_fetch() for argument description
 */
int dummy_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan);

/**
 * @brief mock immplementation for dummy sensor
 * 
 * see sensor_channel_get() for argument description
 */
int dummy_sensor_channel_get(const struct device *dev, enum sensor_channel chan,
			     struct sensor_value *val);

#endif //__TEST_DUMMY_SENSOR_H__
