/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app.h>
#include <sidewalk.h>
#include <sidewalk_dfu/nordic_dfu.h>

#include <buttons.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/smf.h>

LOG_MODULE_REGISTER(sid_location_app, CONFIG_SIDEWALK_LOG_LEVEL);

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

static void on_sidewalk_process(void)
{
	app_event_post(APP_EVENT_SID_PROCESS);
}

static void on_sidewalk_status(const struct sid_status *status)
{
	app.status = *status;
	app_event_post(APP_EVENT_SID_STATUS);
}

static void on_location_result(const struct sid_location_result *result)
{
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

static void location_run_started(int err)
{
	if (err) {
		led_blink(APP_LED_ERROR);
		cycle_restart();
		return;
	}

	app.run_pending = false;
	k_timer_stop(&ble_timer);
	smf_set_state(SMF_CTX(&app), &app_states[APP_STATE_LOCATING]);
}

static void location_run_auto(void)
{
	location_run_started(sidewalk_location_run_auto());
}

static void location_start_auto(void)
{
	/* L1 sends nothing that would trigger auto connect, so it needs the link up */
	if (sidewalk_location_effort() == SID_LOCATION_EFFORT_L1 &&
	    !(app.status.detail.link_status_mask & SID_LINK_TYPE_1)) {
		LOG_INF("Location run waits for a Bluetooth LE connection");
		app.run_pending = true;
		sidewalk_ble_connect_request();
		k_timer_start(&ble_timer, K_SECONDS(SIDEWALK_BLE_CONNECT_TIMEOUT_S), K_NO_WAIT);
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
	int err = sidewalk_location_run_level(mode);

	if (err == -ENOTSUP) {
		LOG_INF("L%d is not available", mode);
		return;
	}
	location_run_started(err);
}

static void location_msg_send(void)
{
	if (sidewalk_location_msg_send()) {
		led_blink(APP_LED_ERROR);
		return;
	}
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
	if (sidewalk_stop()) {
		led_blink(APP_LED_ERROR);
		return;
	}

	int err = nordic_dfu_ble_start();

	if (err) {
		LOG_ERR("DFU start err %d", err);
		led_blink(APP_LED_ERROR);
		(void)sidewalk_start();
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

	if (sidewalk_location_init()) {
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

	if (sidewalk_start()) {
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

static void sidewalk_thread_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	static const struct sidewalk_callbacks callbacks = {
		.on_process = on_sidewalk_process,
		.on_status = on_sidewalk_status,
		.on_location = on_location_result,
	};

	if (sidewalk_init(&callbacks)) {
		led_blink(APP_LED_ERROR);
		return;
	}

	LOG_INF("Waiting for Sidewalk registration and time sync");
	smf_set_initial(SMF_CTX(&app), &app_states[APP_STATE_CONNECTING]);

	while (true) {
		k_msgq_get(&app_msgq, &app.event, K_FOREVER);

		if (app.event == APP_EVENT_SID_PROCESS) {
			sidewalk_process();
			continue;
		}
		smf_run_state(SMF_CTX(&app));
	}
}

static struct k_thread sidewalk_thread;
static K_THREAD_STACK_DEFINE(sidewalk_thread_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);

void app_start(void)
{
#if defined(CONFIG_UART_CONSOLE)
	(void)pm_device_runtime_disable(DEVICE_DT_GET(DT_CHOSEN(zephyr_console)));
#endif

	if (ui_init()) {
		LOG_ERR("UI init failed");
	}

	(void)k_thread_create(&sidewalk_thread, sidewalk_thread_stack,
			      K_THREAD_STACK_SIZEOF(sidewalk_thread_stack), sidewalk_thread_entry,
			      NULL, NULL, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	(void)k_thread_name_set(&sidewalk_thread, "sidewalk");
}
