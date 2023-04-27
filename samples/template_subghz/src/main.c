/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>
#include <sid_error.h>
#include <sid_pal_assert_ifc.h>
#include <app_ble_config.h>
#include <app_subGHz_config.h>

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

LOG_MODULE_REGISTER(main, CONFIG_SIDEWALK_LOG_LEVEL);

static app_ctx_t app_context;

static void button_handler(uint32_t event)
{
	app_event_send((app_event_t) event);
}

static sid_error_t app_buttons_init(btn_handler_t handler)
{
	button_set_action_long_press(DK_BTN1, handler, BUTTON_EVENT_FACTORY_RESET);
	button_set_action_short_press(DK_BTN2, handler, BUTTON_EVENT_GET_DEVICE_PROFILE);
	button_set_action_long_press(DK_BTN2, handler, BUTTON_EVENT_SET_DEVICE_PROFILE);
	button_set_action(DK_BTN3, handler, BUTTON_EVENT_SEND_HELLO);
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

	app_context.config = (struct sid_config) {
		.link_mask = BUILT_IN_LM,
		.time_sync_periodicity_seconds = 7200,
		.callbacks = &app_context.event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
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

int main(void)
{
	PRINT_SIDEWALK_VERSION();

	app_setup();
	if (app_thread_init(&app_context)) {
		LOG_ERR("Failed to start Sidewalk thread");
	}

	return 0;
}
