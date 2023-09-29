/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sid_api.h>
#include <sid_error.h>
#include <sid_pal_assert_ifc.h>
#include <app_ble_config.h>

#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include <sidewalk_version.h>
#if defined(CONFIG_BOOTLOADER_MCUBOOT)
#include <zephyr/dfu/mcuboot.h>
#endif
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_USB)
#include <zephyr/usb/usb_device.h>
#endif
#if defined(CONFIG_GPIO)
#include <state_notifier_gpio_backend.h>
#endif
#if defined(CONFIG_LOG)
#include <state_notifier_log_backend.h>
#endif

#include <pal_init.h>
#include <buttons.h>
#include <application_thread.h>
#include <sidewalk_callbacks.h>
#include <state_notifier.h>

#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(sid_template, CONFIG_SIDEWALK_LOG_LEVEL);

bool factory_reset_bypass = false;

int alpha_handle_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg);
int alpha_handle_export(int (*cb)(const char *name, const void *value, size_t val_len));

struct settings_handler alph_handler = { .name = "test",
					 .h_get = NULL,
					 .h_set = alpha_handle_set,
					 .h_commit = NULL,
					 .h_export = alpha_handle_export };

K_SEM_DEFINE(registered_sem, 0, 1);
K_SEM_DEFINE(not_sending_sem, 0, 1);
K_SEM_DEFINE(working_sem, 0, 1);
K_SEM_DEFINE(connected_sem, 0, 1);
K_SEM_DEFINE(time_sync_sem, 0, 1);
K_SEM_DEFINE(link_sem, 0, 1);

static app_ctx_t app_context;

volatile struct notifier_state current_app_state = {};

int alpha_handle_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg)
{
	const char *next;

	if (settings_name_steq(name, "factory_reset_bypass", &next)) {
		if (len != sizeof(factory_reset_bypass)) {
			return -EINVAL;
		}
		read_cb(cb_arg, &factory_reset_bypass, sizeof(factory_reset_bypass));
		return 0;
	}
	return -ENOENT;
}

int alpha_handle_export(int (*cb)(const char *name, const void *value, size_t val_len))
{
	(void)cb("test/factory_reset_bypass", &factory_reset_bypass, sizeof(factory_reset_bypass));
	return 0;
}

void assert_post_action(const char *file, unsigned int line)
{
	LOG_ERR("Assert in %s:%d", file, line);

	k_panic();
}

static void button_handler(uint32_t event)
{
	app_event_send((app_event_t)event);
}

static sid_error_t app_buttons_init(btn_handler_t handler)
{
	button_set_action_long_press(DK_BTN1, handler, BUTTON_EVENT_FACTORY_RESET);
	button_set_action(DK_BTN2, handler, BUTTON_EVENT_CONNECTION_REQUEST);
	button_set_action(DK_BTN3, handler, BUTTON_EVENT_SEND_HELLO);
	button_set_action_short_press(DK_BTN4, handler, BUTTON_EVENT_SET_BATTERY_LEVEL);
#if defined(CONFIG_SIDEWALK_DFU)
	button_set_action_long_press(DK_BTN4, handler, BUTTON_EVENT_NORDIC_DFU);
#endif

	return buttons_init() ? SID_ERROR_GENERIC : SID_ERROR_NONE;
}

static void app_setup(void)
{
	if (app_buttons_init(button_handler)) {
		LOG_ERR("Failed to initialze buttons.");
		SID_PAL_ASSERT(false);
	}

	if (dk_leds_init()) {
		LOG_ERR("Failed to initialze LEDs.");
		SID_PAL_ASSERT(false);
	}
#if defined(CONFIG_GPIO)
	state_watch_init_gpio(&global_state_notifier);
#endif
#if defined(CONFIG_LOG)
	state_watch_init_log(&global_state_notifier);
#endif

#if defined(CONFIG_SIDEWALK_DFU_SERVICE_USB)
	if (usb_enable(NULL)) {
		LOG_ERR("Failed to enable USB");
		return;
	}
#endif

	if (sidewalk_callbacks_set(&app_context, &app_context.event_callbacks)) {
		LOG_ERR("Failed to set sidewalk callbacks");
		SID_PAL_ASSERT(false);
	}

	app_context.config = (struct sid_config){
		.link_mask = BUILT_IN_LM,
		.time_sync_periodicity_seconds = 7200,
		.callbacks = &app_context.event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = NULL,
	};

#if defined(CONFIG_BOOTLOADER_MCUBOOT)
	if (!boot_is_img_confirmed()) {
		int ret = boot_write_img_confirmed();

		if (ret) {
			LOG_ERR("Couldn't confirm image: %d", ret);
		} else {
			LOG_INF("Marked image as OK");
		}
	}
#endif
}

static void state_change_handler_power_test(const struct notifier_state *state)
{
	current_app_state = *state;
	if (current_app_state.registered) {
		k_sem_give(&registered_sem);
	}
	if (current_app_state.sending == 0) {
		k_sem_give(&not_sending_sem);
	} else {
		k_sem_reset(&not_sending_sem);
	}
	if (current_app_state.connected) {
		k_sem_give(&connected_sem);
	} else {
		k_sem_reset(&connected_sem);
	}
	if (current_app_state.time_sync) {
		k_sem_give(&time_sync_sem);
	} else {
		k_sem_reset(&time_sync_sem);
	}
	if (current_app_state.working) {
		k_sem_give(&working_sem);
	}
	if (current_app_state.link) {
		k_sem_give(&link_sem);
	} else {
		k_sem_reset(&link_sem);
	}
}

void wait_for_registered()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.registered) {
		k_sem_take(&registered_sem, K_FOREVER);
	}
}

void wait_for_not_sending()
{
	LOG_INF("%s", __func__);
	if (current_app_state.sending) {
		while (0 != k_sem_take(&not_sending_sem, K_FOREVER)) {
		};
	}
}

void wait_for_connected()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.connected) {
		while (0 != k_sem_take(&connected_sem, K_FOREVER)) {
		};
	}
}

void wait_for_time_sync()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.time_sync) {
		while (0 != k_sem_take(&time_sync_sem, K_FOREVER)) {
		};
	}
}

void wait_for_link()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.link) {
		while (0 != k_sem_take(&link_sem, K_FOREVER)) {
		};
	}
}

static inline void perform_power_test(void)
{
	LOG_INF("%s", __func__);
	wait_for_registered();
	wait_for_time_sync();
	wait_for_not_sending();

	for (int i = 0; i < CONFIG_MESSAGES_TO_SEND; i++) {
		if (!current_app_state.connected) {
			wait_for_registered();
			wait_for_time_sync();
			app_event_send(BUTTON_EVENT_CONNECTION_REQUEST);
			wait_for_connected();
		}

		wait_for_connected();
		wait_for_not_sending();
		k_sleep(K_MSEC(CONFIG_DELAY_BETWEN_MESSAGES));
		app_event_send(BUTTON_EVENT_SEND_HELLO);
	}
}

int main(void)
{
	extern struct notifier_ctx global_state_notifier;

	if (!subscribe_for_state_change(&global_state_notifier, state_change_handler_power_test)) {
		__ASSERT(
			false,
			"failed to initialize the state watch, is the CONFIG_STATE_NOTIFIER_HANDLER_MAX too low ?");
	}

	app_setup();
	if (app_thread_init(&app_context)) {
		LOG_ERR("Failed to start Sidewalk thread");
	}

	if (!current_app_state.working) {
		k_sem_take(&working_sem, K_FOREVER);
	}

	settings_subsys_init();
	settings_register(&alph_handler);
	settings_load();

	if (factory_reset_bypass == false) {
		factory_reset_bypass = true;
		settings_save();
		settings_commit();

		LOG_INF("Perform factory reset");
		k_sleep(K_SECONDS(1));
		app_event_send(BUTTON_EVENT_FACTORY_RESET);
		k_sleep(K_FOREVER);
	}
	factory_reset_bypass = false;
	settings_save();
	settings_commit();

	perform_power_test();

	return 0;
}
