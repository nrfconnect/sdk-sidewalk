/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zephyr/kernel.h>

#include <flashing_led.h>

struct pattern_state {
	bool pattern_currently_active : 1;
	bool current_LED_state : 1;
	bool loopback : 1;
	void *ctx;
	led_pattern_base_t *pattern;
	size_t pattern_size;
	size_t current_step;
	led_act act;
	struct k_timer pattern_timer;
};

static struct pattern_state active_pattern_repository[MAX_PATTERNS];

static void led_timer_handler(struct k_timer *timer_id)
{
	struct pattern_state *state = CONTAINER_OF(timer_id, struct pattern_state, pattern_timer);

	if (!state->pattern_currently_active) {
		return;
	}
	state->current_LED_state = !state->current_LED_state;
	state->act(state->current_LED_state, state->ctx);

	state->current_step++;
	if (state->current_step >= state->pattern_size) {
		if (state->loopback) {
			state->current_step = 0;
		} else {
			state->pattern_currently_active = false;
			return;
		}
	}
	k_timer_start(&state->pattern_timer, K_MSEC(state->pattern[state->current_step]), K_NO_WAIT);
}

pattern_id play_toggle_pattern(bool initial_state, led_act act, void *ctx, led_pattern_t led_pattern,
			       size_t pattern_length,
			       bool loopback)
{
	if (pattern_length == 0) {
		return -EINVAL;
	}
	for (size_t i = 0; i < MAX_PATTERNS; i++) {
		if (active_pattern_repository[i].pattern_currently_active == false) {
			active_pattern_repository[i] =
				(struct pattern_state){ .pattern_currently_active = true,
							.current_LED_state = initial_state,
							.current_step = 0, .pattern = led_pattern,
							.pattern_size = pattern_length,
							.loopback = loopback,
							.act = act,
							.ctx = ctx };
			act(initial_state, ctx);
			k_timer_init(&active_pattern_repository[i].pattern_timer, led_timer_handler, NULL);
			k_timer_start(&active_pattern_repository[i].pattern_timer,
				      K_MSEC(
					      active_pattern_repository[i].pattern[active_pattern_repository[i].
										   current_step]),
				      K_NO_WAIT);
			return i;
		}
	}
	return -ENOMEM;
}

int stop_toggle_pattern(pattern_id id)
{
	if (id >= MAX_PATTERNS || id < 0) {
		return -EINVAL;
	}
	if (active_pattern_repository[id].pattern_currently_active) {
		active_pattern_repository[id].pattern_currently_active = false;
		k_timer_stop(&active_pattern_repository[id].pattern_timer);
	}
	return 0;
}
