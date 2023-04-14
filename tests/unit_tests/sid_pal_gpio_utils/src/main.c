/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_gpio_utils.h>
#include <zephyr/drivers/cmock_gpio.h>
#include <unity.h>

#define E_OK (0)

#define INVALID_GPIO    (99)

#define GPIO_NUMBER_1     (1)
#define GPIO_PIN_NUMBER_1 (1)

#define GPIO_NUMBER_0_MASK    (1 << 0)
#define GPIO_NUMBER_1_MASK    (1 << 1)
#define GPIO_NUMBER_31_MASK   (1 << 31)

#define GPIO_LV_HI      (1)
#define GPIO_LV_LO      (0)

#define TEST_GPIO_LIST					   \
	/*     GPIO_NUMBER, PIN_NUMBER, PORT */		   \
	X_GPIO(0, 0, DEVICE_DT_GET(DT_NODELABEL(gpio0)))   \
	X_GPIO(1, 1, DEVICE_DT_GET(DT_NODELABEL(gpio0)))   \
	X_GPIO(15, 15, DEVICE_DT_GET(DT_NODELABEL(gpio0))) \
	X_GPIO(31, 31, DEVICE_DT_GET(DT_NODELABEL(gpio0))) \
/* end of TEST_GPIO_LIST */

static uint32_t test_gpio_number[] = {
#define X_GPIO(gpio_number, pin_number, port) gpio_number,
	TEST_GPIO_LIST
#undef X_GPIO
};

static gpio_port_pin_t test_port_pin[] = {
#define X_GPIO(gpio_number, pin_number, port) { port, pin_number },
	TEST_GPIO_LIST
#undef X_GPIO
};

static const struct device fake_gpio_port;

void setUp(void)
{
}

void tearDown(void)
{
}

void test_sid_gpio_utils_port_pin_get_fail(void)
{
	gpio_port_pin_t port_pin;

	TEST_ASSERT_EQUAL(-ENOENT, sid_gpio_utils_port_pin_get(GPIO_NUMBER_1, NULL));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_port_pin_get(INVALID_GPIO, &port_pin));
}

void test_sid_gpio_utils_port_pin_get_pass(void)
{
	gpio_port_pin_t port_pin;

	for (int it = 0; it < ARRAY_SIZE(test_gpio_number); it++) {
		TEST_ASSERT_EQUAL(E_OK, sid_gpio_utils_port_pin_get(test_gpio_number[it], &port_pin));

		TEST_ASSERT_EQUAL(test_port_pin[it].pin, port_pin.pin);
		TEST_ASSERT_EQUAL(test_port_pin[it].port, port_pin.port);
	}
}

void test_sid_gpio_utils_gpio_number_get_fail(void)
{
	TEST_ASSERT_EQUAL(GPIO_UNUSED_PIN, sid_gpio_utils_gpio_number_get(NULL, GPIO_NUMBER_1_MASK));

	TEST_ASSERT_EQUAL(GPIO_UNUSED_PIN, sid_gpio_utils_gpio_number_get(&fake_gpio_port, GPIO_NUMBER_1_MASK));
}

void test_sid_gpio_utils_gpio_number_get_pass(void)
{
	for (int it = 0; it < ARRAY_SIZE(test_gpio_number); it++) {
		TEST_ASSERT_EQUAL(test_gpio_number[it],
				  sid_gpio_utils_gpio_number_get(test_port_pin[it].port, BIT(test_port_pin[it].pin)));
	}
}

void test_sid_gpio_utils_gpio_read_fail(void)
{
	uint8_t gpio_state = 0;

	TEST_ASSERT_EQUAL(-ENOENT, sid_gpio_utils_gpio_read(GPIO_NUMBER_1, NULL));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_gpio_read(INVALID_GPIO, &gpio_state));
	__cmock_gpio_pin_get_raw_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(-EIO, sid_gpio_utils_gpio_read(GPIO_NUMBER_1, &gpio_state));
}

void test_sid_gpio_utils_gpio_read_pass(void)
{
	uint8_t gpio_state = 0;

	__cmock_gpio_pin_get_raw_IgnoreAndReturn(GPIO_LV_HI);
	TEST_ASSERT_GREATER_OR_EQUAL(0, sid_gpio_utils_gpio_read(GPIO_NUMBER_1, &gpio_state));
	TEST_ASSERT_EQUAL(GPIO_LV_HI, gpio_state);
	__cmock_gpio_pin_get_raw_IgnoreAndReturn(GPIO_LV_LO);
	TEST_ASSERT_GREATER_OR_EQUAL(0, sid_gpio_utils_gpio_read(GPIO_NUMBER_1, &gpio_state));
	TEST_ASSERT_EQUAL(GPIO_LV_LO, gpio_state);
}

void test_sid_gpio_utils_gpio_set_fail(void)
{
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_gpio_set(INVALID_GPIO, GPIO_LV_HI));

	__cmock_gpio_pin_set_raw_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(-EIO, sid_gpio_utils_gpio_set(GPIO_NUMBER_1, GPIO_LV_HI));
	TEST_ASSERT_EQUAL(-EIO, sid_gpio_utils_gpio_set(GPIO_NUMBER_1, GPIO_LV_LO));
}

void test_sid_gpio_utils_gpio_set_pass(void)
{
	__cmock_gpio_pin_set_raw_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_utils_gpio_set(GPIO_NUMBER_1, GPIO_LV_HI));
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_utils_gpio_set(GPIO_NUMBER_1, GPIO_LV_LO));
}

void test_sid_gpio_utils_gpio_toggle_fail(void)
{
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_gpio_toggle(INVALID_GPIO));

	__cmock_gpio_pin_toggle_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(-EIO, sid_gpio_utils_gpio_toggle(GPIO_NUMBER_1));
}

void test_sid_gpio_utils_gpio_toggle_pass(void)
{
	__cmock_gpio_pin_toggle_IgnoreAndReturn(-E_OK);
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_utils_gpio_toggle(GPIO_NUMBER_1));
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
