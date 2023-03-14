/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <state_notifier.h>

#define X(name, ...) [APPLICATION_STATE_ENUM(name)] = #name,
const char *application_state_name[] = { X_APPLICAITON_STATES };
#undef X

#define X(name, ...) if (s1->name != s2->name) { action(APPLICATION_STATE_ENUM(name), s2->name); }
void enumerate_differences(const struct notifier_state *s1, const struct notifier_state *s2,
			   const enumerate_function action)
{
	X_APPLICAITON_STATES
}

#undef X

#define X(name, ...) action(APPLICATION_STATE_ENUM(name), state->name);
void enumerate_states(const struct notifier_state *state, const enumerate_function action)
{
	X_APPLICAITON_STATES
}

#undef X

static void notify_all(const struct notifier_ctx *ctx)
{
	for (int i = 0; i < CONFIG_STATE_NOTIFIER_HANDLER_MAX; i++) {
		if (ctx->handler[i]) {
			ctx->handler[i](&ctx->app_state);
		}
	}
}

#define X(name, ...) void application_state_##name(struct notifier_ctx *ctx, const uint32_t value) \
	{ ctx->app_state.name = value; notify_all(ctx); }
X_APPLICAITON_STATES
#undef X

typedef void (*change_state_handler)(struct notifier_ctx *ctx, uint32_t value);

bool subscribe_for_state_change(struct notifier_ctx *ctx, state_change_handler handler)
{
	for (int i = 0; i < CONFIG_STATE_NOTIFIER_HANDLER_MAX; i++) {
		if (ctx->handler[i] == NULL) {
			ctx->handler[i] = handler;
			return true;
		}
	}
	return false;
}
struct notifier_ctx global_state_notifier;
