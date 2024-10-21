/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_gpio_utils.c
 *  @brief GPIO utils.
 */

#include "zephyr/drivers/gpio.h"
#include "zephyr/kernel.h"
#include "zephyr/sys/util.h"
#include <sid_gpio_utils.h>
#include <stddef.h>
#include <string.h>

#include <stdint.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_gpio_util, CONFIG_SIDEWALK_LOG_LEVEL);

struct sid_gpio_util_pin {
	gpio_port_pin_t gpio;
	gpio_flags_t configuration_cache;
	struct gpio_callback callback;

	struct k_work handler_worker;
	sid_pal_gpio_irq_handler_t gpio_irq_handler;
	void *handler_arg;
};
static struct ctx {
	struct sid_gpio_util_pin supported_pins[CONFIG_SIDEWALK_GPIO_MAX];
	uint32_t next_free_slot;
} ctx;

K_THREAD_STACK_DEFINE(sidewalk_gpio_workq_stack, CONFIG_SIDEWALK_GPIO_IRQ_STACK_SIZE);
struct k_work_q sidewalk_gpio_workq;

#define CHECK_IF_GPIO_IS_REGISTERED(gpio_number)                                                   \
	if (gpio_number == GPIO_UNUSED_PIN) {                                                      \
		return -ENOTSUP;                                                                   \
	}                                                                                          \
	if (gpio_number >= ctx.next_free_slot) {                                                   \
		return -EINVAL;                                                                    \
	}

void sid_gpio_utils_clear_register(void)
{
	memset(&ctx, 0, sizeof(ctx));
}

static uint32_t find_gpio_in_register(struct gpio_dt_spec gpio_from_dts)
{
	for (uint32_t i = 0; i < ctx.next_free_slot; i++) {
		if (ctx.supported_pins[i].gpio.port == gpio_from_dts.port &&
		    ctx.supported_pins[i].gpio.pin == gpio_from_dts.pin) {
			return i;
		}
	}
	return ctx.next_free_slot;
}

uint32_t sid_gpio_utils_register_gpio(struct gpio_dt_spec gpio_from_dts)
{
	if (ctx.next_free_slot >= CONFIG_SIDEWALK_GPIO_MAX)
		return -ENOSPC;

	if (gpio_from_dts.port == NULL) {
		return GPIO_UNUSED_PIN;
	}
	int gpio = find_gpio_in_register(gpio_from_dts);
	if (gpio < ctx.next_free_slot) {
		return gpio;
	}
	ctx.supported_pins[ctx.next_free_slot] =
		(struct sid_gpio_util_pin){ .gpio = { .port = gpio_from_dts.port,
						      .pin = gpio_from_dts.pin },
					    .configuration_cache = gpio_from_dts.dt_flags };

	sid_gpio_utils_gpio_set_flags(ctx.next_free_slot,
				      ctx.supported_pins[ctx.next_free_slot].configuration_cache);
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		k_work_queue_init(&sidewalk_gpio_workq);
		k_work_queue_start(&sidewalk_gpio_workq, sidewalk_gpio_workq_stack,
				   CONFIG_SIDEWALK_GPIO_IRQ_STACK_SIZE,
				   K_PRIO_COOP(CONFIG_SIDEWALK_GPIO_IRQ_PRIORITY), NULL);
	}
	return ctx.next_free_slot++;
}

int sid_gpio_utils_gpio_read(uint32_t gpio_number, uint8_t *value)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)

	if (!(ctx.supported_pins[gpio_number].configuration_cache & GPIO_INPUT)) {
		LOG_ERR("Reading from pin that is not input %d", gpio_number);
		return -EINVAL;
	}

	int ret_val = gpio_pin_get_raw(ctx.supported_pins[gpio_number].gpio.port,
				       ctx.supported_pins[gpio_number].gpio.pin);
	if (ret_val < 0) {
		LOG_ERR("GPIO read returned error %d", ret_val);
		return -EIO;
	}
	*value = ret_val;

	return SID_ERROR_NONE;
}

int sid_gpio_utils_gpio_set(uint32_t gpio_number, uint8_t value)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)
	if (!(ctx.supported_pins[gpio_number].configuration_cache & GPIO_OUTPUT)) {
		LOG_ERR("Can not set value to GPIO that is not OUTPUT %d", gpio_number);
		return -EINVAL;
	}

	return gpio_pin_set_raw(ctx.supported_pins[gpio_number].gpio.port,
				ctx.supported_pins[gpio_number].gpio.pin, value);
}

int sid_gpio_utils_gpio_toggle(uint32_t gpio_number)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)
	if (!(ctx.supported_pins[gpio_number].configuration_cache & GPIO_OUTPUT)) {
		LOG_ERR("Can not toggle value to GPIO that is not OUTPUT %d", gpio_number);
		return -EINVAL;
	}

	return gpio_pin_toggle(ctx.supported_pins[gpio_number].gpio.port,
			       ctx.supported_pins[gpio_number].gpio.pin);
}

int sid_gpio_utils_gpio_set_flags(uint32_t gpio_number, gpio_flags_t flag)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)
	ctx.supported_pins[gpio_number].configuration_cache &= GPIO_INT_MASK;
	ctx.supported_pins[gpio_number].configuration_cache |= (flag & ~GPIO_INT_MASK);
	return gpio_pin_configure(
		ctx.supported_pins[gpio_number].gpio.port, ctx.supported_pins[gpio_number].gpio.pin,
		ctx.supported_pins[gpio_number].configuration_cache & ~GPIO_INT_MASK);
}

int sid_gpio_utils_disconnect(uint32_t gpio_number)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)
	return gpio_pin_configure(ctx.supported_pins[gpio_number].gpio.port,
				  ctx.supported_pins[gpio_number].gpio.pin, GPIO_DISCONNECTED);
}

int sid_gpio_utils_gpio_get_flags(uint32_t gpio_number, gpio_flags_t *flag)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)

	if (!flag) {
		return -ENOENT;
	}

	*flag = ctx.supported_pins[gpio_number].configuration_cache;
	return 0;
}

static void sid_gpio_utils_irq_worker(struct k_work *w)
{
	struct sid_gpio_util_pin *pin = CONTAINER_OF(w, struct sid_gpio_util_pin, handler_worker);
	int id = ((char *)pin - (char *)ctx.supported_pins) / sizeof(*pin);
	if (pin->gpio_irq_handler) {
		pin->gpio_irq_handler(id, pin->handler_arg);
	}
}

int sid_gpio_utils_irq_handler_set(uint32_t gpio_number,
				   sid_pal_gpio_irq_handler_t gpio_irq_handler, void *callback_arg)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)

	ctx.supported_pins[gpio_number].gpio_irq_handler = gpio_irq_handler;
	ctx.supported_pins[gpio_number].handler_arg = callback_arg;
	k_work_init(&ctx.supported_pins[gpio_number].handler_worker, sid_gpio_utils_irq_worker);
	return 0;
}

static void sid_gpio_irq_callback(const struct device *gpio, struct gpio_callback *cb,
				  uint32_t pins)
{
	if (!gpio) {
		return;
	}
	struct sid_gpio_util_pin *pin = CONTAINER_OF(cb, struct sid_gpio_util_pin, callback);
	k_work_submit_to_queue(&sidewalk_gpio_workq, &pin->handler_worker);
}

int sid_gpio_utils_irq_configure(uint32_t gpio_number, gpio_flags_t irq_flags)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)
	int erc = 0;
	gpio_port_pin_t *port_pin = &ctx.supported_pins[gpio_number].gpio;

	struct gpio_callback *clbk = &ctx.supported_pins[gpio_number].callback;
	bool is_initialized = ((0 != clbk->pin_mask) ? true : false);

	WRITE_BIT(clbk->pin_mask, port_pin->pin,
		  (GPIO_INT_DISABLE != (irq_flags & GPIO_INT_DISABLE)));

	if (!is_initialized) {
		if (GPIO_INT_DISABLE != irq_flags) {
			gpio_init_callback(clbk, sid_gpio_irq_callback, clbk->pin_mask);
			erc = gpio_add_callback(port_pin->port, clbk);
			if (erc) {
				WRITE_BIT(clbk->pin_mask, port_pin->pin, false);
				return erc;
			}
			ctx.supported_pins[gpio_number].configuration_cache &=
				~(GPIO_INPUT | GPIO_OUTPUT);
			ctx.supported_pins[gpio_number].configuration_cache |= GPIO_INPUT;
			gpio_pin_configure(port_pin->port, port_pin->pin,
					   ctx.supported_pins[gpio_number].configuration_cache &
						   ~GPIO_INT_MASK);
		}
	} else {
		if (0 == clbk->pin_mask) {
			erc = gpio_remove_callback(port_pin->port, clbk);
			if (erc) {
				return erc;
			}
		}
	}
	ctx.supported_pins[gpio_number].configuration_cache &= ~GPIO_INT_MASK;
	ctx.supported_pins[gpio_number].configuration_cache |= (irq_flags & GPIO_INT_MASK);

	erc = gpio_pin_interrupt_configure(port_pin->port, port_pin->pin,
					   ctx.supported_pins[gpio_number].configuration_cache &
						   GPIO_INT_MASK);
	return erc;
}

int sid_gpio_utils_irq_set(uint32_t gpio_number, bool set)
{
	CHECK_IF_GPIO_IS_REGISTERED(gpio_number)

	gpio_port_pin_t *port_pin = &ctx.supported_pins[gpio_number].gpio;

	gpio_flags_t flags = GPIO_INT_DISABLE;
	if (set) {
		flags = ctx.supported_pins[gpio_number].configuration_cache & GPIO_INT_MASK;
	}

	int erc = gpio_pin_interrupt_configure(port_pin->port, port_pin->pin, flags);

	return erc;
}
