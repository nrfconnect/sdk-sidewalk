/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include "sid_error.h"
#include "zephyr/drivers/gpio.h"
#include <sid_pal_gpio_ifc.h>
#include <stdint.h>
#include <zephyr/drivers/cmock_gpio.h>
#include <unity.h>
#include <zephyr/kernel.h>

#include <sid_gpio_utils.h>

#define INVALID_GPIO (99)
#define GPIO_NUMBER (0)

#define GPIO_LV_HI (1)
#define GPIO_LV_LO (0)

#define INVALID_DIRECTION_VALUE (20)
#define INVALID_INPUT_MODE_VALUE (20)
#define INVALID_PULL_MODE_VALUE (20)
#define INVALID_IRQ_TRIGGER_VALUE (20)

#define E_OK (0)

void setUp(void)
{
	cmock_gpio_Init();
}

void tearDown(void)
{
	sid_gpio_utils_clear_register();
	cmock_gpio_Verify();
	cmock_gpio_Destroy();
}

void test_register_too_much(void)
{
	for (int i = 0; i < CONFIG_SIDEWALK_GPIO_MAX; i++) {
		int ret = sid_gpio_utils_register_gpio(
			(struct gpio_dt_spec){ .port = (struct device *)0x123 + i, .pin = i });
		TEST_ASSERT_GREATER_OR_EQUAL(0, ret);
	}
	int ret = sid_gpio_utils_register_gpio((struct gpio_dt_spec){
		.port = (struct device *)0x123 + CONFIG_SIDEWALK_GPIO_MAX + 1,
		.pin = CONFIG_SIDEWALK_GPIO_MAX + 1 });
	TEST_ASSERT_EQUAL(-ENOSPC, ret);
}

void test_read_without_register(void)
{
	uint8_t test_gpio_state = 123;

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_read(0, &test_gpio_state));
}

void test_read_not_registered(void)
{
	uint8_t test_gpio_state = 123;
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_read(gpio + 1, &test_gpio_state));
	TEST_ASSERT_EQUAL(123, test_gpio_state);
}

void test_read_without_direction(void)
{
	uint8_t test_gpio_state = 123;
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_read(gpio, &test_gpio_state));
}

void test_read_gpio_OUTPUT(void)
{
	uint8_t test_gpio_state = 123;
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_read(gpio, &test_gpio_state));
	TEST_ASSERT_EQUAL(123, test_gpio_state);
}

void test_read_NULL(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_gpio_read(gpio, NULL));
}

void test_read_gpio(void)
{
	uint8_t test_gpio_state = 123;
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_get_raw_ExpectAndReturn(&dev, 2, 0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_read(gpio, &test_gpio_state));
	TEST_ASSERT_EQUAL(0, test_gpio_state);
}

void test_set_direction_not_registered(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	sid_error_t err = sid_pal_gpio_set_direction(gpio + 1, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_set_direction_not_registered_internal(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	int err = sid_gpio_utils_gpio_set_flags(gpio + 1, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(-EINVAL, err);
}

void test_set_direction_input(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_direction_output(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_OUTPUT, 0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_direction_invalid(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT + 1);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_write_without_register(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_write(0, 1));
}

void test_write_not_registered(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	const uint8_t gpio_value = 1;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_write(gpio + 1, gpio_value));
}

void test_write_without_direction(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_write(gpio, 0));
}

void test_write_gpio_INPUT(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	const uint8_t gpio_value = 1;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_write(gpio, gpio_value));
}

void test_write_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	const uint8_t gpio_value = 1;
	__cmock_gpio_pin_set_raw_ExpectAndReturn(&dev, 2, gpio_value, 0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_write(gpio, gpio_value));
}

void test_toggle_without_register(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_toggle(0));
}

void test_toggle_not_registered(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_toggle(gpio + 1));
}

void test_toggle_without_direction(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_toggle(gpio));
}

void test_toggle_gpio_INPUT(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_toggle(gpio));
}

void test_toggle_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_toggle_ExpectAndReturn(&dev, 2, 0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_toggle(gpio));
}

void test_toggle_set_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_toggle_ExpectAndReturn(&dev, 2, 0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_toggle(gpio));
	const uint8_t gpio_value = 1;
	__cmock_gpio_pin_set_raw_ExpectAndReturn(&dev, 2, gpio_value, 0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_write(gpio, gpio_value));
}

void test_set_toggle_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	const uint8_t gpio_value = 1;
	__cmock_gpio_pin_set_raw_ExpectAndReturn(&dev, 2, gpio_value, 0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_write(gpio, gpio_value));
	__cmock_gpio_pin_toggle_ExpectAndReturn(&dev, 2, 0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_toggle(gpio));
}

void test_sid_gpio_utils_gpio_get_flags_unregistered()
{
	TEST_ASSERT_EQUAL(-EINVAL, sid_gpio_utils_gpio_get_flags(0, NULL));
}

void test_sid_gpio_utils_gpio_get_flags_null()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	TEST_ASSERT_EQUAL(-ENOENT, sid_gpio_utils_gpio_get_flags(gpio, NULL));
}

void test_input_mode_connect_to_output_gpio()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_CONNECT);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_input_mode_connect_invalid_gpio()
{
	sid_error_t err = sid_pal_gpio_input_mode(0, SID_PAL_GPIO_INPUT_CONNECT);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_input_mode_connect()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_CONNECT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_input_mode_disconnect()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_DISCONNECT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_input_mode_invalid()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_DISCONNECT + 1);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, err);
}

void test_output_mode_connect_to_input_gpio()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	err = sid_pal_gpio_output_mode(gpio, SID_PAL_GPIO_OUTPUT_PUSH_PULL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_output_mode_connect_invalid_gpio()
{
	sid_error_t err = sid_pal_gpio_output_mode(0, SID_PAL_GPIO_OUTPUT_PUSH_PULL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_output_mode_push_pull()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	err = sid_pal_gpio_output_mode(gpio, SID_PAL_GPIO_OUTPUT_PUSH_PULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_output_mode_open_drain()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	err = sid_pal_gpio_output_mode(gpio, SID_PAL_GPIO_OUTPUT_OPEN_DRAIN);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_output_mode_invalid()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_IgnoreAndReturn(0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	err = sid_pal_gpio_output_mode(gpio, SID_PAL_GPIO_OUTPUT_OPEN_DRAIN + 1);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, err);
}

void test_pull_mode_unregistered()
{
	sid_error_t err = sid_pal_gpio_pull_mode(0, SID_PAL_GPIO_PULL_NONE);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_pull_mode_unregistered2()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	sid_error_t err = sid_pal_gpio_pull_mode(gpio + 1, SID_PAL_GPIO_PULL_NONE);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_pull_mode_NONE()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, 0, 0);
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_NONE);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_pull_mode_UP()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_PULL_UP, 0);
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_UP);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_pull_mode_DOWN()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_PULL_DOWN, 0);
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_DOWN);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_pull_mode_DOWN_UP()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_PULL_DOWN, 0);
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_DOWN);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_PULL_UP, 0);
	err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_UP);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_pull_mode_UP_DOWN()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_PULL_UP, 0);
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_UP);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_PULL_DOWN, 0);
	err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_DOWN);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_pull_mode_invalid()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_DOWN + 1);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_set_irq_unregistered()
{
	sid_error_t err = sid_pal_gpio_set_irq(0, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_set_irq_unregistered2()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	sid_error_t err = sid_pal_gpio_set_irq(gpio + 1, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_set_irq_none()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_irq_rising()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_EDGE_RISING, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_RISING, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_irq_falling()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_EDGE_FALLING, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_FALLING, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_irq_edge()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_EDGE_BOTH, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_EDGE, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_irq_low()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_LOW, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_LOW, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_irq_high()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_irq_invalid()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH + 1, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_set_irq_configure_and_disable()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_remove_callback_IgnoreAndReturn(0);
	err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_irq_configure_cb_error()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, -22);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_set_irq_configure_and_disable_cb_error()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_remove_callback_IgnoreAndReturn(-22);
	err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

static gpio_callback_handler_t gpio_init_callback_STUB_handler;
static struct gpio_callback *gpio_init_callback_STUB_callback;
static gpio_port_pins_t gpio_init_callback_STUB_pin_mask;
void gpio_init_callback_STUB(struct gpio_callback *callback, gpio_callback_handler_t handler,
			     gpio_port_pins_t pin_mask, int cmock_num_calls)
{
	gpio_init_callback_STUB_callback = callback;
	gpio_init_callback_STUB_handler = handler;
	gpio_init_callback_STUB_pin_mask = pin_mask;
}

struct gpio_irq_arg {
	uint32_t gpio_number;
	int call_count;
};
void sid_pal_gpio_irq_handler_test(uint32_t gpio_number, void *callback_arg)
{
	struct gpio_irq_arg *ctx = (struct gpio_irq_arg *)callback_arg;
	ctx->gpio_number = gpio_number;
	ctx->call_count++;
}

void test_irq_handler()
{
	gpio_init_callback_STUB_handler = NULL;
	gpio_init_callback_STUB_callback = NULL;
	struct device dev;
	struct gpio_irq_arg arg = { 0, 0 };
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(0);
	__cmock_gpio_init_callback_AddCallback(gpio_init_callback_STUB);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH,
					       sid_pal_gpio_irq_handler_test, &arg);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);

	TEST_ASSERT_NOT_NULL(gpio_init_callback_STUB_handler);
	TEST_ASSERT_NOT_NULL(gpio_init_callback_STUB_callback);
	gpio_init_callback_STUB_handler(&dev, gpio_init_callback_STUB_callback,
					gpio_init_callback_STUB_pin_mask);
	k_yield();
	TEST_ASSERT_EQUAL(1, arg.call_count);
	TEST_ASSERT_EQUAL(gpio, arg.gpio_number);
}

void test_irq_handler_invalid_dev_in_callback()
{
	gpio_init_callback_STUB_handler = NULL;
	gpio_init_callback_STUB_callback = NULL;
	struct device dev;
	struct gpio_irq_arg arg = { 123, 0 };
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(0);
	__cmock_gpio_init_callback_AddCallback(gpio_init_callback_STUB);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH,
					       sid_pal_gpio_irq_handler_test, &arg);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);

	TEST_ASSERT_NOT_NULL(gpio_init_callback_STUB_handler);
	TEST_ASSERT_NOT_NULL(gpio_init_callback_STUB_callback);
	gpio_init_callback_STUB_handler(NULL, gpio_init_callback_STUB_callback,
					gpio_init_callback_STUB_pin_mask);
	k_yield();
	TEST_ASSERT_EQUAL(0, arg.call_count);
	TEST_ASSERT_EQUAL(123, arg.gpio_number);
}

void test_sid_pal_gpio_irq_enable()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, 0, 0);
	sid_error_t err = sid_pal_gpio_irq_enable(gpio);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_sid_pal_gpio_irq_enable_set_irq()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	err = sid_pal_gpio_irq_enable(gpio);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_sid_pal_gpio_irq_disable()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, 0);
	sid_error_t err = sid_pal_gpio_irq_disable(gpio);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_sid_pal_gpio_irq_disable_unregister()
{
	sid_error_t err = sid_pal_gpio_irq_disable(0);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, err);
}

void test_error_mapping()
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, -EINVAL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_irq_disable(gpio));
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, -ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_irq_disable(gpio));
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, -EIO);
	TEST_ASSERT_EQUAL(SID_ERROR_IO_ERROR, sid_pal_gpio_irq_disable(gpio));
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, -EBUSY);
	TEST_ASSERT_EQUAL(SID_ERROR_BUSY, sid_pal_gpio_irq_disable(gpio));
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, -0xffff);
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, sid_pal_gpio_irq_disable(gpio));
}

void test_unused_pins()
{
	uint8_t val;
	gpio_flags_t flag;
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_gpio_read(GPIO_UNUSED_PIN, &val));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_gpio_set(GPIO_UNUSED_PIN, 123));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_gpio_toggle(GPIO_UNUSED_PIN));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_gpio_set_flags(GPIO_UNUSED_PIN, 123));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_disconnect(GPIO_UNUSED_PIN));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_gpio_get_flags(GPIO_UNUSED_PIN, &flag));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_irq_handler_set(GPIO_UNUSED_PIN, NULL, NULL));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_irq_configure(GPIO_UNUSED_PIN, 123));
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_utils_irq_set(GPIO_UNUSED_PIN, true));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_UNUSED_PIN,
								     SID_PAL_GPIO_DIRECTION_INPUT));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_read(GPIO_UNUSED_PIN, &val));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_write(GPIO_UNUSED_PIN, 123));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_toggle(GPIO_UNUSED_PIN));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_gpio_set_irq(GPIO_UNUSED_PIN, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL,
					       NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_irq_enable(GPIO_UNUSED_PIN));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_irq_disable(GPIO_UNUSED_PIN));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_gpio_input_mode(GPIO_UNUSED_PIN, SID_PAL_GPIO_INPUT_CONNECT));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_gpio_output_mode(GPIO_UNUSED_PIN, SID_PAL_GPIO_OUTPUT_PUSH_PULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_pal_gpio_pull_mode(GPIO_UNUSED_PIN, SID_PAL_GPIO_PULL_NONE));
}

struct read_intput_on_irq_handler_args {
	int call_count;
	uint8_t readed_gpio_value;
	sid_error_t read_return_code;
};

static void read_intput_on_irq_handler(uint32_t gpio_number, void *callback_arg)
{
	struct read_intput_on_irq_handler_args *arg =
		(struct read_intput_on_irq_handler_args *)callback_arg;
	arg->call_count++;
	arg->read_return_code = sid_pal_gpio_read(gpio_number, &arg->readed_gpio_value);
}

void test_read_intput_on_irq()
{
	gpio_init_callback_STUB_handler = NULL;
	gpio_init_callback_STUB_callback = NULL;
	struct device dev;
	struct read_intput_on_irq_handler_args handler_arg = { 0, 0, 0 };

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	__cmock_gpio_init_callback_AddCallback(gpio_init_callback_STUB);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, read_intput_on_irq_handler,
				   &handler_arg);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);

	__cmock_gpio_pin_get_raw_ExpectAndReturn(&dev, 2, 1);

	TEST_ASSERT_NOT_NULL(gpio_init_callback_STUB_handler);
	TEST_ASSERT_NOT_NULL(gpio_init_callback_STUB_callback);
	gpio_init_callback_STUB_handler(&dev, gpio_init_callback_STUB_callback,
					gpio_init_callback_STUB_pin_mask);
	k_yield();
	TEST_ASSERT_EQUAL(1, handler_arg.call_count);
	TEST_ASSERT_EQUAL(1, handler_arg.readed_gpio_value);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, handler_arg.read_return_code);
}

void test_read_intput_on_irq_gpio_not_input()
{
	gpio_init_callback_STUB_handler = NULL;
	gpio_init_callback_STUB_callback = NULL;
	struct device dev;
	struct read_intput_on_irq_handler_args handler_arg = { 0, 0, 0 };

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	__cmock_gpio_init_callback_AddCallback(gpio_init_callback_STUB);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH,
					       read_intput_on_irq_handler, &handler_arg);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);

	__cmock_gpio_pin_get_raw_ExpectAndReturn(&dev, 2, 1);

	TEST_ASSERT_NOT_NULL(gpio_init_callback_STUB_handler);
	TEST_ASSERT_NOT_NULL(gpio_init_callback_STUB_callback);
	gpio_init_callback_STUB_handler(&dev, gpio_init_callback_STUB_callback,
					gpio_init_callback_STUB_pin_mask);
	k_yield();
	TEST_ASSERT_EQUAL(1, handler_arg.call_count);
	TEST_ASSERT_EQUAL(1, handler_arg.readed_gpio_value);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, handler_arg.read_return_code);
}

void test_set_irq_flags_disable_enable()
{
	struct device dev;

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_ExpectAndReturn(&dev, NULL, 0);
	__cmock_gpio_add_callback_IgnoreArg_callback();
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_DISABLE, 0);
	err = sid_pal_gpio_irq_disable(gpio);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(&dev, 2, GPIO_INT_LEVEL_HIGH, 0);
	err = sid_pal_gpio_irq_enable(gpio);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_set_flags_disable_enable()
{
	struct device dev;

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT, 0);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT | GPIO_PULL_UP, 0);
	err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_UP);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_DISCONNECTED, 0);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_DISCONNECT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
	__cmock_gpio_pin_configure_ExpectAndReturn(&dev, 2, GPIO_INPUT | GPIO_PULL_UP, 0);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_CONNECT);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, err);
}

void test_register_twice_gpio()
{
	struct device dev;

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio);
	uint32_t gpio2 =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	TEST_ASSERT_GREATER_OR_EQUAL(0, gpio2);
	TEST_ASSERT_EQUAL(gpio, gpio2);
}

void test_register_invalid_gpio()
{
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = NULL, .pin = 0 });
	TEST_ASSERT_EQUAL(GPIO_UNUSED_PIN, gpio);
}
/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

int main(void)
{
	return unity_main();
}
