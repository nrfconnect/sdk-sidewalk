/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <state_notifier.h>
#include <state_notifier_log_backend.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(application_state, LOG_LEVEL_INF);

static void log_enumerate_state(enum application_state state_id, uint32_t value)
{
	LOG_INF("%s = %s", application_state_name[state_id], value?"true":"false");
}

static void state_change_handler_log(const struct notifier_state *state)
{
	static struct notifier_state prev_state = {};

	enumerate_differences(&prev_state, state, log_enumerate_state);
	prev_state = *state;
}

void state_watch_init_log(struct notifier_ctx *ctx)
{
	if (!subscribe_for_state_change(ctx, state_change_handler_log)) {
		__ASSERT(false, "failed to initialize the state watch, is the CONFIG_NOTIFIER_HANDLER_MAX too low ?");
	}
}
