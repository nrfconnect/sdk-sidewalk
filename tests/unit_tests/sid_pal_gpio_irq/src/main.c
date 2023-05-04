/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_gpio_irq.h>
#include <zephyr/drivers/cmock_gpio.h>
#include <zephyr/drivers/cmock_sid_gpio_utils.h>
#include <unity.h>

#define E_OK    (0)

#define INVALID_GPIO    (99)
#define GPIO_NUMBER_1   (1)
#define GPIO_NUMBER_2   (2)
#define GPIO_NUMBER_3   (3)
#define GPIO_NUMBER_4   (4)
#define GPIO_NUMBER_9   (9)

#define TEST_PORT (DEVICE_DT_GET(DT_NODELABEL(gpio0)))

static gpio_pin_t test_pin = GPIO_NUMBER_1;

void setUp(void)
{
	cmock_gpio_Init();
	cmock_sid_gpio_utils_Init();
}

void tearDown(void)
{
	cmock_gpio_Destroy();
	cmock_sid_gpio_utils_Destroy();
}

static int port_pin_get_cb(uint32_t gpio_number, gpio_port_pin_t *port_pin, int cmock_num_calls)
{
	port_pin->port = TEST_PORT;
	port_pin->pin = test_pin;

	return E_OK;
}

static void sid_gpio_utils_port_pin_get_test_setup(gpio_port_pin_t *port_pin)
{
	__cmock_sid_gpio_utils_port_pin_get_ExpectAndReturn(port_pin->pin, 0, E_OK);
	__cmock_sid_gpio_utils_port_pin_get_IgnoreArg_gpio_number();
	__cmock_sid_gpio_utils_port_pin_get_IgnoreArg_port_pin();
	__cmock_sid_gpio_utils_port_pin_get_ReturnThruPtr_port_pin(port_pin);
}

void test_sid_gpio_irq_configure_fail(void)
{
	gpio_flags_t test_flag = GPIO_INT_DISABLE;
	gpio_port_pin_t port_pin = { .pin = GPIO_NUMBER_1 };

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_irq_configure(INVALID_GPIO, 0));

	sid_gpio_utils_port_pin_get_test_setup(&port_pin);
	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_irq_configure(port_pin.pin, test_flag));
}

void test_sid_gpio_irq_configure_add_cb_fail(void)
{
	gpio_port_pin_t port_pin = { .pin = GPIO_NUMBER_1 };
	gpio_flags_t test_flag = GPIO_INT_ENABLE;

	sid_gpio_utils_port_pin_get_test_setup(&port_pin);
	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(-ENOTSUP);

	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_irq_configure(port_pin.pin, test_flag));

	port_pin.pin = GPIO_NUMBER_9;
	sid_gpio_utils_port_pin_get_test_setup(&port_pin);
	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_irq_configure(port_pin.pin, test_flag));
}

void test_sid_gpio_irq_configure_remove_cb_fail(void)
{
	gpio_flags_t test_flag = GPIO_INT_ENABLE;

	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(E_OK);
	__cmock_sid_gpio_utils_port_pin_get_Stub(port_pin_get_cb);

	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(GPIO_NUMBER_1, test_flag));

	test_flag = GPIO_INT_DISABLE;
	__cmock_gpio_remove_callback_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_irq_configure(GPIO_NUMBER_1, test_flag));
}

void test_sid_gpio_irq_configure_pass(void)
{
	gpio_flags_t test_flag = GPIO_INT_ENABLE;

	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(E_OK);
	__cmock_sid_gpio_utils_port_pin_get_Stub(port_pin_get_cb);

	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_IgnoreAndReturn(E_OK);
	test_pin = GPIO_NUMBER_1;
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));

	test_pin = GPIO_NUMBER_2;
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));

	test_pin = GPIO_NUMBER_4;
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));
}

void test_sid_gpio_irq_configure_add_remove_pass(void)
{
	gpio_flags_t test_flag = GPIO_INT_ENABLE;

	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(E_OK);
	__cmock_sid_gpio_utils_port_pin_get_Stub(port_pin_get_cb);

	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_IgnoreAndReturn(E_OK);
	test_pin = GPIO_NUMBER_1;
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));

	test_pin = GPIO_NUMBER_2;
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));

	test_pin = GPIO_NUMBER_4;
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));

	test_flag = GPIO_INT_DISABLE;
	test_pin = GPIO_NUMBER_1;
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));

	test_pin = GPIO_NUMBER_4;
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));

	test_pin = GPIO_NUMBER_2;
	__cmock_gpio_remove_callback_IgnoreAndReturn(E_OK);
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));
}

void test_sid_gpio_irq_set_fail(void)
{
	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(-ENOTSUP);
	TEST_ASSERT_EQUAL(-ENOTSUP, sid_gpio_irq_set(test_pin, false));

	__cmock_sid_gpio_utils_port_pin_get_IgnoreAndReturn(E_OK);
	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(-EINVAL);
	TEST_ASSERT_EQUAL(-EINVAL, sid_gpio_irq_set(test_pin, true));
}

void test_sid_gpio_irq_set_pass(void)
{
	gpio_flags_t test_flag = GPIO_INT_EDGE_FALLING;

	test_pin = GPIO_NUMBER_1;

	__cmock_gpio_pin_interrupt_configure_IgnoreAndReturn(E_OK);
	__cmock_sid_gpio_utils_port_pin_get_Stub(port_pin_get_cb);

	__cmock_gpio_init_callback_ExpectAnyArgs();
	__cmock_gpio_add_callback_IgnoreAndReturn(E_OK);

	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_configure(test_pin, test_flag));
	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(TEST_PORT, test_pin, GPIO_INT_DISABLE, E_OK);
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_set(test_pin, false));

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(TEST_PORT, test_pin, test_flag, E_OK);
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_set(test_pin, true));

	__cmock_gpio_pin_interrupt_configure_ExpectAndReturn(TEST_PORT, test_pin, GPIO_INT_DISABLE, E_OK);
	TEST_ASSERT_EQUAL(E_OK, sid_gpio_irq_set(test_pin, false));
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
