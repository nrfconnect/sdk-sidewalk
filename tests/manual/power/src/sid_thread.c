/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <zephyr/kernel.h>

#include <sid_api.h>
#include <sid_thread.h>

#include <app_ble_config.h>
#include <sid_callbacks.h>

#ifdef CONFIG_STATE_NOTIFIER
#include <state_notifier.h>
#endif

#ifdef CONFIG_SIDEWALK_CLI
#include <sid_shell.h>
#endif

K_THREAD_STACK_DEFINE(sidewalk_work_q_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);
// ////////////////////////////////////////////////////////////////////////////

static sid_thread_ctx_t g_sid_thread_ctx;
static struct sid_event_callbacks event_callbacks;

///////////////////////////////////////////////////////////////////////////////

struct k_work_q *sid_thread_init(void)
{
	#ifdef CONFIG_STATE_NOTIFIER
	#ifdef CONFIG_GPIO
	state_watch_init_gpio(&global_state_notifier);
	#endif
	#ifdef CONFIG_LOG
	state_watch_init_log(&global_state_notifier);
	#endif
	#endif

	k_work_queue_init(&g_sid_thread_ctx.sidewalk_work_q);
	static struct k_work_queue_config cfg = { .name = "sidewalk_thread", .no_yield = false };

	k_work_queue_start(&g_sid_thread_ctx.sidewalk_work_q, sidewalk_work_q_stack,
			   K_THREAD_STACK_SIZEOF(sidewalk_work_q_stack), CONFIG_SIDEWALK_THREAD_PRIORITY,
			   &cfg);

	#ifdef CONFIG_SIDEWALK_CLI
	CLI_init(&g_sid_thread_ctx.sidewalk_work_q, g_sid_thread_ctx.sidewalk_handle);
	#endif
	return &g_sid_thread_ctx.sidewalk_work_q;
}

struct sid_config *get_sidewalk_config()
{
	static bool config_initialized = false;

	if (!config_initialized) {
		if (sid_callbacks_set(&g_sid_thread_ctx, &event_callbacks)) {
			return NULL;
		}
		g_sid_thread_ctx.sidewalk_config = (struct sid_config){
			.link_mask = 0,
			.callbacks = &event_callbacks,
			.link_config = app_get_ble_config(),
			.time_sync_periodicity_seconds = 7200,
			.sub_ghz_link_config = NULL,
		};
		config_initialized = true;
	}
	return &g_sid_thread_ctx.sidewalk_config;
}

struct sid_handle **get_sidewalk_handle()
{
	return &g_sid_thread_ctx.sidewalk_handle;
}
