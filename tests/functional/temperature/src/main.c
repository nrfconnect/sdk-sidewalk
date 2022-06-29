/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr.h>
#include <stdint.h>
#include <string.h>

#include "sid_pal_temperature_ifc.h"
#include <unity.h>
#include <stdlib.h>


#define MAX_ABS_ERROR 1

void test_initialize()
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_temperature_init());
}

void test_valid_temperature_range_for_SoC()
{
	test_initialize();
	int16_t die_temp = sid_pal_temperature_get();
	TEST_ASSERT_GREATER_OR_EQUAL(-40, die_temp);
	TEST_ASSERT_LESS_OR_EQUAL(85, die_temp);
	char message[50];
	sprintf(message, "die temperature = %d", die_temp);
	TEST_MESSAGE(message);
}

static void validate_temp_reading(int16_t value)
{
	TEST_ASSERT_GREATER_OR_EQUAL(-40, value);
	TEST_ASSERT_LESS_OR_EQUAL(85, value);
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

	TEST_ASSERT_LESS_OR_EQUAL(limit, max_mean_error);
	char message[50];
	sprintf(message, "mean reading= %d.%02d, max deviation = %d.%02d",
		(uint8_t)mean,
		(uint8_t)((mean - (uint8_t)mean) * 100),
		(uint8_t)max_mean_error,
		(uint8_t)((max_mean_error - (uint8_t)max_mean_error) * 100));
	TEST_MESSAGE(message);
}

void test_multiple_fast_readings()
{
	test_initialize();
	int16_t results[50] = { 0 };
	for (int i = 0; i < ARRAY_SIZE(results); i++) {
		results[i] = sid_pal_temperature_get();
	}

	analyze_temp_readings(results, ARRAY_SIZE(results), MAX_ABS_ERROR);

}

void test_multiple_slow_readings()
{
	test_initialize();
	int16_t results[20] = { 0 };
	for (int i = 0; i < ARRAY_SIZE(results); i++) {
		results[i] = sid_pal_temperature_get();
		k_sleep(K_MSEC(50));
	}
	analyze_temp_readings(results, ARRAY_SIZE(results), MAX_ABS_ERROR);
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
