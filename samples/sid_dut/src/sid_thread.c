/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

#include <sid_api.h>
#include <sid_error.h>
#include <sid_pal_common_ifc.h>
#include <sid_thread.h>
#include <sid_api_delegated.h>
#include <sidTypes2Json.h>
#include <json_printer.h>

#include <app_mfg_config.h>
#include <app_ble_config.h>
#if defined(CONFIG_SIDEWALK_SUBGHZ)
#include <app_subGHz_config.h>
#endif

LOG_MODULE_REGISTER(sid_thread, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(sidewalk_dut_work_q_stack, SIDEWALK_DUT_WORK_Q_STACK_SIZE);

static void sidewalk_event_worker(struct k_work *work);
static void on_sidewalk_event(bool in_isr, void *context);
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context);
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context);
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context);
static void on_sidewalk_factory_reset(void *context);
static void on_sidewalk_status_changed(const struct sid_status *status, void *context);
static void on_control_event_notify(const struct sid_control_event_data *data, void *context);

// ////////////////////////////////////////////////////////////////////////////

static struct k_work_q sidewalk_dut_work_q;

static struct sid_handle *sid_handle = NULL;

static struct app_context g_app_context = {
	.sidewalk_handle = &sid_handle,
};

static struct sid_event_callbacks event_callbacks = {
	.context = &g_app_context,
	.on_event = on_sidewalk_event, /* Called from ISR context */
	.on_msg_received = on_sidewalk_msg_received, /* Called from sid_process() */
	.on_msg_sent = on_sidewalk_msg_sent, /* Called from sid_process() */
	.on_send_error = on_sidewalk_send_error, /* Called from sid_process() */
	.on_status_changed = on_sidewalk_status_changed, /* Called from sid_process() */
	.on_factory_reset = on_sidewalk_factory_reset, /* Called from sid_process */
	.on_control_event_notify = on_control_event_notify,
};

static struct sid_config config;

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
	struct app_context *ctx = (struct app_context *)context;

	if (ctx->sidewalk_event_work.handler == NULL) {
		k_work_init(&ctx->sidewalk_event_work, sidewalk_event_worker);
	}
	k_work_submit_to_queue(&sidewalk_dut_work_q, &ctx->sidewalk_event_work);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	struct app_context *ctx = (struct app_context *)context;
	shell_fprintf(ctx->shell, SHELL_NORMAL,
		      JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
			      "on_msg_received",
			      JSON_OBJ(JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 1))))));
	LOG_DBG("Sidewalk -> App");
	LOG_HEXDUMP_INF(msg->data, msg->size, "");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	struct app_context *ctx = (struct app_context *)context;
	shell_fprintf(ctx->shell, SHELL_NORMAL,
		      JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
			      "on_msg_sent",
			      JSON_OBJ(JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc, 0))))));
	LOG_DBG("Sidewalk -> App");
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	struct app_context *ctx = (struct app_context *)context;
	shell_fprintf(ctx->shell, SHELL_NORMAL,
		      JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
			      "on_send_error",
			      JSON_OBJ(JSON_LIST_2(JSON_VAL_sid_error_t("error", error),
						   JSON_VAL_sid_msg_desc("sid_msg_desc", msg_desc,
									 0)))))));
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
		if (*app_context->sidewalk_handle) {
			LOG_ERR("sidewalk error: %d",
				(int)sid_get_error(*app_context->sidewalk_handle));
		} else {
			LOG_ERR("sidewalk error: handle NULL");
		}
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		LOG_INF("Sidewalk status changed to SID_STATE_SECURE_CHANNEL_READY");
		break;
	}

	LOG_INF("EVENT SID STATUS: State: %d, Reg: %d, Time: %d, Link_Mask: %x", status->state,
		status->detail.registration_status, status->detail.time_sync_status,
		status->detail.link_status_mask);
	LOG_INF("EVENT SID STATUS LINK MODE: LORA: %x, FSK: %x, BLE: %x",
		status->detail.supported_link_modes[2], status->detail.supported_link_modes[1],
		status->detail.supported_link_modes[0]);
}

static void on_control_event_notify(const struct sid_control_event_data *data, void *context)
{
	struct app_context *app_context = (struct app_context *)context;
	if (data == NULL || data->event_data == NULL) {
		shell_fprintf(app_context->shell, SHELL_NORMAL,
			      "on_control_event_notify called with invalid data");
		return;
	}
	shell_fprintf(app_context->shell, SHELL_NORMAL,
		      JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
			      "on_control_event_notify",
			      JSON_OBJ(JSON_VAL_sid_control_event_data_t("data", data))))));
}

sid_error_t sid_thread_init(void)
{
	k_work_queue_init(&sidewalk_dut_work_q);
	static struct k_work_queue_config cfg = { .name = "sidewalk_thread", .no_yield = false };

	k_work_queue_start(&sidewalk_dut_work_q, sidewalk_dut_work_q_stack,
			   K_THREAD_STACK_SIZEOF(sidewalk_dut_work_q_stack),
			   CONFIG_SIDEWALK_THREAD_PRIORITY, &cfg);
	sid_api_delegated(&sidewalk_dut_work_q);

	const struct sid_sub_ghz_links_config *sub_ghz_lc = NULL;

#if defined(CONFIG_SIDEWALK_SUBGHZ)
	sub_ghz_lc = app_get_sub_ghz_config();
#endif

	config = (struct sid_config){
		.link_mask = 0,
		.callbacks = &event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = sub_ghz_lc,
	};

	platform_parameters_t platform_parameters = {
		.mfg_store_region.addr_start = APP_MFG_CFG_FLASH_START,
		.mfg_store_region.addr_end = APP_MFG_CFG_FLASH_END,
		.platform_init_parameters.radio_cfg =
			(radio_sx126x_device_config_t *)get_radio_cfg(),
	};

	sid_error_t ret_code = sid_platform_init(&platform_parameters);
	if (ret_code != SID_ERROR_NONE) {
		LOG_ERR("Sidewalk Platform Init err: %d", ret_code);
		return SID_ERROR_GENERIC;
	}

	if (app_mfg_cfg_is_valid()) {
		LOG_ERR("The mfg.hex version mismatch");
		LOG_ERR("Check if the file has been generated and flashed properly");
		LOG_ERR("START ADDRESS: 0x%08x", APP_MFG_CFG_FLASH_START);
		LOG_ERR("SIZE: 0x%08x", APP_MFG_CFG_FLASH_SIZE);
		return SID_ERROR_NOT_FOUND;
	}

	return SID_ERROR_NONE;
}

struct sid_config *sid_thread_get_config()
{
	return &config;
}

struct app_context *sid_thread_get_app_context()
{
	return &g_app_context;
}
