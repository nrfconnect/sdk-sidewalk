/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_error.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <sid_thread.h>

#include <sid_api.h>

#include <app_ble_config.h>
#if defined(CONFIG_SIDEWALK_SUBGHZ)
#include <app_subGHz_config.h>
#endif

LOG_MODULE_REGISTER(sid_thread, CONFIG_SID_THREAD_LOG_LEVEL);

// ////////////////////////////////////////////////////////////////////////////

#define SIDEWALK_DUT_WORK_Q_STACK_SIZE KB(10)

K_THREAD_STACK_DEFINE(sidewalk_dut_work_q_stack, SIDEWALK_DUT_WORK_Q_STACK_SIZE);
static struct k_work_q sidewalk_dut_work_q;

struct app_context {
	struct sid_handle *sidewalk_handle;
};

static struct app_context g_app_context;

// ////////////////////////////////////////////////////////////////////////////

static struct k_work sidewalk_event_work;
static void sidewalk_event_worker(struct k_work *work)
{
	// struct app_context *app_ctx = CONTAINER_OF(work, struct app_context, sidewalk_event_work);

	sid_error_t e = sid_process(g_app_context.sidewalk_handle);

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sid process failed with error %d", e);
	}
}

// ////////////////////////////////////////////////////////////////////////////
static void on_sidewalk_event(bool in_isr, void *context)
{
	ARG_UNUSED(in_isr);
	ARG_UNUSED(context);
	if (sidewalk_event_work.handler == NULL) {
		k_work_init(&sidewalk_event_work, sidewalk_event_worker);
	}
	k_work_submit_to_queue(&sidewalk_dut_work_q, &sidewalk_event_work);
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
	struct app_context *app_context = (struct app_context *)context;

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

static struct sid_event_callbacks event_callbacks = {
	.context = &g_app_context,
	.on_event = on_sidewalk_event,                          /* Called from ISR context */
	.on_msg_received = on_sidewalk_msg_received,            /* Called from sid_process() */
	.on_msg_sent = on_sidewalk_msg_sent,                    /* Called from sid_process() */
	.on_send_error = on_sidewalk_send_error,                /* Called from sid_process() */
	.on_status_changed = on_sidewalk_status_changed,        /* Called from sid_process() */
	.on_factory_reset = on_sidewalk_factory_reset,          /* Called from sid_process */
};

static struct sid_config config;

///////////////////////////////////////////////////////////////////////////////

struct k_work_q * sid_thread_init(void)
{
	k_work_queue_init(&sidewalk_dut_work_q);
	static struct k_work_queue_config cfg = { .name = "sidewalk_thread", .no_yield = false };

	k_work_queue_start(&sidewalk_dut_work_q, sidewalk_dut_work_q_stack,
			   K_THREAD_STACK_SIZEOF(sidewalk_dut_work_q_stack), CONFIG_SIDEWALK_THREAD_PRIORITY,
			   &cfg);
	return &sidewalk_dut_work_q;
}

struct sid_config *get_sidewalk_config()
{
	static bool config_initialized = false;

	if (!config_initialized) {
		config_initialized = true;
		config = (struct sid_config){
			.link_mask = 0,
			.callbacks = &event_callbacks,
			.link_config = app_get_ble_config(),
			.time_sync_periodicity_seconds = 7200,
#if defined(CONFIG_SIDEWALK_SUBGHZ)
			.sub_ghz_link_config = app_get_sub_ghz_config(),
#else
			.sub_ghz_link_config = NULL,
#endif
		};

	}
	return &config;
}

struct sid_handle **get_sidewalk_handle()
{
	return &g_app_context.sidewalk_handle;
}
