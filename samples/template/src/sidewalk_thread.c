/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sidewalk_thread.h>

#include <sid_api.h>
#include <sid_error.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_link_config_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>

#include <zephyr/kernel.h>
#include <sys/reboot.h>
#include <storage/flash_map.h>
#include <dk_buttons_and_leds.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(sid_thread, CONFIG_SIDEWALK_LOG_LEVEL);

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

#define SID_LED_INDICATE_CONNECTED   (DK_LED1)
#define SID_LED_INDICATE_INIT_ERROR  (DK_ALL_LEDS_MSK)

#ifdef CONFIG_SIDEWALK_LINK_MASK_BLE
#define LINK_MASK       (SID_LINK_TYPE_1)
#define LINK_MASK_START (LINK_MASK)
#elif  CONFIG_SIDEWALK_LINK_MASK_FSK
#define LINK_MASK       (SID_LINK_TYPE_2)
#define LINK_MASK_START (SID_LINK_TYPE_ANY)
#elif CONFIG_SIDEWALK_LINK_MASK_LORA
#define LINK_MASK       (SID_LINK_TYPE_3)
#define LINK_MASK_START (SID_LINK_TYPE_ANY)
#else
#error "Not defined Sidewalk link mask!!"
#endif

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

K_MSGQ_DEFINE(sid_msgq, sizeof(enum event_type), CONFIG_SIDEWALK_THREAD_QUEUE_SIZE, 4);
K_THREAD_STACK_DEFINE(sid_stack_area, CONFIG_SIDEWALK_THREAD_STACK_SIZE);
static struct k_thread sid_thread;
static k_tid_t sid_tid;

static uint8_t *status_name[] = {
	"init", "is ready", "not ready", "secure conn"
};

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_DBG("on event, from %s, context %p", in_isr ? "ISR" : "App", context);
	sidewalk_thread_message_q_write(EVENT_TYPE_SIDEWALK);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_DBG("received message(type: %d, id: %u size %u)", (int)msg_desc->type, msg_desc->id, msg->size);
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_ERR("failed to send message(type: %d, id: %u), err:%d", (int)msg_desc->type, msg_desc->id, (int)error);
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	LOG_INF("status changed: %s", status_name[status->state]);

	app_context_t *app_context = (app_context_t *)context;

	switch (status->state) {
	case SID_STATE_READY:
		dk_set_led_on(SID_LED_INDICATE_CONNECTED);
		app_context->state = STATE_SIDEWALK_READY;
		app_context->connection_request = false;
		break;
	case SID_STATE_NOT_READY:
		dk_set_led_off(SID_LED_INDICATE_CONNECTED);
		app_context->state = STATE_SIDEWALK_NOT_READY;
		break;
	case SID_STATE_ERROR:
		LOG_ERR("sidewalk error: %d", (int)sid_get_error(app_context->sidewalk_handle));
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		app_context->state = STATE_SIDEWALK_SECURE_CONNECTION;
		break;
	}

	LOG_DBG("Device %sregistred, Time Sync %s, Link status %s",
		(SID_STATUS_REGISTERED == status->detail.registration_status) ? "Is " : "Un",
		(SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) ? "Success" : "Fail",
		status->detail.link_status_mask ? "Up" : "Down");
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_DBG("factory reset notification received from sid api");
	sys_reboot(SYS_REBOOT_WARM);
}

static void connection_request(app_context_t *app_context)
{
	if (STATE_SIDEWALK_READY == app_context->state) {
		LOG_WRN("Sidewalk ready, operation not valid");
	} else {
		bool next = !app_context->connection_request;
		LOG_INF("%s connection request", next ? "Set" : "Clear");
		sid_error_t ret = sid_ble_bcn_connection_request(app_context->sidewalk_handle, next);
		if (SID_ERROR_NONE == ret) {
			app_context->connection_request = next;
		} else {
			LOG_ERR("Connection request failed %d", ret);
		}
	}
}

static void factory_reset(app_context_t *app_context)
{
	sid_error_t ret = sid_set_factory_reset(app_context->sidewalk_handle);

	if (SID_ERROR_NONE != ret) {
		LOG_ERR("Notification of factory reset to sid api failed!");
	} else {
		LOG_DBG("Wait for Sid api to notify to proceed with factory reset!");
	}
}

static void send_message(app_context_t *app_context)
{
	if (STATE_SIDEWALK_READY == app_context->state ||
	    STATE_SIDEWALK_SECURE_CONNECTION == app_context->state) {
		LOG_INF("sending counter update: %d", app_context->counter);
		struct sid_msg msg = { .data = (uint8_t *)&app_context->counter, .size = sizeof(uint8_t) };
		struct sid_msg_desc desc = {
			.type = SID_MSG_TYPE_NOTIFY,
			.link_type = SID_LINK_TYPE_ANY,
		};
		sid_error_t ret = sid_put_msg(app_context->sidewalk_handle, &msg, &desc);
		if (SID_ERROR_NONE != ret) {
			LOG_ERR("failed queueing data, err:%d", (int) ret);
		} else {
			LOG_DBG("queued data message id:%u", desc.id);
			app_context->counter++;
		}
	} else {
		LOG_ERR("sidewalk is not ready yet!");
	}
}

static void set_battery_level(app_context_t *app_context)
{
	static uint8_t fake_bat_lev = 70;

	++fake_bat_lev;
	if (fake_bat_lev > 100) {
		fake_bat_lev = 0;
	}
	sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_BLE_BATTERY_LEVEL,
				     &fake_bat_lev, sizeof(fake_bat_lev));
	if (SID_ERROR_NONE != ret) {
		LOG_ERR("failed setting sidewalk option!");
	} else {
		LOG_DBG("set battery level to %d", fake_bat_lev);
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

static sid_error_t sid_lib_run(app_context_t *app_context)
{
	static const struct sid_ble_config ble_config;

	static const sid_ble_link_config_t ble_link_config = {
		.create_ble_adapter = sid_pal_ble_adapter_create,
		.config = &ble_config,
	};

	struct sid_event_callbacks event_callbacks = {
		.context = app_context,
		.on_event = on_sidewalk_event,                          /* Called from ISR context */
		.on_msg_received = on_sidewalk_msg_received,            /* Called from sid_process() */
		.on_msg_sent = on_sidewalk_msg_sent,                    /* Called from sid_process() */
		.on_send_error = on_sidewalk_send_error,                /* Called from sid_process() */
		.on_status_changed = on_sidewalk_status_changed,        /* Called from sid_process() */
		.on_factory_reset = on_sidewalk_factory_reset,          /* Called from sid_process() */
	};

	struct sid_config config = {
		.link_mask = LINK_MASK,
		.callbacks = &event_callbacks,
		.link_config = &ble_link_config,
	};

	sid_error_t ret_code = sid_init(&config, &app_context->sidewalk_handle);

	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("failed to initialize sidewalk, err: %d", (int)ret_code);
		dk_set_leds(SID_LED_INDICATE_INIT_ERROR);
		return ret_code;
	}

	ret_code = sid_start(app_context->sidewalk_handle, LINK_MASK_START);
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("failed to start sidewalk, err: %d", (int)ret_code);
		dk_set_leds(SID_LED_INDICATE_INIT_ERROR);
		return ret_code;
	}

	app_context->state = STATE_SIDEWALK_NOT_READY;
	app_context->connection_request = false;
	return SID_ERROR_NONE;
}

static void sidewalk_thread(void *context, void *u2, void *u3)
{
	ARG_UNUSED(u2);
	ARG_UNUSED(u3);

	app_context_t *sid_app_ctx = (app_context_t *)context;

	if (SID_ERROR_NONE != sid_pal_init()) {
		return;
	}

	if (SID_ERROR_NONE != sid_lib_run(sid_app_ctx)) {
		return;
	}

	LOG_INF("Starting sidewalk thread ...");

	while (1) {
		enum event_type event;

		if (!k_msgq_get(&sid_msgq, &event, K_FOREVER)) {
			switch (event) {
			case EVENT_TYPE_SIDEWALK:
			{
				sid_error_t ret = sid_process(sid_app_ctx->sidewalk_handle);
				if (ret) {
					LOG_WRN("Process error (code %d)", ret);
				}
				break;
			}
			case EVENT_TYPE_SEND_HELLO:
			{
				send_message(sid_app_ctx);
				break;
			}
			case EVENT_TYPE_SET_BATTERY_LEVEL:
			{
				set_battery_level(sid_app_ctx);
				break;
			}
			case EVENT_TYPE_FACTORY_RESET:
			{
				factory_reset(sid_app_ctx);
				break;
			}
			case EVENT_TYPE_CONNECTION_REQUEST:
			{
				connection_request(sid_app_ctx);
				break;
			}
			default: break;
			}
		}
	}
}

void sidewalk_thread_message_q_write(enum event_type event)
{
	while (0 != k_msgq_put(&sid_msgq, &event, K_NO_WAIT)) {
		k_msgq_purge(&sid_msgq);
	}
}

void sidewalk_thread_enable(void)
{
	static app_context_t sid_app_ctx = {
		.sidewalk_handle = NULL,
		.state = STATE_INIT,
	};

	sid_tid = k_thread_create(&sid_thread, sid_stack_area,
				  K_THREAD_STACK_SIZEOF(sid_stack_area),
				  sidewalk_thread, &sid_app_ctx, NULL, NULL,
				  CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&sid_thread, "sidewalk");
}
