/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_gpio_irq.c
 *  @brief GPIO interrupts implementation.
 */

#include <sid_gpio_irq.h>

#include <sid_gpio_utils.h>
#include <sid_gpio_irq_handler.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(sid_gpio_irq, CONFIG_SIDEWALK_LOG_LEVEL);

static gpio_flags_t interrupt_flags[MAX_NUMBERS_OF_PINS];
static struct gpio_callback gpio_cb[GPIO_COUNT];

int sid_gpio_irq_set(uint32_t gpio_number, bool set)
{
	gpio_port_pin_t port_pin;
	gpio_flags_t irq_flags = GPIO_INT_DISABLE;
	int erc = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);

	if (erc) {
		LOG_ERR("Port pin get error %d", erc);
		return erc;
	}

	if (set) {
		irq_flags = interrupt_flags[gpio_number];
	}

	erc = gpio_pin_interrupt_configure(port_pin.port, port_pin.pin, irq_flags);
	if (erc) {
		LOG_ERR("Interrupt configure error %d", erc);
	}

	return erc;
}

int sid_gpio_irq_configure(uint32_t gpio_number, gpio_flags_t irq_flags)
{
	int erc = 0;
	gpio_port_pin_t port_pin;
	uint8_t port_number = GPIO_NUM_TO_GPIO_PORT(gpio_number);
	struct gpio_callback *clbk = &gpio_cb[port_number];
	bool is_initialized = ((0 != clbk->pin_mask) ? true : false);

	erc = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);
	if (erc) {
		LOG_ERR("Port pin get error %d", erc);
		return erc;
	}

	WRITE_BIT(clbk->pin_mask, port_pin.pin, (GPIO_INT_DISABLE != (irq_flags & GPIO_INT_DISABLE)));

	if (!is_initialized) {
		if (GPIO_INT_DISABLE != irq_flags) {
			gpio_init_callback(clbk, sid_gpio_irq_callback, clbk->pin_mask);
			erc = gpio_add_callback(port_pin.port, clbk);
			if (erc) {
				WRITE_BIT(clbk->pin_mask, port_pin.pin, false);
				LOG_ERR("Add callback error %d", erc);
				return erc;
			}
		}
	} else {
		if (0 == clbk->pin_mask) {
			erc = gpio_remove_callback(port_pin.port, clbk);
			if (erc) {
				LOG_ERR("Remove callback error %d", erc);
				return erc;
			}
		}
	}

	interrupt_flags[gpio_number] = irq_flags;
	erc = gpio_pin_interrupt_configure(port_pin.port, port_pin.pin, irq_flags);
	if (erc) {
		LOG_ERR("Interrupt configure error %d", erc);
	}
	return erc;
}
