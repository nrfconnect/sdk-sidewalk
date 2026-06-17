/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef GPIO_MOCK_H
#define GPIO_MOCK_H

#include <zephyr/fff.h>
#include <stdint.h>

struct device;
struct gpio_callback;
typedef void (*gpio_callback_handler_t)(const struct device *, struct gpio_callback *, uint32_t);
typedef uint32_t gpio_port_pins_t;
typedef unsigned int gpio_pin_t;
typedef unsigned long gpio_flags_t;

FAKE_VALUE_FUNC(int, gpio_pin_configure, const struct device *, gpio_pin_t, gpio_flags_t);
FAKE_VALUE_FUNC(int, gpio_pin_get_raw, const struct device *, gpio_pin_t);
FAKE_VALUE_FUNC(int, gpio_pin_set_raw, const struct device *, gpio_pin_t, int);
FAKE_VALUE_FUNC(int, gpio_pin_toggle, const struct device *, gpio_pin_t);
FAKE_VALUE_FUNC(int, gpio_pin_interrupt_configure, const struct device *, gpio_pin_t, gpio_flags_t);
FAKE_VOID_FUNC(gpio_init_callback, struct gpio_callback *, gpio_callback_handler_t,
	       gpio_port_pins_t);
FAKE_VALUE_FUNC(int, gpio_add_callback, const struct device *, struct gpio_callback *);
FAKE_VALUE_FUNC(int, gpio_remove_callback, const struct device *, struct gpio_callback *);

#define FFF_FAKES_LIST_GPIO(FAKE)                                                                  \
	FAKE(gpio_pin_configure)                                                                   \
	FAKE(gpio_pin_get_raw)                                                                     \
	FAKE(gpio_pin_set_raw)                                                                     \
	FAKE(gpio_pin_toggle)                                                                      \
	FAKE(gpio_pin_interrupt_configure)                                                         \
	FAKE(gpio_init_callback)                                                                   \
	FAKE(gpio_add_callback)                                                                    \
	FAKE(gpio_remove_callback)

#endif /* GPIO_MOCK_H */
