/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef SID_GPIO_UTILS_H
#define SID_GPIO_UTILS_H

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <nrf_peripherals.h>

#define GPIO_PORT0 (0)
#define GPIO_PORT1 (1)

#define MAX_NUMBERS_OF_PINS (P0_PIN_NUM + P1_PIN_NUM)

#define GPIO_PIN_MASK (5)

#define GPIO_NUM_TO_GPIO_PORT(_num) ((_num >> GPIO_PIN_MASK) & 0x0F)
#define GPIO_NUM_TO_GPIO_PIN(_num)  (_num & (GPIO_MAX_PINS_PER_PORT - 1))

#define NUMBERS_OF_PINS														      \
	(DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay) && DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay) ? (MAX_NUMBERS_OF_PINS - 1) : \
	 ((DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay) ? (P0_PIN_NUM - 1) :							      \
	   (DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay) ? (P1_PIN_NUM - 1) : 0))))

typedef struct {
	const struct device *port;
	gpio_pin_t pin;
} gpio_port_pin_t;

/**
 * @brief Convert Sidewalk GPIO number to GPIO port and pin number.
 *
 * @param gpio_number - GPIO number.
 * @param port_pin - pointer to object where port and pin can be storage.
 * @return 0 when pin and port can be set, error code otherwise.
 *
 * NOTE: GPIO port number is mapped to device ID.
 */
int sid_gpio_utils_port_pin_get(uint32_t gpio_number, gpio_port_pin_t *port_pin);

/**
 * @brief Convert GPIO port and pin mask to GPIO pin number.
 *
 * @param port - GPIO port.
 * @param pin_mask - GPIO pin mask.
 * @return GPIO number or __UINT32_MAX__ if pin_mask or port is incorrect.
 */
uint32_t sid_gpio_utils_gpio_number_get(const struct device *port, uint32_t pin_mask);

/**
 * @brief Get GPIO pin state.
 *
 * @param gpio_number - GPIO pin number.
 * @param value - pointer to value.
 * @return positive value when success, error code otherwise.
 */
int sid_gpio_utils_gpio_read(uint32_t gpio_number, uint8_t *value);

/**
 * @brief Set GPIO pin state.
 *
 * @param gpio_number - GPIO pin number.
 * @param value - if 0 the pin will be set to low physical level,
 * 				other value will set it to high physical level.
 * @return 0 when success, error code otherwise.
 */
int sid_gpio_utils_gpio_set(uint32_t gpio_number, uint8_t value);

/**
 * @brief toggle the GPIO pin.
 *
 * @param gpio_number - GPIO pin number.
 * @return 0 when success, error code otherwise.
 */
int sid_gpio_utils_gpio_toggle(uint32_t gpio_number);

#endif /* SID_GPIO_UTILS_H */
