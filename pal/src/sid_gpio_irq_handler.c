/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_gpio_irq_handler.c
 *  @brief GPIO interrupts handler.
 */
#include <zephyr/kernel.h>
#include <sid_gpio_irq_handler.h>
#include <sid_gpio_irq.h>
#include <sid_gpio_utils.h>

#ifndef CONFIG_SIDEWALK_GPIO_IRQ_PRIORITY
	#error "CONFIG_SIDEWALK_GPIO_IRQ_PRIORITY must be defined"
#endif

#ifndef CONFIG_SIDEWALK_GPIO_IRQ_STACK_SIZE
	#error "CONFIG_SIDEWALK_GPIO_IRQ_STACK_SIZE must be defined"
#endif

static K_SEM_DEFINE(gpio_trigger_sem, 0, 1);

typedef struct {
	sid_pal_gpio_irq_handler_t handler;
	void *args;
} sid_gpio_irq_cb_t;

static sid_gpio_irq_cb_t gpio_irq_cb[MAX_NUMBERS_OF_PINS];
volatile uint32_t gpio_number;

void sid_gpio_irq_callback(const struct device *gpiob,
			   struct gpio_callback *cb,
			   uint32_t pins)
{
	ARG_UNUSED(cb);

	if (!gpiob) {
		return;
	}

	gpio_number = sid_gpio_utils_gpio_number_get(gpiob, pins);
	k_sem_give(&gpio_trigger_sem);
}

void sid_gpio_irq_handler_set(uint32_t gpio_number,
			      sid_pal_gpio_irq_handler_t gpio_irq_handler,
			      void *callback_arg)
{
	if (NUMBERS_OF_PINS < gpio_number) {
		return;
	}

	if (gpio_irq_handler) {
		gpio_irq_cb[gpio_number].handler = gpio_irq_handler;
		gpio_irq_cb[gpio_number].args = callback_arg;
	}
}

static void gpio_task(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (1) {
		k_sem_take(&gpio_trigger_sem, K_FOREVER);
		if (__UINT32_MAX__ == gpio_number) {
			return;
		}

		if (gpio_irq_cb[gpio_number].handler) {
			gpio_irq_cb[gpio_number].handler(gpio_number, gpio_irq_cb[gpio_number].args);
		}
	}
}

K_THREAD_DEFINE(gpio_thread, CONFIG_SIDEWALK_GPIO_IRQ_STACK_SIZE, gpio_task, NULL, NULL, NULL,
		K_PRIO_COOP(CONFIG_SIDEWALK_GPIO_IRQ_PRIORITY), 0, 0);
