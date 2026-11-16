/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app.h>
#include <app_ble_config.h>
#include <app_mfg_config.h>
#include <app_subGHz_config.h>
#include <bt_app_callbacks.h>
#include <json_printer/sidTypes2str.h>
#include <sid_api.h>
#include <sid_hal_reset_ifc.h>
#include <sid_location.h>
#include <sid_pal_common_ifc.h>
#include <sidewalk_dfu/nordic_dfu.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <sid_pal_radio_ifc.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
#include <app_location_lr11xx_config.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */

#include <buttons.h>
#include <stdio.h>
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

#ifdef CONFIG_SIDEWALK_SUBGHZ_REGION_EU868
#define APP_SUBGHZ_REGION SID_PAL_RADIO_RC_EU
#else
#define APP_SUBGHZ_REGION SID_PAL_RADIO_RC_NA
#endif /* CONFIG_SIDEWALK_SUBGHZ_REGION_EU868 */

#define APP_BLE_CONNECT_TIMEOUT_S 30

#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
#define APP_LOCATION_METHODS                                                                       \
	(SID_LOCATION_METHOD_GATEWAY | SID_LOCATION_METHOD_WIFI | SID_LOCATION_METHOD_GNSS)
#define APP_LOCATION_MAX_EFFORT SID_LOCATION_EFFORT_L4
#define APP_LOCATION_AUTO_MODE	SID_LOCATION_EFFORT_DEFAULT
#else
#define APP_LOCATION_METHODS	SID_LOCATION_METHOD_GATEWAY
#define APP_LOCATION_MAX_EFFORT SID_LOCATION_EFFORT_L1
#define APP_LOCATION_AUTO_MODE	SID_LOCATION_EFFORT_L1
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */

enum app_event {
	APP_EVENT_SID_PROCESS,
	APP_EVENT_SID_STATUS,
	APP_EVENT_CYCLE,
	APP_EVENT_BLE_TIMEOUT,
	APP_EVENT_DFU_END,
	APP_EVENT_BUTTON_AUTO,
	APP_EVENT_BUTTON_MSG,
	APP_EVENT_BUTTON_WIFI,
	APP_EVENT_BUTTON_GNSS,
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

static void cycle_restart(void)
{
	k_timer_start(&cycle_timer, K_SECONDS(CONFIG_SID_LOCATION_INTERVAL_S), K_NO_WAIT);
}

static void ble_timer_cb(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	app_event_post(APP_EVENT_BLE_TIMEOUT);
}

static K_TIMER_DEFINE(ble_timer, ble_timer_cb, NULL);

static void dfu_check(struct k_work *work)
{
	if (nordic_dfu_is_in_dfu()) {
		(void)k_work_reschedule(k_work_delayable_from_work(work), K_SECONDS(1));
		return;
	}
	app_event_post(APP_EVENT_DFU_END);
}

static K_WORK_DELAYABLE_DEFINE(dfu_check_work, dfu_check);

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

	(void)button_set_action_short_press(DK_BTN1, on_button,
					    DT_NODE_EXISTS(DT_ALIAS(sw1)) ? APP_EVENT_BUTTON_AUTO
									  : APP_EVENT_BUTTON_MSG);
	(void)button_set_action_long_press(DK_BTN1, on_button, APP_EVENT_BUTTON_DFU);
	(void)button_set_action(DK_BTN2, on_button, APP_EVENT_BUTTON_MSG);
	(void)button_set_action(DK_BTN3, on_button, APP_EVENT_BUTTON_WIFI);
	(void)button_set_action(DK_BTN4, on_button, APP_EVENT_BUTTON_GNSS);

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

	LOG_INF("Factory reset done, rebooting");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Cannot reboot");
	}
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

static int location_init(void)
{
	struct sid_location_config config = {
		.sid_location_type_mask = APP_LOCATION_METHODS,
		.max_effort = APP_LOCATION_MAX_EFFORT,
		.manage_effort = true,
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

static void location_run(struct sid_location_run_config *config)
{
	sid_error_t err = sid_location_run(app.handle, config, 0);

	if (err) {
		LOG_ERR("Location run err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		led_blink(APP_LED_ERROR);
		cycle_restart();
		return;
	}

	app.run_pending = false;
	k_timer_stop(&ble_timer);
	LOG_INF("Location run started at L%d", sid_location_get_effort_mode(app.handle));
	smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_LOCATING]);
}

static void location_run_auto(void)
{
	struct sid_location_run_config config = {
		.mode = APP_LOCATION_AUTO_MODE,
		.type = SID_LOCATION_SCAN_AND_SEND,
	};

	location_run(&config);
}

static void location_start_auto(void)
{
	/* L1 sends nothing that would trigger auto connect, so it needs the link up */
	if (sid_location_get_effort_mode(app.handle) == SID_LOCATION_EFFORT_L1 &&
	    !(app.status.detail.link_status_mask & SID_LINK_TYPE_1)) {
		LOG_INF("Location run waits for a Bluetooth LE connection");
		app.run_pending = true;
		(void)sid_ble_bcn_connection_request(app.handle, true);
		k_timer_start(&ble_timer, K_SECONDS(APP_BLE_CONNECT_TIMEOUT_S), K_NO_WAIT);
		return;
	}
	location_run_auto();
}

static void location_ble_timeout(void)
{
	if (!app.run_pending) {
		return;
	}
	app.run_pending = false;
	LOG_WRN("No Bluetooth LE connection");
	location_run_auto();
}

static void location_run_level(enum sid_location_effort_mode mode)
{
	struct sid_location_run_config config = {
		.type = SID_LOCATION_SCAN_AND_SEND,
		.levels = {mode},
		.num_levels = 1,
	};

	if (!IS_ENABLED(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)) {
		LOG_INF("L%d is not available", mode);
		return;
	}
	location_run(&config);
}

static void location_msg_send(void)
{
	static unsigned int counter;
	char text[24];
	struct sid_msg msg = {
		.data = text,
		.size = snprintf(text, sizeof(text), "Message %u", ++counter),
	};
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
	cycle_restart();
}

static void location_msg_result(void)
{
	if (app.result.status == SID_LOCATION_RESOLVE_MSG_SENT) {
		led_blink(APP_LED_BLE);
	} else if (app.result.status == SID_LOCATION_RESOLVE_MSG_TIMEOUT) {
		led_blink(APP_LED_ERROR);
	}
}

static void dfu_start(void)
{
	sid_error_t sid_err = sid_stop(app.handle, APP_LINK_MASK);

	if (sid_err) {
		LOG_ERR("Sidewalk stop err %d (%s)", (int)sid_err, SID_ERROR_T_STR(sid_err));
		led_blink(APP_LED_ERROR);
		return;
	}

	int err = nordic_dfu_ble_start();

	if (err) {
		LOG_ERR("DFU start err %d", err);
		led_blink(APP_LED_ERROR);
		(void)sid_start(app.handle, APP_LINK_MASK);
		return;
	}
	app.run_pending = false;
	k_timer_stop(&ble_timer);
	k_timer_stop(&cycle_timer);
	(void)k_work_reschedule(&dfu_check_work, K_SECONDS(1));
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
	cycle_restart();
	return SMF_EVENT_HANDLED;
}

static enum smf_state_result idle_run(void *o)
{
	ARG_UNUSED(o);

	switch (app.event) {
	case APP_EVENT_CYCLE:
	case APP_EVENT_BUTTON_AUTO:
		location_start_auto();
		break;
	case APP_EVENT_BLE_TIMEOUT:
		location_ble_timeout();
		break;
	case APP_EVENT_BUTTON_WIFI:
		location_run_level(SID_LOCATION_EFFORT_L3);
		break;
	case APP_EVENT_BUTTON_GNSS:
		location_run_level(SID_LOCATION_EFFORT_L4);
		break;
	case APP_EVENT_BUTTON_MSG:
		location_msg_send();
		break;
	case APP_EVENT_BUTTON_DFU:
		dfu_start();
		break;
	case APP_EVENT_LOCATION_RESULT:
		if (app.result.status == SID_LOCATION_LVL1_READY) {
			location_start_auto();
			break;
		}
		location_msg_result();
		break;
	case APP_EVENT_SID_STATUS:
		if (app.run_pending && (app.status.detail.link_status_mask & SID_LINK_TYPE_1)) {
			location_run_auto();
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

	switch (app.event) {
	case APP_EVENT_BUTTON_AUTO:
	case APP_EVENT_BUTTON_WIFI:
	case APP_EVENT_BUTTON_GNSS:
	case APP_EVENT_BUTTON_DFU:
		LOG_INF("Location run in progress");
		break;
	case APP_EVENT_BUTTON_MSG:
		location_msg_send();
		break;
	case APP_EVENT_LOCATION_RESULT:
		LOG_INF("Location status %d, err %d, effort %d, link 0x%x", result->status,
			(int)result->err, result->mode, result->link);
		location_msg_result();
		if (result->status != SID_LOCATION_SCAN_AND_SEND_DONE) {
			break;
		}
		led_blink(result->err == SID_ERROR_NONE ? effort_to_led(result->mode)
							: APP_LED_ERROR);
		smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
		break;
	default:
		break;
	}

	return SMF_EVENT_HANDLED;
}

static void locating_exit(void *o)
{
	ARG_UNUSED(o);
	cycle_restart();
}

static enum smf_state_result dfu_run(void *o)
{
	ARG_UNUSED(o);

	switch (app.event) {
	case APP_EVENT_BUTTON_DFU:
		if (nordic_dfu_is_in_dfu()) {
			(void)nordic_dfu_ble_stop();
		}
		smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
		break;
	case APP_EVENT_DFU_END:
		smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_IDLE]);
		break;
	default:
		break;
	}

	return SMF_EVENT_HANDLED;
}

static void dfu_exit(void *o)
{
	ARG_UNUSED(o);

	(void)k_work_cancel_delayable(&dfu_check_work);
	(void)k_work_reschedule(&leds_off_work, K_NO_WAIT);

	sid_error_t err = sid_start(app.handle, APP_LINK_MASK);

	if (err) {
		LOG_ERR("Sidewalk start err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		led_blink(APP_LED_ERROR);
	}
	cycle_restart();
}

static const struct smf_state app_states[] = {
	[APP_STATE_CONNECTING] = SMF_CREATE_STATE(NULL, connecting_run, NULL, NULL, NULL),
	[APP_STATE_IDLE] = SMF_CREATE_STATE(NULL, idle_run, NULL, NULL, NULL),
	[APP_STATE_LOCATING] = SMF_CREATE_STATE(NULL, locating_run, locating_exit, NULL, NULL),
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
		.platform_init_parameters.region = APP_SUBGHZ_REGION,
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
		.connection_attempt_timeout_seconds = APP_BLE_CONNECT_TIMEOUT_S,
	};

	(void)sid_option(app.handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &policy,
			 sizeof(policy));
	(void)sid_option(app.handle, SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS, &params,
			 sizeof(params));

	return 0;
}

static void sidewalk_thread_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (platform_init() || sidewalk_start()) {
		led_blink(APP_LED_ERROR);
		return;
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
}

static struct k_thread sidewalk_thread;
static K_THREAD_STACK_DEFINE(sidewalk_thread_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);

void app_start(void)
{
	if (ui_init()) {
		LOG_ERR("UI init failed");
	}

	(void)k_thread_create(&sidewalk_thread, sidewalk_thread_stack,
			      K_THREAD_STACK_SIZEOF(sidewalk_thread_stack), sidewalk_thread_entry,
			      NULL, NULL, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	(void)k_thread_name_set(&sidewalk_thread, "sidewalk");
}
