/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "sid_error.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

#include <sid_thread.h>
#include <sid_api_delegated.h>

#include <sid_api.h>
#include <sid_ble_link_config_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_crypto_ifc.h>
#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
#include <sx126x_config.h>
#endif

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

LOG_MODULE_REGISTER(sid_thread, LOG_LEVEL_INF);

K_THREAD_STACK_DEFINE(sidewalk_dut_work_q_stack, SIDEWALK_DUT_WORK_Q_STACK_SIZE);

static void sidewalk_event_worker(struct k_work *work);
static void on_sidewalk_event(bool in_isr, void *context);
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context);
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context);
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context);
static void on_sidewalk_factory_reset(void *context);
static void on_sidewalk_status_changed(const struct sid_status *status, void *context);

// ////////////////////////////////////////////////////////////////////////////

struct k_work_q sidewalk_dut_work_q;

static struct sid_handle *sid_handle = NULL;

static struct app_context g_app_context = {
	.sidewalk_handle = &sid_handle,
};

struct sid_event_callbacks event_callbacks = {
	.context = &g_app_context,
	.on_event = on_sidewalk_event,                          /* Called from ISR context */
	.on_msg_received = on_sidewalk_msg_received,            /* Called from sid_process() */
	.on_msg_sent = on_sidewalk_msg_sent,                    /* Called from sid_process() */
	.on_send_error = on_sidewalk_send_error,                /* Called from sid_process() */
	.on_status_changed = on_sidewalk_status_changed,        /* Called from sid_process() */
	.on_factory_reset = on_sidewalk_factory_reset,          /* Called from sid_process */
};

static const sid_ble_link_config_t ble_link_config = {
	.create_ble_adapter = sid_pal_ble_adapter_create,
	.config = NULL,
};

struct sid_config config = {
	.callbacks = &event_callbacks,
	.link_config = &ble_link_config,
};

// ////////////////////////////////////////////////////////////////////////////

static void sidewalk_event_worker(struct k_work *work)
{
	struct app_context *app_ctx = CONTAINER_OF(work, struct app_context, sidewalk_event_work);

	sid_error_t e = sid_process(*app_ctx->sidewalk_handle);

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sid process failed with error %d", e);
	}
}

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_DBG("sidewalk event");
	struct app_context *ctx = (struct app_context *)context;

	if (ctx->sidewalk_event_work.handler == NULL) {
		k_work_init(&ctx->sidewalk_event_work, sidewalk_event_worker);
	}
	k_work_submit_to_queue(&sidewalk_dut_work_q, &ctx->sidewalk_event_work);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_INF("Message received");
	LOG_HEXDUMP_INF(msg->data, msg->size, "");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("send message succeded");
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("send message failed with error %d", error);
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_INF("factory reset handled");
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
		if (*app_context->sidewalk_handle) {
			LOG_ERR("sidewalk error: %d", (int)sid_get_error(*app_context->sidewalk_handle));
		} else {
			LOG_ERR("sidewalk error: handle NULL");
		}
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		LOG_INF("Sidewalk status changed to SID_STATE_SECURE_CHANNEL_READY");
		break;
	}
}

// ////////////////////////////////////////////////////////////////////////////
#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
static void initialize_radio_busy_gpio(void)
{
	LOG_DBG("Init Semtech busy pin");
	if (SID_ERROR_NONE !=
	    sid_pal_gpio_set_direction(radio_sx1262_cfg.gpio_radio_busy, SID_PAL_GPIO_DIRECTION_INPUT)) {
		LOG_ERR("sid_pal_gpio_set_direction failed");
		return;
	}
	if (SID_ERROR_NONE != sid_pal_gpio_input_mode(radio_sx1262_cfg.gpio_radio_busy, SID_PAL_GPIO_INPUT_CONNECT)) {
		LOG_ERR("sid_pal_gpio_input_mode failed");
		return;
	}
	if (SID_ERROR_NONE != sid_pal_gpio_pull_mode(radio_sx1262_cfg.gpio_radio_busy, SID_PAL_GPIO_PULL_NONE)) {
		LOG_ERR("sid_pal_gpio_pull_mode failed");
		return;
	}
}

#endif

static sid_error_t sid_pal_init(void)
{
	sid_error_t ret_code;

	ret_code = sid_pal_storage_kv_init();
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("Sidewalk KV store init failed, err: %d", ret_code);
		return ret_code;
	}

	ret_code = sid_pal_crypto_init();
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("Sidewalk Init Crypto HAL, err: %d", ret_code);
		return ret_code;
	}

	static const sid_pal_mfg_store_region_t mfg_store_region = {
		.addr_start = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage)),
		.addr_end = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage) + FLASH_AREA_SIZE(mfg_storage)),
	};

	sid_pal_mfg_store_init(mfg_store_region);

#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	initialize_radio_busy_gpio();
	set_radio_sx126x_device_config(&radio_sx1262_cfg);
#endif /* defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */

	return SID_ERROR_NONE;
}

sid_error_t sid_thread_init(void)
{
	k_work_queue_init(&sidewalk_dut_work_q);
	k_work_queue_start(&sidewalk_dut_work_q, sidewalk_dut_work_q_stack,
			   K_THREAD_STACK_SIZEOF(sidewalk_dut_work_q_stack), SIDEWALK_DUT_WORK_Q_PRIORITY,
			   NULL);
	sid_api_delegated(&sidewalk_dut_work_q);
	return sid_pal_init();
}

struct sid_config *sid_thread_get_config()
{
	return &config;
}

struct app_context *sid_thread_get_app_context()
{
	return &g_app_context;
}
