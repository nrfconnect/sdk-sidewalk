/**
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * 
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/fff.h>
#include <zephyr/ztest.h>
#include <dummy_sensor.h>

#include <sid_pal_temperature_ifc.h>

char DT_N_NODELABEL_temp_FULL_NAME[50];
DEFINE_FFF_GLOBALS;

// NOLINTNEXTLINE (bugprone-sizeof-expression)
FAKE_VALUE_FUNC(int, dummy_sensor_sample_fetch, const struct device * /*dev*/,
		enum sensor_channel /*chan*/);

// NOLINTNEXTLINE (bugprone-sizeof-expression)
FAKE_VALUE_FUNC(int, dummy_sensor_channel_get, const struct device * /*dev*/,
		enum sensor_channel /*chan*/, struct sensor_value * /*val*/);

struct sensor_value custom_fake_dummy_sensor_channel_get_sensor_value;
static int custom_fake_dummy_sensor_channel_get(const struct device *dev, enum sensor_channel chan,
						struct sensor_value *val)
{
	memcpy(val, &custom_fake_dummy_sensor_channel_get_sensor_value, sizeof(*val));
	return dummy_sensor_channel_get_fake.return_val;
}

/**
 * @brief Create sensor_value instance
 * 
 * @param value - float value to be set into sensor_value structure
 */
#define SENSOR_VALUE_FLOAT(value)                                                                  \
	(struct sensor_value)                                                                      \
	{                                                                                          \
		.val1 = (int)value, .val2 = (int)(1000000 * ((double)value - (int)value))          \
	}

/**
 * @brief Set up for test.
 * 
 * @param f Pointer to fixture.
 */
void setUp(void *f)
{
	memset(DT_N_NODELABEL_temp_FULL_NAME, 0, sizeof(DT_N_NODELABEL_temp_FULL_NAME));
	/* Initialize with empty node name, to clean the driver internal state before test */
	sid_pal_temperature_init();
	strcpy(DT_N_NODELABEL_temp_FULL_NAME, "dummy_sensor");

	RESET_FAKE(dummy_sensor_sample_fetch);
	RESET_FAKE(dummy_sensor_channel_get);
}

ZTEST_SUITE(temperature, NULL, NULL, setUp, NULL, NULL);

ZTEST(temperature, test_sid_pal_temperature_init)
{
	sid_error_t error = sid_pal_temperature_init();
	zassert_equal(SID_ERROR_NONE, error, "Initialization failed with error %d, expected %d.",
		      error, SID_ERROR_NONE);
}

ZTEST(temperature, test_sid_pal_temperature_init_wrong_device)
{
	memset(DT_N_NODELABEL_temp_FULL_NAME, 0, sizeof(DT_N_NODELABEL_temp_FULL_NAME));
	sid_error_t error = sid_pal_temperature_init();
	zassert_equal(SID_ERROR_NOSUPPORT, error,
		      "Initialization failed with error %d, expected %d.", error,
		      SID_ERROR_NOSUPPORT);
}

ZTEST(temperature, test_get_value)
{
	dummy_sensor_sample_fetch_fake.return_val = 0;

	dummy_sensor_channel_get_fake.custom_fake = custom_fake_dummy_sensor_channel_get;
	dummy_sensor_channel_get_fake.return_val = 0;
	custom_fake_dummy_sensor_channel_get_sensor_value = SENSOR_VALUE_FLOAT(15.123);

	sid_error_t error = sid_pal_temperature_init();
	zassert_equal(SID_ERROR_NONE, error, "Initialization failed with error %d, expected %d.",
		      error, SID_ERROR_NONE);
	int16_t temperature_C = sid_pal_temperature_get();
	zassert_equal(15, temperature_C, "Unexpected temperature read %d, expected %d.",
		      temperature_C, 15);

	zassert_equal(1, dummy_sensor_sample_fetch_fake.call_count,
		      "dummy_sensor_sample_fetch called %d times, expected %d",
		      dummy_sensor_sample_fetch_fake.call_count, 1);
	zassert_equal(1, dummy_sensor_channel_get_fake.call_count,
		      "dummy_sensor_channel_get called %d times, expected %d",
		      dummy_sensor_channel_get_fake.call_count, 1);
}

ZTEST(temperature, test_get_value_uninitialized)
{
	memset(DT_N_NODELABEL_temp_FULL_NAME, 0, sizeof(DT_N_NODELABEL_temp_FULL_NAME));

	sid_error_t error = sid_pal_temperature_init();
	zassert_equal(SID_ERROR_NOSUPPORT, error,
		      "Initialization failed with error %d, expected %d.", error,
		      SID_ERROR_NOSUPPORT);
	int16_t temperature_C = sid_pal_temperature_get();
	zassert_equal(INT16_MIN, temperature_C, "Unexpected temperature read %d, expected %d.",
		      temperature_C, INT16_MIN);

	zassert_equal(0, dummy_sensor_sample_fetch_fake.call_count,
		      "dummy_sensor_sample_fetch called %d times, expected %d",
		      dummy_sensor_sample_fetch_fake.call_count, 0);
	zassert_equal(0, dummy_sensor_channel_get_fake.call_count,
		      "dummy_sensor_channel_get called %d times, expected %d",
		      dummy_sensor_channel_get_fake.call_count, 0);
}

ZTEST(temperature, test_get_value_fetch_fail_1)
{
	dummy_sensor_sample_fetch_fake.return_val = 1;
	sid_error_t error = sid_pal_temperature_init();
	zassert_equal(SID_ERROR_NONE, error, "Initialization failed with error %d, expected %d.",
		      error, SID_ERROR_NONE);
	int16_t temperature_C = sid_pal_temperature_get();
	zassert_equal(INT16_MIN, temperature_C, "Unexpected temperature read %d, expected %d.",
		      temperature_C, INT16_MIN);

	zassert_equal(1, dummy_sensor_sample_fetch_fake.call_count,
		      "dummy_sensor_sample_fetch called %d times, expected %d",
		      dummy_sensor_sample_fetch_fake.call_count, 1);
	zassert_equal(0, dummy_sensor_channel_get_fake.call_count,
		      "dummy_sensor_channel_get called %d times, expected %d",
		      dummy_sensor_channel_get_fake.call_count, 0);
}

ZTEST(temperature, test_get_value_fetch_fail_negative_1)
{
	dummy_sensor_sample_fetch_fake.return_val = -1;
	sid_error_t error = sid_pal_temperature_init();
	zassert_equal(SID_ERROR_NONE, error, "Initialization failed with error %d, expected %d.",
		      error, SID_ERROR_NONE);
	int16_t temperature_C = sid_pal_temperature_get();
	zassert_equal(INT16_MIN, temperature_C, "Unexpected temperature read %d, expected %d.",
		      temperature_C, INT16_MIN);

	zassert_equal(1, dummy_sensor_sample_fetch_fake.call_count,
		      "dummy_sensor_sample_fetch called %d times, expected %d",
		      dummy_sensor_sample_fetch_fake.call_count, 1);
	zassert_equal(0, dummy_sensor_channel_get_fake.call_count,
		      "dummy_sensor_channel_get called %d times, expected %d",
		      dummy_sensor_channel_get_fake.call_count, 0);
}

ZTEST(temperature, test_get_value_get_ret_1)
{
	dummy_sensor_sample_fetch_fake.return_val = 0;

	dummy_sensor_channel_get_fake.custom_fake = custom_fake_dummy_sensor_channel_get;
	dummy_sensor_channel_get_fake.return_val = 1;
	custom_fake_dummy_sensor_channel_get_sensor_value = SENSOR_VALUE_FLOAT(15.123);

	sid_error_t error = sid_pal_temperature_init();
	zassert_equal(SID_ERROR_NONE, error, "Initialization failed with error %d, expected %d.",
		      error, SID_ERROR_NONE);
	int16_t temperature_C = sid_pal_temperature_get();
	zassert_equal(INT16_MIN, temperature_C, "Unexpected temperature read %d, expected %d.",
		      temperature_C, INT16_MIN);

	zassert_equal(1, dummy_sensor_sample_fetch_fake.call_count,
		      "dummy_sensor_sample_fetch called %d times, expected %d",
		      dummy_sensor_sample_fetch_fake.call_count, 1);
	zassert_equal(1, dummy_sensor_channel_get_fake.call_count,
		      "dummy_sensor_channel_get called %d times, expected %d",
		      dummy_sensor_channel_get_fake.call_count, 1);
}

ZTEST(temperature, test_get_value_get_ret_negative_1)
{
	dummy_sensor_sample_fetch_fake.return_val = 0;

	dummy_sensor_channel_get_fake.custom_fake = custom_fake_dummy_sensor_channel_get;
	dummy_sensor_channel_get_fake.return_val = -1;
	custom_fake_dummy_sensor_channel_get_sensor_value = SENSOR_VALUE_FLOAT(15.123);

	sid_error_t error = sid_pal_temperature_init();
	zassert_equal(SID_ERROR_NONE, error, "Initialization failed with error %d, expected %d.",
		      error, SID_ERROR_NONE);
	int16_t temperature_C = sid_pal_temperature_get();
	zassert_equal(INT16_MIN, temperature_C, "Unexpected temperature read %d, expected %d.",
		      temperature_C, INT16_MIN);

	zassert_equal(1, dummy_sensor_sample_fetch_fake.call_count,
		      "dummy_sensor_sample_fetch called %d times, expected %d",
		      dummy_sensor_sample_fetch_fake.call_count, 1);
	zassert_equal(1, dummy_sensor_channel_get_fake.call_count,
		      "dummy_sensor_channel_get called %d times, expected %d",
		      dummy_sensor_channel_get_fake.call_count, 1);
}
