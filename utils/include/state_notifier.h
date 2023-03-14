/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef STATE_NOTIFIER_H
#define STATE_NOTIFIER_H

#include <stdint.h>
#include <stdbool.h>
#include <application_states.h>
#include <autoconf.h> /* for kconfig options */

#define APPLICATION_STATE_ENUM(name) APPLICATION_STATE_##name

/**
 * @brief Enumeration of application states
 */
#define X(name, ...) APPLICATION_STATE_ENUM(name),
enum application_state {
	X_APPLICAITON_STATES
};
#undef X

/**
 * @brief Structure for holding values of individual states
 */
#define X(name, ...) uint32_t name : 1;
struct notifier_state {
	X_APPLICAITON_STATES
};
#undef X

/**
 * @brief Array holding the string representations of the state names
 */
extern const char *application_state_name[];

/**
 * @brief Handler for the enumeration
 *
 * @param state_id enum representation of the state
 * @param value The value of the state
 * @param boolean signal if the value is a boolean
 */
typedef void (*enumerate_function)(const enum application_state state_id, const uint32_t value);

/**
 * @brief Compare the two application states, and call action on new state that is different than previous state.
 *
 * @param s1 previous state
 * @param s2 new state
 * @param action enumerate handler
 */
void enumerate_differences(const struct notifier_state *s1, const struct notifier_state *s2,
			   const enumerate_function action);

/**
 * @brief Call action on every state
 *
 * @param state The state of the application on which to call action
 * @param action enumerate handler
 */
void enumerate_states(const struct notifier_state *state, const enumerate_function action);

/**
 * @brief Handler for change state
 *
 * @param new_state the new state of the application
 * @param chaged enum representing what state triggered the state_change_handler to be called
 */
typedef void (*state_change_handler)(const struct notifier_state *new_state);

/**
 * @brief Notifier context stores the current state, and handles the registered listeners
 */
struct notifier_ctx {
	struct notifier_state app_state;
	state_change_handler handler[CONFIG_STATE_NOTIFIER_HANDLER_MAX];
};

#define X(name, ...) void application_state_##name(struct notifier_ctx *ctx, const uint32_t value);
X_APPLICAITON_STATES
#undef X

/**
 * @brief Register handler for the state change
 *
 * @param ctx notifier context where to register
 * @param handler function to call on state change
 * @return true on success
 * @return false on error
 */
bool subscribe_for_state_change(struct notifier_ctx *ctx, state_change_handler handler);

extern struct notifier_ctx global_state_notifier;
#endif
