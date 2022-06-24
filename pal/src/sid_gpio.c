/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_gpio.c
 *  @brief GPIO interface implementation.
 */

#include <sid_pal_gpio_ifc.h>
#include <sid_gpio_irq.h>
#include <sid_gpio_irq_handler.h>
#include <sid_gpio_utils.h>

#define GPIO_SET_DIRECTION(dir)    ((dir == SID_PAL_GPIO_DIRECTION_INPUT) ? GPIO_INPUT : GPIO_OUTPUT)

static gpio_flags_t gpio_config_flags[MAX_NUMBERS_OF_PINS];

/**
 * @brief Convert GPIO error code to Sidewalk error code.
 *
 * @param gpio_erc - GPIO error code.
 * @return Sidewalk error code.
 */
static sid_error_t sid_error_get(int gpio_erc)
{
	sid_error_t sid_erc = SID_ERROR_NONE;

	if (0 > gpio_erc) {
		switch (gpio_erc) {
		case -EINVAL:
			sid_erc = SID_ERROR_INVALID_ARGS;
			break;
		case -ENOTSUP:
			sid_erc = SID_ERROR_NOSUPPORT;
			break;
		case -EIO:
			sid_erc = SID_ERROR_IO_ERROR;
			break;
		case -EBUSY:
			sid_erc = SID_ERROR_BUSY;
			break;
		default:
			sid_erc = SID_ERROR_GENERIC;
		}
	}
	return sid_erc;
}

sid_error_t sid_pal_gpio_read(uint32_t gpio_number, uint8_t *value)
{
	if (!value) {
		return SID_ERROR_NULL_POINTER;
	}
	return sid_error_get(sid_gpio_utils_gpio_read(gpio_number, value));
}

sid_error_t sid_pal_gpio_write(uint32_t gpio_number, uint8_t value)
{
	return sid_error_get(sid_gpio_utils_gpio_set(gpio_number, value));
}

sid_error_t sid_pal_gpio_toggle(uint32_t gpio_number)
{
	return sid_error_get(sid_gpio_utils_gpio_toggle(gpio_number));
}

sid_error_t sid_pal_gpio_set_direction(uint32_t gpio_number,
				       sid_pal_gpio_direction_t direction)
{
	if (!IN_RANGE(direction,
		      SID_PAL_GPIO_DIRECTION_INPUT,
		      SID_PAL_GPIO_DIRECTION_OUTPUT)) {
		return SID_ERROR_INVALID_ARGS;
	}

	gpio_port_pin_t port_pin;
	int erc = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);
	if (!erc) {
		gpio_config_flags[gpio_number] &= ~(GPIO_INPUT | GPIO_OUTPUT);
		gpio_config_flags[gpio_number] |= GPIO_SET_DIRECTION(direction);
		erc = gpio_pin_configure(port_pin.port, port_pin.pin, gpio_config_flags[gpio_number]);
	}
	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_input_mode(uint32_t gpio_number, sid_pal_gpio_input_t mode)
{
	if (!IN_RANGE(mode,
		      SID_PAL_GPIO_INPUT_CONNECT,
		      SID_PAL_GPIO_INPUT_DISCONNECT)) {
		return SID_ERROR_INVALID_ARGS;
	}

	gpio_port_pin_t port_pin;
	int erc = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);

	if (!erc) {
		if (GPIO_INPUT != (gpio_config_flags[gpio_number] & GPIO_INPUT)) {
			return SID_ERROR_INVALID_ARGS;
		}

		if (SID_PAL_GPIO_INPUT_DISCONNECT == mode) {
			erc = gpio_pin_configure(port_pin.port, port_pin.pin, GPIO_DISCONNECTED);
		} else {
			erc = gpio_pin_configure(port_pin.port, port_pin.pin, gpio_config_flags[gpio_number]);
		}
	}
	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_output_mode(uint32_t gpio_number, sid_pal_gpio_output_t mode)
{
	if (!IN_RANGE(mode,
		      SID_PAL_GPIO_OUTPUT_PUSH_PULL,
		      SID_PAL_GPIO_OUTPUT_OPEN_DRAIN)) {
		return SID_ERROR_INVALID_ARGS;
	}

	gpio_port_pin_t port_pin;
	int erc = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);
	if (!erc) {
		if (GPIO_OUTPUT != (gpio_config_flags[gpio_number] & GPIO_OUTPUT)) {
			return SID_ERROR_INVALID_ARGS;
		}

		if (mode == SID_PAL_GPIO_OUTPUT_OPEN_DRAIN) {
			gpio_config_flags[gpio_number] &= ~GPIO_PUSH_PULL;
			gpio_config_flags[gpio_number] |= GPIO_OPEN_DRAIN;
		} else {
			gpio_config_flags[gpio_number] &= ~(GPIO_OPEN_DRAIN);
			gpio_config_flags[gpio_number] |= GPIO_PUSH_PULL;
		}
		erc = gpio_pin_configure(port_pin.port, port_pin.pin, gpio_config_flags[gpio_number]);
	}

	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_pull_mode(uint32_t gpio_number, sid_pal_gpio_pull_t pull)
{
	gpio_port_pin_t port_pin;
	int erc = sid_gpio_utils_port_pin_get(gpio_number, &port_pin);

	if (!erc) {
		switch (pull) {
		case SID_PAL_GPIO_PULL_NONE:
			gpio_config_flags[gpio_number] &= ~(GPIO_PULL_UP | GPIO_PULL_DOWN);
			break;
		case SID_PAL_GPIO_PULL_UP:
			gpio_config_flags[gpio_number] &= ~GPIO_PULL_DOWN;
			gpio_config_flags[gpio_number] |= GPIO_PULL_UP;
			break;
		case SID_PAL_GPIO_PULL_DOWN:
			gpio_config_flags[gpio_number] &= ~GPIO_PULL_UP;
			gpio_config_flags[gpio_number] |= GPIO_PULL_DOWN;
			break;
		default:
			return SID_ERROR_INVALID_ARGS;
		}
		erc = gpio_pin_configure(port_pin.port, port_pin.pin, gpio_config_flags[gpio_number]);
	}

	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_set_irq(uint32_t gpio_number, sid_pal_gpio_irq_trigger_t irq_trigger,
				 sid_pal_gpio_irq_handler_t gpio_irq_handler, void *callback_arg)
{
	gpio_flags_t irq_flags = GPIO_INT_DISABLE;

	switch (irq_trigger) {
	case SID_PAL_GPIO_IRQ_TRIGGER_NONE:
		irq_flags = GPIO_INT_DISABLE;
		break;
	case SID_PAL_GPIO_IRQ_TRIGGER_RISING:
		irq_flags = GPIO_INT_EDGE_RISING;
		break;
	case SID_PAL_GPIO_IRQ_TRIGGER_FALLING:
		irq_flags = GPIO_INT_EDGE_FALLING;
		break;
	case SID_PAL_GPIO_IRQ_TRIGGER_EDGE:
		irq_flags = GPIO_INT_EDGE_BOTH;
		break;
	case SID_PAL_GPIO_IRQ_TRIGGER_LOW:
		irq_flags = GPIO_INT_LEVEL_LOW;
		break;
	case SID_PAL_GPIO_IRQ_TRIGGER_HIGH:
		irq_flags = GPIO_INT_LEVEL_HIGH;
		break;
	default:
		return SID_ERROR_INVALID_ARGS;
		break;
	}

	if (SID_PAL_GPIO_IRQ_TRIGGER_NONE != irq_trigger) {
		sid_gpio_irq_handler_set(gpio_number, gpio_irq_handler, callback_arg);
	} else {
		sid_gpio_irq_handler_set(gpio_number, NULL, NULL);
	}
	return sid_error_get(sid_gpio_irq_configure(gpio_number, irq_flags));
}

sid_error_t sid_pal_gpio_irq_enable(uint32_t gpio_number)
{
	return sid_error_get(sid_gpio_irq_set(gpio_number, true));
}

sid_error_t sid_pal_gpio_irq_disable(uint32_t gpio_number)
{
	return sid_error_get(sid_gpio_irq_set(gpio_number, false));
}
