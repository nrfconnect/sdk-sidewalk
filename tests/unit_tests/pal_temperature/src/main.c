/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_pal_temperature_ifc.h>
#include <syscalls/device.h>
#include <mock_device.h>
#include <drivers/mock_sensor.h>
#include <unity.h>

char CONFIG_SIDEWALK_TEMPERATURE_SENSOR_NAME_VALUE[50] = "TEST_PROBE";

void setUp(void)
{
	mock_sensor_Init();
}

void tearDown(void)
{
	mock_sensor_Verify();
}

void test_sid_pal_temperature_init()
{
	struct device device;

	sprintf(CONFIG_SIDEWALK_TEMPERATURE_SENSOR_NAME_VALUE, "TEST_1_PROBE_DEVICE");
	__wrap_device_get_binding_ExpectAndReturn(CONFIG_SIDEWALK_TEMPERATURE_SENSOR_NAME_VALUE,
						  &device);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_temperature_init());
}

void test_sid_pal_temperature_init_fail()
{
	sprintf(CONFIG_SIDEWALK_TEMPERATURE_SENSOR_NAME_VALUE, "TEST_2_PROBE_DEVICE");
	__wrap_device_get_binding_ExpectAndReturn(CONFIG_SIDEWALK_TEMPERATURE_SENSOR_NAME_VALUE,
						  NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_temperature_init());
}

void test_sid_pal_temperature_get()
{
	struct sensor_value mock_temp = { 16, 0 };

	__wrap_sensor_sample_fetch_IgnoreAndReturn(0);

	__wrap_sensor_channel_get_ExpectAndReturn(NULL, SENSOR_CHAN_DIE_TEMP, NULL, 0);
	__wrap_sensor_channel_get_IgnoreArg_dev();
	__wrap_sensor_channel_get_IgnoreArg_val();
	__wrap_sensor_channel_get_ReturnThruPtr_val(&mock_temp);
	test_sid_pal_temperature_init();

	int16_t pal_temp = sid_pal_temperature_get();
	TEST_ASSERT_EQUAL(mock_temp.val1, pal_temp);
}

void test_sid_pal_temperature_get_failed_fetch_errno_min()
{
	__wrap_sensor_sample_fetch_IgnoreAndReturn(-EPERM);
	test_sid_pal_temperature_init();

	int16_t pal_temp = sid_pal_temperature_get();
	TEST_ASSERT_EQUAL(INT16_MIN, pal_temp);
}

void test_sid_pal_temperature_get_failed_fetch_errno_max()
{
	__wrap_sensor_sample_fetch_IgnoreAndReturn(-INT32_MAX);
	test_sid_pal_temperature_init();

	int16_t pal_temp = sid_pal_temperature_get();
	TEST_ASSERT_EQUAL(INT16_MIN, pal_temp);
}

void test_sid_pal_temperature_get_failed_get_errno_min()
{
	__wrap_sensor_sample_fetch_IgnoreAndReturn(0);

	__wrap_sensor_channel_get_ExpectAndReturn(NULL, SENSOR_CHAN_DIE_TEMP, NULL, -EPERM);
	__wrap_sensor_channel_get_IgnoreArg_dev();
	__wrap_sensor_channel_get_IgnoreArg_val();

	test_sid_pal_temperature_init();
	int16_t pal_temp = sid_pal_temperature_get();
	TEST_ASSERT_EQUAL(INT16_MIN, pal_temp);
}

void test_sid_pal_temperature_get_failed_get_errno_max()
{
	__wrap_sensor_sample_fetch_IgnoreAndReturn(0);

	__wrap_sensor_channel_get_ExpectAndReturn(NULL, SENSOR_CHAN_DIE_TEMP, NULL, -INT32_MAX);
	__wrap_sensor_channel_get_IgnoreArg_dev();
	__wrap_sensor_channel_get_IgnoreArg_val();

	test_sid_pal_temperature_init();
	int16_t pal_temp = sid_pal_temperature_get();
	TEST_ASSERT_EQUAL(INT16_MIN, pal_temp);
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
