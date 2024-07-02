/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sidewalk.h>
#include <sid_pal_common_ifc.h>
#include <sid_hal_memory_ifc.h>
#include <json_printer/sidTypes2Json.h>

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <app_subGHz_config.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
#include <sbdt/file_transfer.h>
#include <sid_bulk_data_transfer_api.h>
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
#include <sidewalk_dfu/nordic_dfu_img.h>
#include <zephyr/dfu/mcuboot.h>
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
#include <stdio.h> // print hash only
#include <sid_pal_crypto_ifc.h> // print hash only
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER */
#ifdef CONFIG_SID_END_DEVICE_CLI
#include <cli/app_dut.h>
#endif /* CONFIG_SID_END_DEVICE_CLI */
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
#include <settings_utils.h>
#endif /* CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

LOG_MODULE_REGISTER(sidewalk_app, CONFIG_SIDEWALK_LOG_LEVEL);

static struct k_thread sid_thread;
K_THREAD_STACK_DEFINE(sid_thread_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);

static uint8_t __aligned(4)
	sid_msgq_buff[CONFIG_SIDEWALK_THREAD_QUEUE_SIZE * sizeof(sidewalk_ctx_event_t)];

static struct k_msgq msgq;
K_SEM_DEFINE(sid_thread_started, 0, 1);
static void sid_thread_entry(void *context, void *unused, void *unused2)
{
	ARG_UNUSED(unused);
	ARG_UNUSED(unused2);

	sidewalk_ctx_t *sid = (sidewalk_ctx_t *)context;
	sidewalk_ctx_event_t event = {};

	k_msgq_init(&msgq, (char *)sid_msgq_buff, sizeof(sidewalk_ctx_event_t),
		    CONFIG_SIDEWALK_THREAD_QUEUE_SIZE);
	k_sem_give(&sid_thread_started);

	while (1) {
		int err = k_msgq_get(&msgq, &event, K_FOREVER);
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
	sidewalk_ctx_event_t ctx_event = {
		.handler = event,
		.ctx = ctx,
		.ctx_free = free,
	};

	k_timeout_t timeout = K_NO_WAIT;

#ifdef CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT
	if (!k_is_in_isr()) {
		timeout = K_MSEC(CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT_VALUE);
	}
#endif /* CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT */

	const int result = k_msgq_put(&msgq, (void *)&ctx_event, timeout);
	LOG_DBG("sidewalk_event_send event = %p, context = %p, k_msgq_put result %d", (void *)event,
		ctx, result);

	return result;
}
