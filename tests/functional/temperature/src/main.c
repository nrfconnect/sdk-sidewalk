/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <zephyr/kernel.h>
#include <stdint.h>
#include <string.h>

#include "sid_pal_temperature_ifc.h"

#include <stdlib.h>

#define MAX_ABS_ERROR 1

static void test_initialize()
{
	zassert_equal(SID_ERROR_NONE, sid_pal_temperature_init());
}
static void validate_temp_reading(int16_t value)
{
	zassert_between_inclusive(value, -40, 85);
}

ZTEST(temperature, test_valid_temperature_range_for_SoC)
{
	test_initialize();
	int16_t die_temp = sid_pal_temperature_get();
	printk("die temperature = %d", die_temp);
	validate_temp_reading(die_temp);
}

static void analyze_temp_readings(int16_t *values, size_t count, double limit)
{
	double mean;
	int64_t sum = 0;

	for (int i = 0; i < count; i++) {
		sum += values[i];
		validate_temp_reading(values[i]);
	}

	mean = (double)sum / count;
	double max_mean_error = 0;

	for (int i = 0; i < count; i++) {
		double err = values[i] - mean;
		if (err < 0) {
			err = -err;
		}
		if (err > max_mean_error) {
			max_mean_error = err;
		}
	}

	zassert_true(max_mean_error <= limit);

	printk("mean reading= %d.%02d, max deviation = %d.%02d", (uint8_t)mean,
	       (uint8_t)((mean - (uint8_t)mean) * 100), (uint8_t)max_mean_error,
	       (uint8_t)((max_mean_error - (uint8_t)max_mean_error) * 100));
}

ZTEST(temperature, test_multiple_fast_readings)
{
	test_initialize();
	int16_t results[50] = { 0 };

	for (int i = 0; i < ARRAY_SIZE(results); i++) {
		results[i] = sid_pal_temperature_get();
	}

	analyze_temp_readings(results, ARRAY_SIZE(results), MAX_ABS_ERROR);
}

ZTEST(temperature, test_multiple_slow_readings)
{
	test_initialize();
	int16_t results[20] = { 0 };

	for (int i = 0; i < ARRAY_SIZE(results); i++) {
		results[i] = sid_pal_temperature_get();
		k_sleep(K_MSEC(50));
	}
	analyze_temp_readings(results, ARRAY_SIZE(results), MAX_ABS_ERROR);
}

ZTEST(temperature, test_sanity)
{
	zassert_equal(true, true);
}

ZTEST_SUITE(temperature, NULL, NULL, NULL, NULL, NULL);
