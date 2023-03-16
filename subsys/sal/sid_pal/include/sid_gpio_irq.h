/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_GPIO_IRQ_H
#define SID_GPIO_IRQ_H

#include <zephyr/drivers/gpio.h>

/**
 * @brief Enable or disable interrupt for GPIO pin.
 *
 * @param gpio_number - gpio number.
 * @param set - true when interrupt should be enabled.
 * @return 0 when success, error code otherwise.
 */
int sid_gpio_irq_set(uint32_t gpio_number, bool set);

/**
 * @brief Configure interrupt for GPIO pin.
 *
 * @param gpio_number - gpio number.
 * @param irq_flags - interrupt configuration flags.
 * @return 0 when success, error code otherwise.
 */
int sid_gpio_irq_configure(uint32_t gpio_number, gpio_flags_t irq_flags);

#endif /* SID_GPIO_IRQ_H */
