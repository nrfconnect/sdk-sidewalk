/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app_ble_config.h>
#include <app_mfg_config.h>
#include <app_subGHz_config.h>
#include <bt_app_callbacks.h>
#include <json_printer/sidTypes2str.h>
#include <sid_api.h>
#include <sid_location.h>
#include <sid_pal_common_ifc.h>
#include <sidewalk_dfu/nordic_dfu.h>
#include <sidewalk_version.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <sid_pal_radio_ifc.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
#include <app_location_lr11xx_config.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */

#include <buttons.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/smf.h>

LOG_MODULE_REGISTER(sid_location_app, CONFIG_SIDEWALK_LOG_LEVEL);

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#define APP_LINK_MASK (SID_LINK_TYPE_1 | SID_LINK_TYPE_3)
#else
#define APP_LINK_MASK (SID_LINK_TYPE_1)
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */

#define APP_LOCATION_METHODS                                                                       \
	(SID_LOCATION_METHOD_GATEWAY |                                                             \
	 (IS_ENABLED(CONFIG_SID_LOCATION_WIFI) ? SID_LOCATION_METHOD_WIFI : 0) |                   \
	 (IS_ENABLED(CONFIG_SID_LOCATION_GNSS) ? SID_LOCATION_METHOD_GNSS : 0))

static const enum sid_location_effort_mode levels[] = {
	SID_LOCATION_EFFORT_L1,
#ifdef CONFIG_SID_LOCATION_WIFI
	SID_LOCATION_EFFORT_L3,
#endif
#ifdef CONFIG_SID_LOCATION_GNSS
	SID_LOCATION_EFFORT_L4,
#endif
};

enum app_event {
	APP_EVENT_SID_PROCESS,
	APP_EVENT_SID_STATUS,
	APP_EVENT_CYCLE,
	APP_EVENT_BUTTON_L1,
	APP_EVENT_BUTTON_L3,
	APP_EVENT_BUTTON_L4,
	APP_EVENT_BUTTON_MSG,
	APP_EVENT_BUTTON_DFU,
	APP_EVENT_LOCATION_RESULT,
};

K_MSGQ_DEFINE(app_msgq, sizeof(enum app_event), 16, 4);

enum app_state {
	APP_STATE_CONNECTING,
	APP_STATE_IDLE,
	APP_STATE_LOCATING,
	APP_STATE_DFU,
};

struct app_ctx {
	struct smf_ctx smf;
	enum app_event event;
	struct sid_handle *handle;
	struct sid_status status;
	struct sid_location_result result;
	size_t run_first;
	size_t run_last;
	bool run_pending;
};

static struct app_ctx app;
static const struct smf_state app_states[];

static void app_event_post(enum app_event event)
{
	if (k_msgq_put(&app_msgq, &event, K_NO_WAIT)) {
		LOG_WRN("Event %d dropped, queue full", event);
	}
}

static void cycle_timer_cb(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	app_event_post(APP_EVENT_CYCLE);
}

static K_TIMER_DEFINE(cycle_timer, cycle_timer_cb, NULL);

#define LED_BLINK_MS 100
#define LED_SPEC(alias) GPIO_DT_SPEC_GET_OR(DT_ALIAS(alias), gpios, {0})

enum app_led {
	APP_LED_BLE,
	APP_LED_WIFI,
	APP_LED_GNSS,
	APP_LED_ERROR,
	APP_LED_COUNT,
};

static const struct gpio_dt_spec leds[APP_LED_COUNT] = {
	[APP_LED_BLE] = LED_SPEC(loc_led_ble),
	[APP_LED_WIFI] = LED_SPEC(loc_led_wifi),
	[APP_LED_GNSS] = LED_SPEC(loc_led_gnss),
	[APP_LED_ERROR] = LED_SPEC(loc_led_error),
};

static void leds_off(struct k_work *work)
{
	ARG_UNUSED(work);

	for (int i = 0; i < APP_LED_COUNT; i++) {
		if (leds[i].port) {
			(void)gpio_pin_set_dt(&leds[i], 0);
		}
	}
}

static K_WORK_DELAYABLE_DEFINE(leds_off_work, leds_off);

static void led_blink(enum app_led led)
{
	if (led >= APP_LED_COUNT || !leds[led].port) {
		return;
	}
	(void)gpio_pin_set_dt(&leds[led], 1);
	(void)k_work_reschedule(&leds_off_work, K_MSEC(LED_BLINK_MS));
}

static void on_button(uint32_t event)
{
	app_event_post(event);
}

static int ui_init(void)
{
	for (int i = 0; i < APP_LED_COUNT; i++) {
		if (leds[i].port && (!gpio_is_ready_dt(&leds[i]) ||
				     gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE))) {
			return -EIO;
		}
	}

	(void)button_set_action_short_press(DK_BTN1, on_button, APP_EVENT_BUTTON_L1);
	(void)button_set_action_long_press(DK_BTN1, on_button, APP_EVENT_BUTTON_DFU);
	(void)button_set_action(DK_BTN2, on_button, APP_EVENT_BUTTON_L3);
	(void)button_set_action(DK_BTN3, on_button, APP_EVENT_BUTTON_L4);
	(void)button_set_action(DK_BTN4, on_button, APP_EVENT_BUTTON_MSG);

	return buttons_init();
}

static void on_sidewalk_event(bool in_isr, void *context)
{
	ARG_UNUSED(in_isr);
	ARG_UNUSED(context);
	app_event_post(APP_EVENT_SID_PROCESS);
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	ARG_UNUSED(context);

	app.status = *status;
	LOG_INF("Sidewalk state %d, registered %d, time synced %d, links 0x%x", status->state,
		status->detail.registration_status == SID_STATUS_REGISTERED,
		status->detail.time_sync_status == SID_STATUS_TIME_SYNCED,
		status->detail.link_status_mask);
	app_event_post(APP_EVENT_SID_STATUS);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	ARG_UNUSED(msg_desc);
	ARG_UNUSED(msg);
	ARG_UNUSED(context);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	ARG_UNUSED(msg_desc);
	ARG_UNUSED(context);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	ARG_UNUSED(msg_desc);
	ARG_UNUSED(context);
	LOG_WRN("Send error %d (%s)", (int)error, SID_ERROR_T_STR(error));
}

static void on_sidewalk_factory_reset(void *context)
{
	ARG_UNUSED(context);
}

static void on_location_update(const struct sid_location_result *const result, void *context)
{
	ARG_UNUSED(context);

	app.result = *result;
	app_event_post(APP_EVENT_LOCATION_RESULT);
}

static enum app_led effort_to_led(enum sid_location_effort_mode mode)
{
	switch (mode) {
	case SID_LOCATION_EFFORT_L1:
		return APP_LED_BLE;
	case SID_LOCATION_EFFORT_L3:
		return APP_LED_WIFI;
	case SID_LOCATION_EFFORT_L4:
		return APP_LED_GNSS;
	default:
		return APP_LED_COUNT;
	}
}

static size_t level_index(enum sid_location_effort_mode mode)
{
	for (size_t i = 0; i < ARRAY_SIZE(levels); i++) {
		if (levels[i] == mode) {
			return i;
		}
	}
	return ARRAY_SIZE(levels);
}

static int location_init(void)
{
	struct sid_location_config config = {
		.sid_location_type_mask = APP_LOCATION_METHODS,
		.max_effort = levels[ARRAY_SIZE(levels) - 1],
		.callbacks = {
			.on_update = on_location_update,
		},
		.auto_link_pause_resume = IS_ENABLED(CONFIG_SIDEWALK_SUBGHZ_SUPPORT),
	};

	sid_error_t err = sid_location_init(app.handle, &config);

	if (err) {
		LOG_ERR("Location init err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}
	return 0;
}

/* The library moves to the next level when a level is not available or its scan finds nothing */
static void location_run(size_t first)
{
	struct sid_location_run_config config = {
		.type = SID_LOCATION_SCAN_AND_SEND,
		.num_levels = app.run_last - first + 1,
	};

	memcpy(config.levels, &levels[first], config.num_levels * sizeof(levels[0]));

	sid_error_t err = sid_location_run(app.handle, &config, 0);

	if (err) {
		LOG_ERR("Location run err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		led_blink(APP_LED_ERROR);
		smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
		return;
	}

	LOG_INF("Location run started at L%d", levels[first]);
	smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_LOCATING]);
}

static void location_start(size_t first, size_t last)
{
	app.run_first = first;
	app.run_last = last;

	/* L1 sends nothing that would trigger auto connect, so it needs the link up */
	app.run_pending = levels[first] == SID_LOCATION_EFFORT_L1 &&
			  !(app.status.detail.link_status_mask & SID_LINK_TYPE_1);
	if (app.run_pending) {
		LOG_INF("Location run waits for a Bluetooth LE connection");
		(void)sid_ble_bcn_connection_request(app.handle, true);
		return;
	}
	location_run(first);
}

static void location_start_all(void)
{
	location_start(0, ARRAY_SIZE(levels) - 1);
}

static void location_start_level(enum sid_location_effort_mode mode)
{
	size_t i = level_index(mode);

	if (i == ARRAY_SIZE(levels)) {
		LOG_INF("L%d is not enabled", mode);
		return;
	}
	location_start(i, i);
}

static void location_msg_send(void)
{
	static unsigned int counter;
	char text[24];
	struct sid_msg msg = {
		.data = text,
		.size = snprintf(text, sizeof(text), "Message %u", ++counter),
	};
	/* The position of a message is resolved from a Bluetooth LE gateway */
	struct sid_msg_desc desc = {
		.type = SID_MSG_TYPE_NOTIFY,
		.link_type = SID_LINK_TYPE_1,
		.link_mode = SID_LINK_MODE_CLOUD,
		.msg_desc_attr.tx_attr.resolve_location = SID_TX_LOCATION_LINK_RESOLVE,
	};

	sid_error_t err = sid_put_msg(app.handle, &msg, &desc);

	if (err) {
		LOG_ERR("Message err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		led_blink(APP_LED_ERROR);
		return;
	}
	LOG_INF("\"%s\" with location resolution queued", text);
}

static void dfu_start(void)
{
	int err = nordic_dfu_ble_start();

	if (err) {
		LOG_ERR("DFU start err %d", err);
		led_blink(APP_LED_ERROR);
		return;
	}
	smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_DFU]);
}

static enum smf_state_result connecting_run(void *o)
{
	ARG_UNUSED(o);

	if (app.event != APP_EVENT_SID_STATUS || app.status.state != SID_STATE_READY ||
	    app.status.detail.registration_status != SID_STATUS_REGISTERED ||
	    app.status.detail.time_sync_status != SID_STATUS_TIME_SYNCED) {
		return SMF_EVENT_HANDLED;
	}

	if (location_init()) {
		led_blink(APP_LED_ERROR);
		return SMF_EVENT_HANDLED;
	}

	smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
	k_timer_start(&cycle_timer, K_SECONDS(CONFIG_SID_LOCATION_INTERVAL_S),
		      K_SECONDS(CONFIG_SID_LOCATION_INTERVAL_S));
	location_start_all();
	return SMF_EVENT_HANDLED;
}

static enum smf_state_result idle_run(void *o)
{
	ARG_UNUSED(o);

	switch (app.event) {
	case APP_EVENT_CYCLE:
		location_start_all();
		break;
	case APP_EVENT_BUTTON_L1:
		location_start_level(SID_LOCATION_EFFORT_L1);
		break;
	case APP_EVENT_BUTTON_L3:
		location_start_level(SID_LOCATION_EFFORT_L3);
		break;
	case APP_EVENT_BUTTON_L4:
		location_start_level(SID_LOCATION_EFFORT_L4);
		break;
	case APP_EVENT_BUTTON_MSG:
		location_msg_send();
		break;
	case APP_EVENT_BUTTON_DFU:
		dfu_start();
		break;
	case APP_EVENT_LOCATION_RESULT:
		if (app.result.status == SID_LOCATION_RESOLVE_MSG_SENT) {
			led_blink(APP_LED_BLE);
		} else if (app.result.status == SID_LOCATION_RESOLVE_MSG_TIMEOUT) {
			led_blink(APP_LED_ERROR);
		}
		break;
	case APP_EVENT_SID_STATUS:
		if (app.run_pending && (app.status.detail.link_status_mask & SID_LINK_TYPE_1)) {
			location_start(app.run_first, app.run_last);
		}
		break;
	default:
		break;
	}

	return SMF_EVENT_HANDLED;
}

static enum smf_state_result locating_run(void *o)
{
	ARG_UNUSED(o);
	const struct sid_location_result *result = &app.result;
	size_t next = level_index(result->mode) + 1;

	switch (app.event) {
	case APP_EVENT_LOCATION_RESULT:
		LOG_INF("Location status %d, err %d, effort %d, link 0x%x", result->status,
			(int)result->err, result->mode, result->link);
		/* The library also reports events that are not a run result */
		if (result->status != SID_LOCATION_SCAN_AND_SEND_DONE) {
			break;
		}
		if (result->err == SID_ERROR_NONE) {
			led_blink(effort_to_led(result->mode));
			smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
			break;
		}
		/* The library does not move on after a failed uplink */
		if (next <= app.run_last) {
			location_run(next);
			break;
		}
		led_blink(APP_LED_ERROR);
		smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
		break;
	default:
		break;
	}

	return SMF_EVENT_HANDLED;
}

static enum smf_state_result dfu_run(void *o)
{
	ARG_UNUSED(o);

	switch (app.event) {
	case APP_EVENT_BUTTON_DFU:
		(void)nordic_dfu_ble_stop();
		smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
		break;
	case APP_EVENT_CYCLE:
		/* The DFU mode ends by itself when no upload starts in time */
		if (!nordic_dfu_is_in_dfu()) {
			smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
		}
		break;
	default:
		break;
	}

	return SMF_EVENT_HANDLED;
}

static void dfu_exit(void *o)
{
	ARG_UNUSED(o);
	leds_off(NULL);
}

static const struct smf_state app_states[] = {
	[APP_STATE_CONNECTING] = SMF_CREATE_STATE(NULL, connecting_run, NULL, NULL, NULL),
	[APP_STATE_IDLE] = SMF_CREATE_STATE(NULL, idle_run, NULL, NULL, NULL),
	[APP_STATE_LOCATING] = SMF_CREATE_STATE(NULL, locating_run, NULL, NULL, NULL),
	[APP_STATE_DFU] = SMF_CREATE_STATE(NULL, dfu_run, dfu_exit, NULL, NULL),
};

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
static sid_pal_radio_rx_packet_t radio_rx_packet;

static void radio_event_notifier(sid_pal_radio_events_t event)
{
	ARG_UNUSED(event);
}

static void radio_irq_handler(void)
{
}
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */

static int platform_init(void)
{
	platform_parameters_t platform_parameters = {
		.mfg_store_region.addr_start = APP_MFG_CFG_FLASH_START,
		.mfg_store_region.addr_end = APP_MFG_CFG_FLASH_END,
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
		.platform_init_parameters.radio_cfg = get_radio_cfg(),
		.platform_init_parameters.region = SID_PAL_RADIO_RC_NA,
		.platform_init_parameters.sub_ghz_cfg = app_get_sub_ghz_config(),
#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
		.platform_init_parameters.gnss_wifi_cfg =
			(lr11xx_gnss_wifi_config_t *)get_location_cfg(),
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
	};

	sid_error_t err = sid_platform_init(&platform_parameters);

	if (err) {
		LOG_ERR("Sidewalk platform init err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}

	if (app_mfg_cfg_is_empty()) {
		LOG_ERR("No manufacturing data at 0x%08x, provision the device",
			APP_MFG_CFG_FLASH_START);
		return -ENODATA;
	}

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
	int32_t radio_err =
		sid_pal_radio_init(radio_event_notifier, radio_irq_handler, &radio_rx_packet);

	if (radio_err) {
		LOG_ERR("Radio init err %d", radio_err);
		return -EIO;
	}
	(void)sid_pal_radio_sleep(0);
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */

	return 0;
}

/* The Sidewalk and SMP services are always registered, each identity may use only its own */
static bool gatt_authorize(struct bt_conn *conn, const struct bt_gatt_attr *attr)
{
	struct bt_conn_info info;

	if (bt_conn_get_info(conn, &info)) {
		return false;
	}
	if (info.id == BT_ID_SIDEWALK) {
		return !sid_ble_bt_attr_is_SMP(attr);
	}
	if (info.id == BT_ID_SMP_DFU) {
		return !sid_ble_bt_attr_is_SIDEWALK(attr);
	}
	return true;
}

static const struct bt_gatt_authorization_cb gatt_authorization = {
	.read_authorize = gatt_authorize,
	.write_authorize = gatt_authorize,
};

static int sidewalk_start(void)
{
	static struct sid_event_callbacks callbacks = {
		.on_event = on_sidewalk_event,
		.on_msg_received = on_sidewalk_msg_received,
		.on_msg_sent = on_sidewalk_msg_sent,
		.on_send_error = on_sidewalk_send_error,
		.on_status_changed = on_sidewalk_status_changed,
		.on_factory_reset = on_sidewalk_factory_reset,
	};
	struct sid_config config = {
		.link_mask = APP_LINK_MASK,
		.dev_ch = {
			.type = SID_END_DEVICE_TYPE_STATIC,
			.power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
			.qualification_id = 0x0001,
		},
		.callbacks = &callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	if (bt_gatt_authorization_cb_register(&gatt_authorization)) {
		LOG_ERR("GATT authorization register failed");
		return -EIO;
	}

	sid_error_t err = sid_init(&config, &app.handle);

	if (err) {
		LOG_ERR("Sidewalk init err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}
	err = sid_start(app.handle, APP_LINK_MASK);
	if (err) {
		LOG_ERR("Sidewalk start err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}

	enum sid_link_connection_policy policy = SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;
	struct sid_link_auto_connect_params params = {
		.link_type = SID_LINK_TYPE_1,
		.enable = true,
		.connection_attempt_timeout_seconds = 30,
	};

	(void)sid_option(app.handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &policy,
			 sizeof(policy));
	(void)sid_option(app.handle, SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS, &params,
			 sizeof(params));

	return 0;
}

int main(void)
{
	PRINT_SIDEWALK_VERSION();

	/* sid_process() runs in this thread, keep the priority of the Sidewalk thread */
	k_thread_priority_set(k_current_get(), CONFIG_SIDEWALK_THREAD_PRIORITY);

	if (ui_init()) {
		LOG_ERR("UI init failed");
	}

	if (platform_init() || sidewalk_start()) {
		led_blink(APP_LED_ERROR);
		return 0;
	}

	LOG_INF("Waiting for Sidewalk registration and time sync");
	smf_set_initial(SMF_CTX(&app), &app_states[APP_STATE_CONNECTING]);

	while (true) {
		k_msgq_get(&app_msgq, &app.event, K_FOREVER);

		if (app.event == APP_EVENT_SID_PROCESS) {
			(void)sid_process(app.handle);
			continue;
		}
		smf_run_state(SMF_CTX(&app));
	}

	return 0;
}
