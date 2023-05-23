/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <state_notifier.h>

struct gpio_pin {
	const struct device *port;
	gpio_pin_t pin;
};

static inline struct gpio_pin state_to_pin_mapper(enum application_state state)
{
	// clang-format off
	switch (state) {
	#define X(name, port_obj, pin_num, ...)\
		case APPLICATION_STATE_ENUM(name):\
			return (struct gpio_pin){ .port =DEVICE_DT_GET(DT_NODELABEL(port_obj)), .pin = pin_num};
		X_APPLICAITON_STATES
	#undef X
	}
	// clang-format on
	return (struct gpio_pin){ NULL, 0 };
}

static void gpio_enumerate_state(enum application_state state_id, uint32_t value)
{
	struct gpio_pin gpio_id = state_to_pin_mapper(state_id);

	/* output activated with low state */
	gpio_pin_set_raw(gpio_id.port, gpio_id.pin, !value);
}

static void state_change_handler_gpio(const struct notifier_state *state)
{
	enumerate_states(state, gpio_enumerate_state);
}

static void gpio_initializer(const enum application_state state_id, const uint32_t value)
{
	struct gpio_pin pin = state_to_pin_mapper(state_id);

	if (pin.port == NULL) {
		return;
	}
	gpio_pin_configure(pin.port, pin.pin, GPIO_OUTPUT_HIGH);
}

void state_watch_init_gpio(struct notifier_ctx *ctx)
{
	if (!subscribe_for_state_change(ctx, state_change_handler_gpio)) {
		__ASSERT(false,
			 "failed to initialize the state watch, is the CONFIG_STATE_NOTIFIER_HANDLER_MAX too low ?");
	}
	enumerate_states(&ctx->app_state, gpio_initializer);
}
