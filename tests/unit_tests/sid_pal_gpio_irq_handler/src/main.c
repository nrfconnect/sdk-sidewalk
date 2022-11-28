/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_gpio_irq_handler.h>
#include <cmock_sid_gpio_utils.h>
#include <unity.h>

#define E_OK    (0)

#define TEST_LOOP       (10)

#define GPIO_NUMBER_1   (1)
#define GPIO_NUMBER_9   (9)

#define GPIO_CALLBACK_NUM       (2)

#define TEST_PORT (DEVICE_DT_GET(DT_NODELABEL(gpio0)))

static int test_arg = 0xFAF177A9;
static int cb_func_call_cnt = 0;
static int *cb_func_arg;

void setUp(void)
{
	cb_func_call_cnt = 0;
	cb_func_arg = NULL;
	cmock_sid_gpio_utils_Init();
}

void tearDown(void)
{
	cmock_sid_gpio_utils_Destroy();
}

static void test_callback(uint32_t gpio_number, void *callback_arg)
{
	++cb_func_call_cnt;
	cb_func_arg = (int *)callback_arg;
}

void test_sid_gpio_irq_handler_set_fail(void)
{
	__cmock_sid_gpio_utils_gpio_number_get_IgnoreAndReturn(GPIO_NUMBER_1);
	sid_gpio_irq_handler_set(GPIO_NUMBER_1, NULL, &test_arg);
	sid_gpio_irq_callback(TEST_PORT, NULL, BIT(GPIO_NUMBER_1));
	TEST_ASSERT_EQUAL(0, cb_func_call_cnt);
	TEST_ASSERT_NULL(cb_func_arg);
}

void test_sid_gpio_irq_handler_set_pass(void)
{
	__cmock_sid_gpio_utils_gpio_number_get_IgnoreAndReturn(GPIO_NUMBER_1);
	sid_gpio_irq_handler_set(GPIO_NUMBER_1, test_callback, &test_arg);

	for (int it = 1; it <= TEST_LOOP; it++) {
		sid_gpio_irq_callback(TEST_PORT, NULL, BIT(GPIO_NUMBER_1));
		TEST_ASSERT_EQUAL(it, cb_func_call_cnt);
		TEST_ASSERT_EQUAL(*cb_func_arg, test_arg);
	}

	cb_func_call_cnt = 0;
	cb_func_arg = NULL;
	sid_gpio_irq_handler_set(GPIO_NUMBER_9, test_callback, &test_arg);

	for (int gpio = 0; gpio < P0_PIN_NUM; gpio++) {
		__cmock_sid_gpio_utils_gpio_number_get_IgnoreAndReturn(gpio);
		sid_gpio_irq_callback(TEST_PORT, NULL, BIT(gpio));
	}
	TEST_ASSERT_EQUAL(GPIO_CALLBACK_NUM, cb_func_call_cnt);
	TEST_ASSERT_EQUAL(*cb_func_arg, test_arg);
}

void test_sid_gpio_irq_callback_fail(void)
{
	for (int gpio = 0; gpio < P0_PIN_NUM; gpio++) {
		sid_gpio_irq_handler_set(gpio, test_callback, &test_arg);
	}

	for (int gpio = 0; gpio < P0_PIN_NUM; gpio++) {
		sid_gpio_irq_callback(NULL, NULL, BIT(gpio));
		TEST_ASSERT_EQUAL(0, cb_func_call_cnt);
		TEST_ASSERT_NULL(cb_func_arg);
	}

	__cmock_sid_gpio_utils_gpio_number_get_IgnoreAndReturn(__UINT32_MAX__);

	for (int gpio = 0; gpio < P0_PIN_NUM; gpio++) {
		sid_gpio_irq_callback(TEST_PORT, NULL, BIT(gpio));
		TEST_ASSERT_EQUAL(0, cb_func_call_cnt);
		TEST_ASSERT_NULL(cb_func_arg);
	}
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
