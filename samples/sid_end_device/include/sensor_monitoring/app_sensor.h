/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <stdint.h>

/**
 * @brief Get current temperature from device sensor.
 *
 * @param[out] temp - sensor temperature in Celsius degrees.
 * @return 0 if successful, negative errno code if failure.
 */
int app_sensor_temperature_get(int16_t *temp);

#endif /* APP_SENSOR_H */
