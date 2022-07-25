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
	struct k_work work;
} sid_gpio_irq_cb_t;

static sid_gpio_irq_cb_t gpio_irq_cb[MAX_NUMBERS_OF_PINS];

static void gpio_work_handler(struct k_work *w)
{
	sid_gpio_irq_cb_t *irq_cb = CONTAINER_OF(w, sid_gpio_irq_cb_t, work);
	uint32_t number = ((size_t)irq_cb - (size_t)gpio_irq_cb) / sizeof(sid_gpio_irq_cb_t);

	if (irq_cb->handler) {
		irq_cb->handler(number, irq_cb->args);
	}
}

// called from irq
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
		k_work_submit(&gpio_irq_cb[gpio_number].work);
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
		k_work_init(&gpio_irq_cb[gpio_number].work, gpio_work_handler);
	}
}
