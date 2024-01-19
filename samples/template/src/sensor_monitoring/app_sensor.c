/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sensor_monitoring/app_sensor.h>
#include <errno.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>

#ifdef CONFIG_BOARD_THINGY53_NRF5340_CPUAPP
#define TEMP_CHANNEL SENSOR_CHAN_AMBIENT_TEMP
static const struct device *temp_dev = DEVICE_DT_GET_ONE(bosch_bme680);
#elif CONFIG_TEMP_NRF5
#define TEMP_CHANNEL SENSOR_CHAN_DIE_TEMP
static const struct device *temp_dev = DEVICE_DT_GET_ONE(nordic_nrf_temp);
#else
#define TEMP_CHANNEL SENSOR_CHAN_ALL
static const struct device *temp_dev;
#endif

int app_sensor_temperature_get(int16_t *temp)
{
	if (!temp) {
		return -EINVAL;
	};

	if (!device_is_ready(temp_dev)) {
		return -EIO;
	}

	int err = sensor_sample_fetch(temp_dev);
	if (err) {
		return -EIO;
	}

	struct sensor_value sensor = { 0, 0 };
	err = sensor_channel_get(temp_dev, TEMP_CHANNEL, &sensor);
	if (err) {
		return -EFAULT;
	}

	*temp = (int16_t)sensor.val1;
	return 0;
}
