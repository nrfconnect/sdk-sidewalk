/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sid_api.h>
#include <sid_error.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <board_events.h>
#include <pal_init.h>
#if defined(CONFIG_SIDEWALK_CLI)
#include <sid_shell.h>
#endif

#include <application_thread.h>
#include <state_notifier.h>

static struct k_thread application_thread;

K_THREAD_STACK_DEFINE(application_thread_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);
K_MSGQ_DEFINE(application_thread_msgq, sizeof(app_event_t), CONFIG_SIDEWALK_THREAD_QUEUE_SIZE, 4);

LOG_MODULE_REGISTER(application, CONFIG_SIDEWALK_LOG_LEVEL);

static void sidewalk_app_entry(void *ctx, void *unused, void *unused2)
{
	ARG_UNUSED(unused);
	ARG_UNUSED(unused2);
	app_ctx_t *application_ctx = (app_ctx_t *)ctx;

	if (application_pal_init()) {
		LOG_ERR("Failed to initialze PAL layer for sidewalk applicaiton.");
		application_state_error(&global_state_notifier, true);
		return;
	}

	sid_error_t err = sid_init(&application_ctx->config, &application_ctx->handle);

	switch (err) {
	case SID_ERROR_NONE:
		break;
	case SID_ERROR_ALREADY_INITIALIZED:
		LOG_WRN("Sidewalk already initialized!");
		break;
	default:
		LOG_ERR("Unknown error (%d) during sidewalk initialization!", err);
		application_state_error(&global_state_notifier, true);
		return;
	}

	err = sid_start(application_ctx->handle, BUILT_IN_LM);
	if (err) {
		LOG_ERR("Unknown error (%d) during sidewalk start!", err);
		application_state_error(&global_state_notifier, true);
		return;
	}
#if defined(CONFIG_SIDEWALK_CLI)
	CLI_init(application_ctx->handle);
#endif
	application_state_connected(&global_state_notifier, false);
	application_state_working(&global_state_notifier, true);
	while (true) {
		app_event_t event = SIDEWALK_EVENT;

		if (!k_msgq_get(&application_thread_msgq, &event, K_FOREVER)) {
			switch (event) {
			case SIDEWALK_EVENT:
				err = sid_process(application_ctx->handle);
				if (err) {
					LOG_WRN("sid_process returned %d", err);
				}
				break;
			case BUTTON_EVENT_SEND_HELLO:
				button_event_send_hello(application_ctx);
				break;
			case BUTTON_EVENT_SET_BATTERY_LEVEL:
				button_event_set_battery(application_ctx);
				break;
			case BUTTON_EVENT_FACTORY_RESET:
				button_event_factory_reset(application_ctx);
				break;
			case BUTTON_EVENT_CONNECTION_REQUEST:
				button_event_connection_request(application_ctx);
				break;

#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
			case BUTTON_EVENT_NORDIC_DFU:
				button_event_DFU(application_ctx);
				break;
#endif

			default:
				LOG_ERR("Invalid Event received!");
			}
		}
	}
	application_state_working(&global_state_notifier, false);
}

void app_event_send(app_event_t event)
{
	int ret = k_msgq_put(&application_thread_msgq, (void *)&event,
			     k_is_in_isr() ? K_NO_WAIT : K_FOREVER);

	if (ret) {
		LOG_ERR("Failed to send event to application thread. err: %d", ret);
	}
}

sid_error_t app_thread_init(app_ctx_t *ctx)
{
	if (!ctx) {
		return SID_ERROR_NULL_POINTER;
	}
	(void)k_thread_create(&application_thread, application_thread_stack,
			      K_THREAD_STACK_SIZEOF(application_thread_stack), sidewalk_app_entry,
			      ctx, NULL, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&application_thread, "sidewalk_thread");
	return SID_ERROR_NONE;
}
