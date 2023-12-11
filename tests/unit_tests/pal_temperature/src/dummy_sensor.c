/**
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * 
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "dummy_sensor.h"
#include "zephyr/toolchain/gcc.h"
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>
#include <errno.h>

LOG_MODULE_REGISTER(dummy_sensor, LOG_LEVEL_DBG);

static const struct sensor_driver_api dummy_sensor_api = {
	.sample_fetch = &dummy_sensor_sample_fetch,
	.channel_get = &dummy_sensor_channel_get,
};

int dummy_sensor_init(const struct device *dev)
{
	return 0;
}

DEVICE_DEFINE(dummy_sensor, DUMMY_SENSOR_NAME, &dummy_sensor_init, NULL, NULL, NULL, POST_KERNEL,
	      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &dummy_sensor_api);
