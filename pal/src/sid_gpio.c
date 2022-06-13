/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_gpio.c
 *  @brief GPIO interface implementation.
 */

#include <sid_pal_gpio_ifc.h>

#include <devicetree.h>
#include <drivers/gpio.h>
#include <nrf_peripherals.h>

#define GPIO_PORT0 (0)
#define GPIO_PORT1 (1)

#define MAX_NUMBERS_OF_PINS (P0_PIN_NUM + P1_PIN_NUM)

#define GPIO_PIN_MASK (5)

#define GPIO_NUM_TO_GPIO_PORT(_num) ((_num >> GPIO_PIN_MASK) & 0x0F)
#define GPIO_NUM_TO_GPIO_PIN(_num)  (_num & (GPIO_MAX_PINS_PER_PORT - 1))

#define IS_ARG_IN_RANGE(_arg, _range_min, _range_max)   ((_range_max >= _arg) && \
							 (_range_min <= _arg))

#define NUMBERS_OF_PINS																  \
	(DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay) && DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay) ? (MAX_NUMBERS_OF_PINS - 1) : \
	 ((DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay) ? (P0_PIN_NUM - 1) :								  \
	   (DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay) ? (P1_PIN_NUM - 1) : 0))))

#define GPIO_SET_DIRECTION(dir)    ((dir == SID_PAL_GPIO_DIRECTION_INPUT) ? GPIO_INPUT : GPIO_OUTPUT)

typedef struct {
	const struct device *port;
	gpio_pin_t pin;
} gpio_port_pin_t;

typedef struct {
	gpio_flags_t flags;
} gpio_config_t;

static gpio_config_t gpio_config[MAX_NUMBERS_OF_PINS];

#if defined (NRF52840_XXAA)
#if DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay)
static const struct device *gpio_port0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
#endif /* DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay) */

#if DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay)
static const struct device *gpio_port1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
#endif  /* DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay) */
#endif  /* defined (NRF52840_XXAA) */

/**
 * @brief Convert GPIO number to GPIO port and pin number.
 *
 * @param gpio_number - GPIO number.
 * @param port_pin - pointer to object where port and pin can be storage.
 * @return 0 when pin and port can be set, error code otherwise.
 *
 * NOTE: GPIO port number is mapped to device ID.
 */
static int gpio_port_pin_get(uint32_t gpio_number, gpio_port_pin_t *port_pin)
{
	if (!port_pin) {
		return -ENOENT;
	}

	if (NUMBERS_OF_PINS < gpio_number) {
		return -ENOTSUP;
	}

	int gpio_port_number = GPIO_NUM_TO_GPIO_PORT(gpio_number);

	switch (gpio_port_number) {
	#if DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay)
	case GPIO_PORT0:
		port_pin->port = gpio_port0;
		break;
	#endif /* DT_NODE_HAS_STATUS(DT_NODELABEL(gpio0), okay) */
	#if DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay)
	case GPIO_PORT1:
		port_pin->port = gpio_port1;
		break;
	#endif /* DT_NODE_HAS_STATUS(DT_NODELABEL(gpio1), okay) */
	default:
		return -EINVAL;
	}

	port_pin->pin = GPIO_NUM_TO_GPIO_PIN(gpio_number);

	return 0;
}

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

sid_error_t sid_pal_gpio_set_direction(uint32_t gpio_number,
				       sid_pal_gpio_direction_t direction)
{
	if (!IS_ARG_IN_RANGE(direction,
			     SID_PAL_GPIO_DIRECTION_INPUT,
			     SID_PAL_GPIO_DIRECTION_OUTPUT)) {
		return SID_ERROR_INVALID_ARGS;
	}

	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);
	if (!erc) {
		gpio_config[gpio_number].flags |= GPIO_SET_DIRECTION(direction);
		erc = gpio_pin_configure(port_pin.port, port_pin.pin, gpio_config[gpio_number].flags);
	}

	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_read(uint32_t gpio_number, uint8_t *value)
{
	if (!value) {
		return SID_ERROR_NULL_POINTER;
	}

	gpio_port_pin_t port_pin;
	int ret_val = gpio_port_pin_get(gpio_number, &port_pin);
	if (!ret_val) {
		ret_val = gpio_pin_get_raw(port_pin.port, port_pin.pin);
		*value = ret_val;
	}
	return sid_error_get(ret_val);
}

sid_error_t sid_pal_gpio_write(uint32_t gpio_number, uint8_t value)
{
	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);

	if (!erc) {
		erc = gpio_pin_set_raw(port_pin.port, port_pin.pin, value);
	}
	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_toggle(uint32_t gpio_number)
{
	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);

	if (!erc) {
		erc = gpio_pin_toggle(port_pin.port, port_pin.pin);
	}
	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_input_mode(uint32_t gpio_number, sid_pal_gpio_input_t mode)
{
	if (!IS_ARG_IN_RANGE(mode,
			     SID_PAL_GPIO_INPUT_CONNECT,
			     SID_PAL_GPIO_INPUT_DISCONNECT)) {
		return SID_ERROR_INVALID_ARGS;
	}

	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);

	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_output_mode(uint32_t gpio_number, sid_pal_gpio_output_t mode)
{
	if (!IS_ARG_IN_RANGE(mode,
			     SID_PAL_GPIO_OUTPUT_PUSH_PULL,
			     SID_PAL_GPIO_OUTPUT_OPEN_DRAIN)) {
		return SID_ERROR_INVALID_ARGS;
	}

	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);
	if (!erc) {
		if (GPIO_INPUT == (gpio_config[gpio_number].flags & GPIO_INPUT)) {
			return SID_ERROR_INVALID_ARGS;
		}

		if (mode == SID_PAL_GPIO_OUTPUT_OPEN_DRAIN) {
			gpio_config[gpio_number].flags |= GPIO_OPEN_DRAIN;
		} else {
			gpio_config[gpio_number].flags |= GPIO_PUSH_PULL;
		}
		erc = gpio_pin_configure(port_pin.port, port_pin.pin, gpio_config[gpio_number].flags);
	}

	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_pull_mode(uint32_t gpio_number, sid_pal_gpio_pull_t pull)
{
	if (!IS_ARG_IN_RANGE(pull,
			     SID_PAL_GPIO_PULL_NONE,
			     SID_PAL_GPIO_PULL_DOWN)) {
		return SID_ERROR_INVALID_ARGS;
	}

	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);
	if (!erc) {
		switch (pull) {
		case SID_PAL_GPIO_PULL_NONE:
			gpio_config[gpio_number].flags &= ~(GPIO_PULL_UP | GPIO_PULL_DOWN);
			break;
		case SID_PAL_GPIO_PULL_UP:
			gpio_config[gpio_number].flags &= ~GPIO_PULL_DOWN;
			gpio_config[gpio_number].flags |= GPIO_PULL_UP;
			break;
		case SID_PAL_GPIO_PULL_DOWN:
			gpio_config[gpio_number].flags &= ~GPIO_PULL_UP;
			gpio_config[gpio_number].flags |= GPIO_PULL_DOWN;
			break;
		}
		erc = gpio_pin_configure(port_pin.port, port_pin.pin, gpio_config[gpio_number].flags);
	}

	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_set_irq(uint32_t gpio_number, sid_pal_gpio_irq_trigger_t irq_trigger,
				 sid_pal_gpio_irq_handler_t gpio_irq_handler, void *callback_arg)
{
	if (!IS_ARG_IN_RANGE(irq_trigger,
			     SID_PAL_GPIO_IRQ_TRIGGER_NONE,
			     SID_PAL_GPIO_IRQ_TRIGGER_HIGH)) {
		return SID_ERROR_INVALID_ARGS;
	}

	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);
	if (!erc) {
		erc = gpio_pin_interrupt_configure(port_pin.port, port_pin.pin, GPIO_INT_ENABLE);
	}
	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_irq_enable(uint32_t gpio_number)
{
	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);

	if (!erc) {
		erc = gpio_pin_interrupt_configure(port_pin.port, port_pin.pin, GPIO_INT_ENABLE);
	}
	return sid_error_get(erc);
}

sid_error_t sid_pal_gpio_irq_disable(uint32_t gpio_number)
{
	gpio_port_pin_t port_pin;
	int erc = gpio_port_pin_get(gpio_number, &port_pin);

	if (!erc) {
		erc = gpio_pin_interrupt_configure(port_pin.port, port_pin.pin, GPIO_INT_DISABLE);
	}
	return sid_error_get(erc);
}
