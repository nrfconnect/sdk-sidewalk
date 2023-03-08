/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_api.h>

#include <sid_callbacks.h>
#include <sid_thread.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_callbacks, CONFIG_SID_THREAD_LOG_LEVEL);
// ////////////////////////////////////////////////////////////////////////////

static void sidewalk_event_worker(struct k_work *work)
{
	sid_thread_ctx_t *ctx = CONTAINER_OF(work, sid_thread_ctx_t, sidewalk_event_work);

	sid_error_t e = sid_process(ctx->sidewalk_handle);

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sid process failed with error %d", e);
	}
}

// ////////////////////////////////////////////////////////////////////////////
static void on_sidewalk_event(bool in_isr, void *context)
{
	ARG_UNUSED(in_isr);
	sid_thread_ctx_t *ctx = (sid_thread_ctx_t *)context;

	if (ctx->sidewalk_event_work.handler == NULL) {
		k_work_init(&ctx->sidewalk_event_work, sidewalk_event_worker);
	}
	k_work_submit_to_queue(&ctx->sidewalk_work_q, &ctx->sidewalk_event_work);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_DBG("Sidewalk -> App");
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("Sidewalk -> App");
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("Sidewalk -> App: error %d", error);
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_DBG("Sidewalk -> App");
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	sid_thread_ctx_t *app_context = (sid_thread_ctx_t *)context;

	switch (status->state) {
	case SID_STATE_READY:
		LOG_INF("Sidewalk status changed to SID_STATE_READY");
		break;
	case SID_STATE_NOT_READY:
		LOG_INF("Sidewalk status changed to SID_STATE_NOT_READY");
		break;
	case SID_STATE_ERROR:
		if (app_context->sidewalk_handle) {
			LOG_ERR("sidewalk error: %d", (int)sid_get_error(app_context->sidewalk_handle));
		} else {
			LOG_ERR("sidewalk error: handle NULL");
		}
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		LOG_INF("Sidewalk status changed to SID_STATE_SECURE_CHANNEL_READY");
		break;
	}

	LOG_INF("EVENT SID STATUS: State: %d, Reg: %d, Time: %d, Link_Mask: %x",
		status->state,
		status->detail.registration_status,
		status->detail.time_sync_status,
		status->detail.link_status_mask);
	LOG_INF("EVENT SID STATUS LINK MODE: LORA: %x, FSK: %x, BLE: %x",
		status->detail.supported_link_modes[2],
		status->detail.supported_link_modes[1],
		status->detail.supported_link_modes[0]);
}

// ////////////////////////////////////////////////////////////////////////////

int sid_callbacks_set(void *ctx, struct sid_event_callbacks *cb)
{
	if (!cb || !ctx) {
		return -1;
	}

	cb->context = ctx;
	cb->on_event = on_sidewalk_event;                               /* Called from ISR context */
	cb->on_msg_received = on_sidewalk_msg_received;                 /* Called from sid_process() */
	cb->on_msg_sent = on_sidewalk_msg_sent;                         /* Called from sid_process() */
	cb->on_send_error = on_sidewalk_send_error;                     /* Called from sid_process() */
	cb->on_status_changed = on_sidewalk_status_changed;             /* Called from sid_process() */
	cb->on_factory_reset = on_sidewalk_factory_reset;               /* Called from sid_process() */

	return 0;
}
