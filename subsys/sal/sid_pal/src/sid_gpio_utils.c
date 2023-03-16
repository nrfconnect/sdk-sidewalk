/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_gpio_utils.c
 *  @brief GPIO utils.
 */

#include <sid_gpio_utils.h>

static const struct device *gpio_port[] = {
#if DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay)
	DEVICE_DT_GET(DT_NODELABEL(gpio0)),
#else
	NULL,
#endif /* DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay) */

#if DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay)
	DEVICE_DT_GET(DT_NODELABEL(gpio1)),
#else
	NULL,
#endif  /* DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay) */
};

static uint32_t sid_gpio_utils_mask_to_pin_number(uint32_t pin_mask)
{
	for (int pin = 0; pin < (1U << GPIO_PIN_MASK); pin++) {
		if (0 != (pin_mask & (1U << pin))) {
			return pin;
		}
	}
	return __UINT32_MAX__;
}

int sid_gpio_utils_port_pin_get(uint32_t gpio_number, gpio_port_pin_t *port_pin)
{
	if (!port_pin) {
		return -ENOENT;
	}

	if (NUMBERS_OF_PINS < gpio_number) {
		return -ENOTSUP;
	}

	int gpio_port_number = GPIO_NUM_TO_GPIO_PORT(gpio_number);

	if (GPIO_COUNT < gpio_port_number) {
		return -EINVAL;
	}

	const struct device *gpio = gpio_port[gpio_port_number];

	if (!gpio) {
		return -EINVAL;
	}

	port_pin->port = gpio;
	port_pin->pin = GPIO_NUM_TO_GPIO_PIN(gpio_number);

	return 0;
}

uint32_t sid_gpio_utils_gpio_number_get(const struct device *port, uint32_t pin_mask)
{
	if (!port) {
		return __UINT32_MAX__;
	}

	for (int port_number = 0; port_number < ARRAY_SIZE(gpio_port); port_number++) {
		if (port == gpio_port[port_number]) {
			int pin = sid_gpio_utils_mask_to_pin_number(pin_mask);
			if (__UINT32_MAX__ == pin) {
				return __UINT32_MAX__;
			}
			return ((1 << GPIO_PIN_MASK) * port_number) + pin;
		}
	}
	return __UINT32_MAX__;
}

int sid_gpio_utils_gpio_read(uint32_t gpio_number, uint8_t *value)
{
	if (!value) {
		return -ENOENT;
	}

	gpio_port_pin_t port_pin;
	int ret_val = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);

	if (ret_val) {
		return ret_val;
	}

	ret_val = gpio_pin_get_raw(port_pin.port, port_pin.pin);
	*value = ret_val;
	return ret_val;
}

int sid_gpio_utils_gpio_set(uint32_t gpio_number, uint8_t value)
{
	gpio_port_pin_t port_pin;
	int ret_val = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);

	if (ret_val) {
		return ret_val;
	}

	return gpio_pin_set_raw(port_pin.port, port_pin.pin, value);
}

int sid_gpio_utils_gpio_toggle(uint32_t gpio_number)
{
	gpio_port_pin_t port_pin;
	int ret_val = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);

	if (ret_val) {
		return ret_val;
	}

	return gpio_pin_toggle(port_pin.port, port_pin.pin);
}
