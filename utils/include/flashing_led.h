/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t led_pattern_base_t;
typedef led_pattern_base_t led_pattern_t[];

#define LED_PATTERN_LENGTH(pattern) sizeof(pattern) / sizeof(led_pattern_base_t)
#define DEFINE_PATTERN(name, pattern ...) \
	static led_pattern_t name = { pattern };

/**
 * @brief Set state to LED.
 *
 * @param state - desired state of the LED.
 *
 * @note This function may be executed from ISR, keep it short and ISR safe.
 */
typedef void (*led_act)(int state, void *ctx);

typedef int pattern_id;

// TODO RG: move to kconfig
#define MAX_PATTERNS 4

/**
 * @brief Start LED pattern.
 *
 * @param initial_state - start pattern with LED state.
 * @param act - function pointer called to set state of led.
 * @param ctx - pointer passed to act to pass custom data to handler.
 * @param led_pattern - pointer to led pattern.
 * @param pattern_length - length of pattern use @LED_PATTERN_LENGTH to
 * determine the size of pattern.
 * @param loopback - if set to true, the pattern will repeat after end is
 * reached.
 *
 * @return - ID of the pattern that has started or -ENOMEM in case of fail.
 */
pattern_id play_toggle_pattern(bool initial_state, led_act act, void *ctx,
			       led_pattern_t led_pattern, size_t pattern_length,
			       bool loopback);

/**
 * @brief Stop pattern.
 *        If the pattern has already ended, no action will be performed.
 *
 * @param id - id of the state to stop see @play_toggle_pattern.
 *
 * @return 0 on successs, -EINVAL in case of invalid id.
 */
int stop_toggle_pattern(pattern_id id);
