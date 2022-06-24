/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_gpio_irq_handler.c
 *  @brief GPIO interrupts handler.
 */

#include <sid_gpio_irq_handler.h>
#include <sid_gpio_irq.h>
#include <sid_gpio_utils.h>

typedef struct {
	sid_pal_gpio_irq_handler_t handler;
	void *args;
} sid_gpio_irq_cb_t;

static sid_gpio_irq_cb_t gpio_irq_cb[MAX_NUMBERS_OF_PINS];

void sid_gpio_irq_callback(const struct device *gpiob,
			   struct gpio_callback *cb,
			   uint32_t pins)
{
	ARG_UNUSED(cb);

	if (!gpiob) {
		return;
	}

	uint32_t gpio_number = sid_gpio_utils_gpio_number_get(gpiob, pins);

	if (__UINT32_MAX__ == gpio_number) {
		return;
	}

	if (gpio_irq_cb[gpio_number].handler) {
		gpio_irq_cb[gpio_number].handler(gpio_number, gpio_irq_cb[gpio_number].args);
	}
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
