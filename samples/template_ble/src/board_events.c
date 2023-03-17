/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>
#include <sid_error.h>

#include <state_notifier.h>
#include <board_events.h>

#if defined(CONFIG_SIDEWALK_DFU)
#include <nordic_dfu.h>
#endif

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(board_events, CONFIG_SIDEWALK_LOG_LEVEL);

void button_event_send_hello(app_ctx_t *application_ctx)
{
	struct sid_status status = { .state = SID_STATE_NOT_READY };
	sid_error_t err;

	static uint8_t counter = 0;
	static struct sid_msg msg;
	static struct sid_msg_desc desc;

	err = sid_get_status(application_ctx->handle, &status);
	switch (err) {
	case SID_ERROR_NONE: break;
	case SID_ERROR_INVALID_ARGS: LOG_ERR("Sidewalk library is not initialzied!"); return;
	default: LOG_ERR("Unknown error during sid_get_status() -> %d", err);
		return;
	}

	if (status.state != SID_STATE_READY && status.state != SID_STATE_SECURE_CHANNEL_READY) {
		LOG_ERR(
			"Sidewalk Status is invalid!, expected SID_STATE_READY or SID_STATE_SECURE_CHANNEL_READY, got %d",
			status.state);
		return;
	}

	msg = (struct sid_msg){ .data = (uint8_t *)&counter, .size = sizeof(uint8_t) };
	desc = (struct sid_msg_desc){
		.type = SID_MSG_TYPE_NOTIFY,
		.link_type = SID_LINK_TYPE_ANY,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

	err = sid_put_msg(application_ctx->handle, &msg, &desc);
	switch (err) {
	case SID_ERROR_NONE: {
		application_state_sending(&global_state_notifier, true);
		counter++;
		LOG_INF("queued data message id:%d", desc.id);
		break;
	}
	case SID_ERROR_TRY_AGAIN: {
		LOG_ERR("there is no space in the transmit queue, Try again.");
		break;
	}
	default: LOG_ERR("Unknown error returned from sid_put_msg() -> %d", err);
	}
}

void button_event_set_battery(app_ctx_t *application_ctx)
{
	static uint8_t fake_bat_lev = 70;

	++fake_bat_lev;
	if (fake_bat_lev > 100) {
		fake_bat_lev = 0;
	}
	sid_error_t ret = sid_option(application_ctx->handle, SID_OPTION_BLE_BATTERY_LEVEL,
				     &fake_bat_lev, sizeof(fake_bat_lev));

	if (SID_ERROR_NONE != ret) {
		LOG_ERR("failed setting Sidewalk option!");
	} else {
		LOG_DBG("set battery level to %d", fake_bat_lev);
	}
}

#if defined(CONFIG_SIDEWALK_DFU)
void button_event_DFU(app_ctx_t *application_ctx)
{
	sid_error_t err = sid_deinit(application_ctx->handle);

	if (err != SID_ERROR_NONE) {
		LOG_ERR("Failed to deinitialize sidewalk! sid_deinit returned %d", err);
		return;
	}

	application_ctx->handle = NULL;

	int ret = nordic_dfu_ble_start();

	if (ret) {
		LOG_ERR("DFU SMP start error (code %d)", ret);
	}
	application_state_dfu(&global_state_notifier, true);
}

#endif

void button_event_factory_reset(app_ctx_t *application_ctx)
{
	sid_error_t ret = sid_set_factory_reset(application_ctx->handle);

	if (SID_ERROR_NONE != ret) {
		LOG_ERR("Notification of factory reset to sid api failed!");
	} else {
		LOG_DBG("Wait for Sid api to notify to proceed with factory reset!");
	}
}

void button_event_connection_request(app_ctx_t *application_ctx)
{
	struct sid_status status = { .state = SID_STATE_NOT_READY };
	sid_error_t err = sid_get_status(application_ctx->handle, &status);

	switch (err) {
	case SID_ERROR_NONE: break;
	case SID_ERROR_INVALID_ARGS: LOG_ERR("Sidewalk library is not initialzied!"); return;
	default: LOG_ERR("Unknown error during sid_get_status() -> %d", err);
		return;
	}

	if (status.state == SID_STATE_READY) {
		LOG_WRN("Sidewalk ready, operation not valid");
		return;
	}

	bool next = !application_ctx->connection_request;

	LOG_INF("%s connection request", next ? "Set" : "Clear");
	sid_error_t ret = sid_ble_bcn_connection_request(application_ctx->handle, next);

	if (SID_ERROR_NONE == ret) {
		application_ctx->connection_request = next;
	} else {
		LOG_ERR("Connection request failed %d", ret);
	}
}
