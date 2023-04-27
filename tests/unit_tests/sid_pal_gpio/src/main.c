/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sid_pal_gpio_ifc.h>
#include <zephyr/drivers/cmock_sid_gpio_utils.h>
#include <zephyr/drivers/cmock_gpio.h>
#include <cmock_sid_gpio_irq.h>
#include <cmock_sid_gpio_irq_handler.h>
#include <unity.h>

#define INVALID_GPIO    (99)
#define GPIO_NUMBER     (1)

#define GPIO_LV_HI      (1)
#define GPIO_LV_LO      (0)

#define INVALID_DIRECTION_VALUE         (20)
#define INVALID_INPUT_MODE_VALUE        (20)
#define INVALID_PULL_MODE_VALUE         (20)
#define INVALID_IRQ_TRIGGER_VALUE       (20)

#define E_OK    (0)

void setUp(void)
{
}

void tearDown(void)
{
}

static void gpio_irq_handler(uint32_t gpio_number, void *callback_arg)
{
}

void test_sid_pal_gpio_read_fail(void)
{
	uint8_t test_gpio_state;

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, sid_pal_gpio_read(INVALID_GPIO, NULL));
	__cmock_sid_gpio_utils_gpio_read_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_read(GPIO_NUMBER, &test_gpio_state));
}

void test_sid_pal_gpio_read_pass(void)
{
	uint8_t test_gpio_state;

	__cmock_sid_gpio_utils_gpio_read_IgnoreAndReturn(GPIO_LV_HI);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_read(GPIO_NUMBER, &test_gpio_state));

	__cmock_sid_gpio_utils_gpio_read_IgnoreAndReturn(GPIO_LV_LO);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_read(GPIO_NUMBER, &test_gpio_state));
}

void test_sid_pal_gpio_write_fail(void)
{
	__cmock_sid_gpio_utils_gpio_set_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_write(INVALID_GPIO, GPIO_LV_HI));

	__cmock_sid_gpio_utils_gpio_set_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(SID_ERROR_IO_ERROR, sid_pal_gpio_write(GPIO_NUMBER, GPIO_LV_HI));
}

void test_sid_pal_gpio_write_pass(void)
{
	__cmock_sid_gpio_utils_gpio_set_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_write(GPIO_NUMBER, GPIO_LV_HI));

	__cmock_sid_gpio_utils_gpio_set_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_write(GPIO_NUMBER, GPIO_LV_LO));
}

void test_sid_pal_gpio_toggle_fail(void)
{
	__cmock_sid_gpio_utils_gpio_toggle_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_toggle(INVALID_GPIO));

	__cmock_sid_gpio_utils_gpio_toggle_IgnoreAndReturn(-EBUSY);
	TEST_ASSERT_EQUAL(SID_ERROR_BUSY, sid_pal_gpio_toggle(GPIO_NUMBER));
}

void test_sid_pal_gpio_toggle_pass(void)
{
	__cmock_sid_gpio_utils_gpio_toggle_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_toggle(GPIO_NUMBER));
}

void test_sid_pal_gpio_set_direction_fail(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_set_direction(GPIO_NUMBER, INVALID_DIRECTION_VALUE));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_set_direction(INVALID_GPIO, SID_PAL_GPIO_DIRECTION_INPUT));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(-EINVAL);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS,
			  sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_INPUT));
}

void test_sid_pal_gpio_set_direction_pass(void)
{
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_INPUT));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_OUTPUT));
}

void test_sid_pal_gpio_input_mode_fail(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_input_mode(GPIO_NUMBER, INVALID_INPUT_MODE_VALUE));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_input_mode(INVALID_GPIO, SID_PAL_GPIO_INPUT_CONNECT));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_OUTPUT));
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_input_mode(GPIO_NUMBER, SID_PAL_GPIO_INPUT_CONNECT));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_INPUT));
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(SID_ERROR_IO_ERROR, sid_pal_gpio_input_mode(GPIO_NUMBER, SID_PAL_GPIO_INPUT_CONNECT));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(SID_ERROR_IO_ERROR, sid_pal_gpio_input_mode(GPIO_NUMBER, SID_PAL_GPIO_INPUT_DISCONNECT));
}

void test_sid_pal_gpio_input_mode_pass(void)
{
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_INPUT));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_input_mode(GPIO_NUMBER, SID_PAL_GPIO_INPUT_CONNECT));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_input_mode(GPIO_NUMBER, SID_PAL_GPIO_INPUT_DISCONNECT));
}

void test_sid_pal_gpio_output_mode_fail(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_output_mode(GPIO_NUMBER, INVALID_INPUT_MODE_VALUE));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_output_mode(INVALID_GPIO, SID_PAL_GPIO_OUTPUT_PUSH_PULL));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_INPUT));
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_output_mode(GPIO_NUMBER, SID_PAL_GPIO_OUTPUT_PUSH_PULL));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_OUTPUT));
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(SID_ERROR_IO_ERROR, sid_pal_gpio_output_mode(GPIO_NUMBER, SID_PAL_GPIO_OUTPUT_OPEN_DRAIN));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(SID_ERROR_IO_ERROR, sid_pal_gpio_output_mode(GPIO_NUMBER, SID_PAL_GPIO_OUTPUT_PUSH_PULL));
}

void test_sid_pal_gpio_output_mode_pass(void)
{
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_NUMBER, SID_PAL_GPIO_DIRECTION_OUTPUT));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_output_mode(GPIO_NUMBER, SID_PAL_GPIO_OUTPUT_OPEN_DRAIN));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_output_mode(GPIO_NUMBER, SID_PAL_GPIO_OUTPUT_PUSH_PULL));
}

void test_sid_pal_gpio_pull_mode_fail(void)
{
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_pull_mode(INVALID_GPIO, SID_PAL_GPIO_PULL_NONE));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_pull_mode(GPIO_NUMBER, INVALID_PULL_MODE_VALUE));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(-EIO);
	TEST_ASSERT_EQUAL(SID_ERROR_IO_ERROR, sid_pal_gpio_pull_mode(GPIO_NUMBER, SID_PAL_GPIO_PULL_NONE));
}

void test_sid_pal_gpio_pull_mode_pass(void)
{
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_configure_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_pull_mode(GPIO_NUMBER, SID_PAL_GPIO_PULL_NONE));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_pull_mode(GPIO_NUMBER, SID_PAL_GPIO_PULL_UP));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_pull_mode(GPIO_NUMBER, SID_PAL_GPIO_PULL_DOWN));
}

void test_sid_pal_gpio_set_irq_fail(void)
{
	const uint8_t handle_argument = 5;

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_gpio_set_irq(GPIO_NUMBER, INVALID_IRQ_TRIGGER_VALUE,
								       gpio_irq_handler, (void *)&handle_argument));
	__cmock_sid_gpio_irq_handler_set_Ignore();
	__cmock_sid_gpio_irq_configure_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_set_irq(INVALID_GPIO, SID_PAL_GPIO_IRQ_TRIGGER_RISING,
								    gpio_irq_handler, (void *)&handle_argument));
	__cmock_sid_gpio_irq_handler_set_StopIgnore();
}

void test_sid_pal_gpio_set_irq_pass(void)
{
	sid_pal_gpio_irq_trigger_t irq_trigger_list[] = {
		SID_PAL_GPIO_IRQ_TRIGGER_NONE,
		SID_PAL_GPIO_IRQ_TRIGGER_RISING,
		SID_PAL_GPIO_IRQ_TRIGGER_FALLING,
		SID_PAL_GPIO_IRQ_TRIGGER_EDGE,
		SID_PAL_GPIO_IRQ_TRIGGER_LOW,
		SID_PAL_GPIO_IRQ_TRIGGER_HIGH
	};
	const uint8_t handle_argument = 5;

	for (sid_pal_gpio_irq_trigger_t trigger = SID_PAL_GPIO_IRQ_TRIGGER_NONE; trigger < ARRAY_SIZE(irq_trigger_list);
	     trigger++) {
		__cmock_sid_gpio_irq_configure_IgnoreAndReturn(E_OK);
		if (SID_PAL_GPIO_IRQ_TRIGGER_NONE == trigger) {
			__cmock_sid_gpio_irq_handler_set_Expect(GPIO_NUMBER, NULL, NULL);
		} else {
			__cmock_sid_gpio_irq_handler_set_Expect(GPIO_NUMBER, gpio_irq_handler,
								(void *)&handle_argument);
		}
		TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_set_irq(GPIO_NUMBER, trigger,
								       gpio_irq_handler, (void *)&handle_argument));
	}
}

void test_sid_pal_gpio_irq_enable_disable_fail(void)
{
	__cmock_sid_gpio_irq_set_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_irq_enable(INVALID_GPIO));
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, sid_pal_gpio_irq_disable(INVALID_GPIO));
}

void test_sid_pal_gpio_irq_enable_disable_pass(void)
{
	__cmock_sid_gpio_irq_set_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_irq_enable(GPIO_NUMBER));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_gpio_irq_disable(GPIO_NUMBER));
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

int main(void)
{
	(void)unity_main();

	return 0;
}
