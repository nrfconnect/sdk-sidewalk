/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_api.h>
#include <sid_error.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_link_config_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <storage/flash_map.h>
#include <zephyr.h>
#include <sys/reboot.h>
#include <dk_buttons_and_leds.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

#define MSG_LOG_BLOCK_SIZE 80

#define SID_INDICATE_CONNECTED DK_LED1

#define IS_RESET_BTN_PRESSED(_btn)      (_btn & DK_BTN1_MSK)
#define IS_CONN_REQ_BTN_PRESSED(_btn)   (_btn & DK_BTN2_MSK)
#define IS_SEND_MSG_BTN_PRESSED(_btn)   (_btn & DK_BTN3_MSK)
#define IS_SET_BAT_LV_BTN_PRESSED(_btn) (_btn & DK_BTN4_MSK)

enum app_state {
	STATE_INIT,
	STATE_SIDEWALK_READY,
	STATE_SIDEWALK_NOT_READY,
	STATE_SIDEWALK_SECURE_CONNECTION,
};

typedef struct app_context {
	struct sid_handle *sidewalk_handle;
	enum app_state state;
	uint8_t counter;
	bool connection_request;
} app_context_t;

static K_SEM_DEFINE(sid_sem, 0, 8);

static app_context_t sid_app_ctx;

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_INF("In func: %s", __func__);
	LOG_INF("From %s, context %p", in_isr ? "ISR" : "App", context);
	k_sem_give(&sid_sem);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_INF("In func: %s", __func__);
	LOG_INF("received message(type: %d, id: %u size %u)", (int)msg_desc->type, msg_desc->id, msg->size);

	for (size_t i = 0; i < msg->size; i += MSG_LOG_BLOCK_SIZE) {
		if (i + MSG_LOG_BLOCK_SIZE > msg->size) {
			LOG_HEXDUMP_INF((uint8_t *)msg->data + i, msg->size - i, "");
		} else {
			LOG_HEXDUMP_INF((uint8_t *)msg->data + i, MSG_LOG_BLOCK_SIZE, "");
		}
	}
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("In func: %s", __func__);
	LOG_INF("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("In func: %s", __func__);
	LOG_ERR("failed to send message(type: %d, id: %u), err:%d", (int)msg_desc->type, msg_desc->id, (int)error);
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	LOG_INF("In func: %s", __func__);
	LOG_INF("status changed: %d", (int)status->state);

	app_context_t *app_context = (app_context_t *)context;

	switch (status->state) {
	case SID_STATE_READY:
		dk_set_led_on(SID_INDICATE_CONNECTED);
		app_context->state = STATE_SIDEWALK_READY;
		app_context->connection_request = false;
		break;
	case SID_STATE_NOT_READY:
		dk_set_led_off(SID_INDICATE_CONNECTED);
		app_context->state = STATE_SIDEWALK_NOT_READY;
		break;
	case SID_STATE_ERROR:
		LOG_ERR("sidewalk error: %d", (int)sid_get_error(app_context->sidewalk_handle));
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		app_context->state = STATE_SIDEWALK_SECURE_CONNECTION;
		break;
	}

	LOG_INF("Registration Status = %d, Time Sync Status = %d and Link Status Mask = %x",
		status->detail.registration_status, status->detail.time_sync_status,
		status->detail.link_status_mask);
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_INF("In func: %s", __func__);
	LOG_INF("factory reset notification received from sid api");
	sys_reboot(SYS_REBOOT_WARM);
}

static void connection_request(void)
{
	if (STATE_SIDEWALK_READY == sid_app_ctx.state) {
		LOG_WRN("Sidewalk ready, operation not valid");
	} else {
		bool next = !sid_app_ctx.connection_request;
		LOG_INF("%s connection request", next ? "Set" : "Clear");
		sid_error_t ret = sid_ble_bcn_connection_request(sid_app_ctx.sidewalk_handle, next);
		if (SID_ERROR_NONE == ret) {
			sid_app_ctx.connection_request = next;
		} else {
			LOG_ERR("Connection request failed %d", ret);
		}
	}
}

static void factory_reset(void)
{
	sid_error_t ret = sid_set_factory_reset(sid_app_ctx.sidewalk_handle);

	if (ret != SID_ERROR_NONE) {
		LOG_ERR("Notification of factory reset to sid api failed!");
	} else {
		LOG_INF("Wait for Sid api to notify to proceed with factory reset!");
	}
}

static void send_message(void)
{
	if (sid_app_ctx.state == STATE_SIDEWALK_READY ||
	    sid_app_ctx.state == STATE_SIDEWALK_SECURE_CONNECTION) {
		LOG_INF("sending counter update: %d", sid_app_ctx.counter);
		struct sid_msg msg = { .data = (uint8_t *)&sid_app_ctx.counter, .size = sizeof(uint8_t) };
		struct sid_msg_desc desc = {
			.type = SID_MSG_TYPE_NOTIFY,
			.link_type = SID_LINK_TYPE_ANY,
		};
		sid_error_t ret = sid_put_msg(sid_app_ctx.sidewalk_handle, &msg, &desc);
		if (ret != SID_ERROR_NONE) {
			LOG_ERR("failed queueing data, err:%d", (int) ret);
		} else {
			LOG_INF("queued data message id:%u", desc.id);
		}
		sid_app_ctx.counter++;
	} else {
		LOG_ERR("sidewalk is not ready yet!");
	}
}

static void set_battery_level(void)
{
	static uint8_t fake_bat_lev = 70;

	++fake_bat_lev;
	if (fake_bat_lev > 100) {
		fake_bat_lev = 0;
	}
	sid_error_t ret = sid_option(sid_app_ctx.sidewalk_handle, SID_OPTION_BLE_BATTERY_LEVEL,
				     &fake_bat_lev, sizeof(fake_bat_lev));
	if (SID_ERROR_NONE != ret) {
		LOG_ERR("failed setting sidewalk option!");
	} else {
		LOG_INF("set battery level to %d", fake_bat_lev);
	}
}

static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	uint32_t button = button_state & has_changed;

	if (IS_RESET_BTN_PRESSED(button)) {
		factory_reset();
	}

	if (IS_CONN_REQ_BTN_PRESSED(button)) {
		connection_request();
	}

	if (IS_SEND_MSG_BTN_PRESSED(button)) {
		send_message();
	}

	if (IS_SET_BAT_LV_BTN_PRESSED(button)) {
		set_battery_level();
	}
}

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

	return SID_ERROR_NONE;
}

static sid_error_t sid_lib_run(void)
{
	static const struct sid_ble_config ble_config;

	static const sid_ble_link_config_t ble_link_config = {
		.create_ble_adapter = sid_pal_ble_adapter_create,
		.config = &ble_config,
	};

	struct sid_event_callbacks event_callbacks = {
		.context = &sid_app_ctx,
		.on_event = on_sidewalk_event,                          /* Called from ISR context */
		.on_msg_received = on_sidewalk_msg_received,            /* Called from sid_process() */
		.on_msg_sent = on_sidewalk_msg_sent,                    /* Called from sid_process() */
		.on_send_error = on_sidewalk_send_error,                /* Called from sid_process() */
		.on_status_changed = on_sidewalk_status_changed,        /* Called from sid_process() */
		.on_factory_reset = on_sidewalk_factory_reset,          /* Called from sid_process() */
	};

	struct sid_config config = {
		.link_mask = SID_LINK_TYPE_1,
		.callbacks = &event_callbacks,
		.link_config = &ble_link_config,
	};

	sid_app_ctx.state = STATE_INIT;
	sid_error_t ret_code = sid_init(&config, &sid_app_ctx.sidewalk_handle);

	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("failed to initialize sidewalk, err: %d", (int)ret_code);
		return ret_code;
	}

	ret_code = sid_start(sid_app_ctx.sidewalk_handle, SID_LINK_TYPE_1);
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("failed to start sidewalk, err: %d", (int)ret_code);
		return ret_code;
	}

	return SID_ERROR_NONE;
}

static int board_init(void)
{
	int err = dk_buttons_init(button_handler);

	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
		return err;
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return err;
	}
	return 0;
}

static void sid_main(void)
{
	if (0 == k_sem_take(&sid_sem, K_MSEC(50))) {
		sid_error_t ret = sid_process(sid_app_ctx.sidewalk_handle);
		LOG_INF("Sidewalk proc, status: %d", ret);
	}
}

void main(void)
{
	LOG_INF("Sidewalk BLE example started!");

	if (0 != board_init()) {
		return;
	}

	if (SID_ERROR_NONE != sid_pal_init()) {
		return;
	}

	if (SID_ERROR_NONE != sid_lib_run()) {
		return;
	}

	for (;;) {
		sid_main();
	}
}
