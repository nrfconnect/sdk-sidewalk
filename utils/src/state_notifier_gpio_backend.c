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

static inline struct gpio_dt_spec state_to_pin_mapper(enum application_state state)
{
	// clang-format off
	switch (state) {
	#define X(name, ...)\
		case APPLICATION_STATE_ENUM(name):\
			return (struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(DT_ALIAS(state_notifier_##name), gpios, { 0 });
		X_APPLICAITON_STATES
	#undef X
	}
	// clang-format on
	return (struct gpio_dt_spec){ 0 };
}

static void gpio_enumerate_state(enum application_state state_id, uint32_t value)
{
	struct gpio_dt_spec gpio = state_to_pin_mapper(state_id);
	if (gpio.port == NULL) {
		return;
	}

#if defined(NRF54L15_ENGA_XXAA)
	gpio_pin_set_raw(gpio.port, gpio.pin, value);
#else
	/* output activated with low state */
	gpio_pin_set_raw(gpio.port, gpio.pin, !value);
#endif
}

static void state_change_handler_gpio(const struct notifier_state *state)
{
	enumerate_states(state, gpio_enumerate_state);
}

static void gpio_initializer(const enum application_state state_id, const uint32_t value)
{
	struct gpio_dt_spec pin = state_to_pin_mapper(state_id);

	if (pin.port == NULL) {
		return;
	}
	gpio_pin_configure(pin.port, pin.pin, GPIO_OUTPUT_HIGH);
}

void state_watch_init_gpio(struct notifier_ctx *ctx)
{
	if (!subscribe_for_state_change(ctx, state_change_handler_gpio)) {
		__ASSERT(
			false,
			"failed to initialize the state watch, is the CONFIG_STATE_NOTIFIER_HANDLER_MAX too low ?");
	}
	enumerate_states(&ctx->app_state, gpio_initializer);
}
