/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_GPIO_UTILS_H
#define SID_GPIO_UTILS_H

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <sid_pal_gpio_ifc.h>

#define GPIO_UNUSED_PIN (uint32_t)(-1)

typedef struct {
	const struct device *port;
	gpio_pin_t pin;
} gpio_port_pin_t;

/**
 * @brief clear internal context of gpio_utils
 * 
 * note: This function does not modify any registers of GPIOs, it only clears its configuration in internal structures of sid_gpio_utils module
 * make sure to clear all flags of used GPIOs before calling this function.
 */
void sid_gpio_utils_clear_register(void);

/**
 * @brief Assign GPIO from DT to use with Sidewalk GPIO API
 * 
 * @param gpio_from_dts - DT_SPEC of the GPIO node, use macro #GPIO_DT_SPEC_GET_OR for more info
 * @return int - gpio ID to use with Sidewalk API, or (uint32_t)(-1) in case of error
 */
uint32_t sid_gpio_utils_register_gpio(struct gpio_dt_spec gpio_from_dts);

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

/**
 * @brief Overwrite gpio flags for gpio
 * 
 * @param gpio_number - GPIO pin number
 * @param flag - desired flag state
 * @return int - ERRNO status code, check function gpio_pin_configure for expected return codes
 */
int sid_gpio_utils_gpio_set_flags(uint32_t gpio_number, gpio_flags_t flag);

/**
 * @brief set the flags to GPIO_DISCONNECT
 * 
 * @param gpio_number - gpio to disconnect
 * @return int - ERRNO status code, check function gpio_pin_configure for expected return codes
 */
int sid_gpio_utils_disconnect(uint32_t gpio_number);

/**
 * @brief Read flags of selected gpio
 * 
 * @param gpio_number - GPIO pin number 
 * @param flag - pointer where to store readed flags
 * @return int - ERRNO status code
 */
int sid_gpio_utils_gpio_get_flags(uint32_t gpio_number, gpio_flags_t *flag);

/**
 * @brief Set Sidewalk handler for trigger in gpio
 * 
 * @param gpio_number - GPIO pin number
 * @param gpio_irq_handler - function callback to execute on trigger
 * @param callback_arg - pointer to arguments for callback
 * @return int - ERRNO status code
 */
int sid_gpio_utils_irq_handler_set(uint32_t gpio_number,
				   sid_pal_gpio_irq_handler_t gpio_irq_handler, void *callback_arg);

/**
 * @brief Set IRQ flags bit for the GPIO
 * 
 * @param gpio_number - GPIO pin number
 * @param irq_flags - bit mask of flags to set
 * @return int - ERRNO status code
 */
int sid_gpio_utils_irq_configure(uint32_t gpio_number, gpio_flags_t irq_flags);

/**
 * @brief Controll GPIO_INT_DISABLE bit field of flags for GPIO
 * 
 * @param gpio_number - GPIO pin number
 * @param set - if true, clear GPIO_INT_DISABLE otherwise set GPIO_INT_DISABLE in gpio flags 
 * @return int - ERRNO status code
 */
int sid_gpio_utils_irq_set(uint32_t gpio_number, bool set);

#endif /* SID_GPIO_UTILS_H */
