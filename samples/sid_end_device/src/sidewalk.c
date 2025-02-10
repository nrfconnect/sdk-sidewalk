/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sid_api.h>
#include <sid_error.h>
#include <sidewalk.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sidewalk_app, CONFIG_SIDEWALK_LOG_LEVEL);

static struct k_thread sid_thread;
K_THREAD_STACK_DEFINE(sid_thread_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);

K_MSGQ_DEFINE(sidewalk_thread_msgq, sizeof(sidewalk_ctx_event_t), CONFIG_SIDEWALK_THREAD_QUEUE_SIZE,
	      4);

K_SEM_DEFINE(sid_thread_started, 0, 1);
static void sid_thread_entry(void *context, void *unused, void *unused2)
{
	ARG_UNUSED(unused);
	ARG_UNUSED(unused2);

	sidewalk_ctx_t *sid = (sidewalk_ctx_t *)context;
	sidewalk_ctx_event_t event = {};

	k_sem_give(&sid_thread_started);

	while (1) {
		int err = k_msgq_get(&sidewalk_thread_msgq, &event, K_FOREVER);
#if CONFIG_SIDEWALK_TRACE_SIDEWALK_QUEUE
		LOG_INF("event handler get = %p, sidewalk workq usage (%d/%d) ( after get )",
			(void *)(event.handler), k_msgq_num_used_get(&sidewalk_thread_msgq),
			CONFIG_SIDEWALK_THREAD_QUEUE_SIZE);
#endif
		if (!err) {
			if (event.handler) {
				event.handler(sid, event.ctx);
			}
			if (event.ctx_free) {
				event.ctx_free(event.ctx);
			}
		} else {
			LOG_ERR("Sidewalk msgq err %d", err);
		}
		sid_error_t _ignore = sid_process(sid->handle);
		ARG_UNUSED(_ignore);
	}

	LOG_ERR("Sidewalk thread ends. You should never see this message.");
}

void sidewalk_start(sidewalk_ctx_t *context)
{
	(void)k_thread_create(&sid_thread, sid_thread_stack,
			      K_THREAD_STACK_SIZEOF(sid_thread_stack), sid_thread_entry, context,
			      NULL, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	(void)k_thread_name_set(&sid_thread, "sidewalk");
	k_sem_take(&sid_thread_started, K_FOREVER);
}

int sidewalk_event_send(event_handler_t event, void *ctx, ctx_free free)
{
	if (event == sidewalk_event_process) {
		return 0;
	}
	sidewalk_ctx_event_t ctx_event = {
		.handler = event,
		.ctx = ctx,
		.ctx_free = free,
	};

	k_timeout_t timeout = K_NO_WAIT;
	int result = -EFAULT;

#if defined(CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT_VALUE) &&                                         \
	CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT_VALUE > 0
	if (!k_is_in_isr()) {
		timeout = K_MSEC(CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT_VALUE);
	}
#endif /* CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT_VALUE > 0 */
	result = k_msgq_put(&sidewalk_thread_msgq, (void *)&ctx_event, timeout);
#if CONFIG_SIDEWALK_TRACE_SIDEWALK_QUEUE
	LOG_INF("event handler send = %p, sidewalk workq usage (%d/%d) (after put)",
		(event_handler_t *)event, k_msgq_num_used_get(&sidewalk_thread_msgq),
		CONFIG_SIDEWALK_THREAD_QUEUE_SIZE);
#endif
	LOG_DBG("sidewalk_event_send event = %p, context = %p, k_msgq_put result %d", (void *)event,
		ctx, result);

	return result;
}
