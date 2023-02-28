/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef SID_GPIO_IRQ_HANDLER_H
#define SID_GPIO_IRQ_HANDLER_H

#include <sid_pal_gpio_ifc.h>
#include <zephyr/drivers/gpio.h>

/**
 * @brief GPIO application callback.
 *
 * @param gpiob Device struct for the GPIO device.
 * @param cb Original struct gpio_callback owning this handler
 * @param pins Mask of pins that triggers the callback handler
 */
void sid_gpio_irq_callback(const struct device *gpiob,
			   struct gpio_callback *cb,
			   uint32_t pins);

/**
 * @brief Set IRQ handler for GPIO pin.
 *
 * @param gpio_number - logical GPIO number.
 * @param gpio_irq_handler - the callback.
 * @param callback_arg - callback arguments.
 */
void sid_gpio_irq_handler_set(uint32_t gpio_number,
			      sid_pal_gpio_irq_handler_t gpio_irq_handler,
			      void *callback_arg);

#endif /* #define SID_GPIO_IRQ_HANDLER_H */
