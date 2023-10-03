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
#include <zephyr/settings/settings.h>

#include <settings_utils.h>
#include <dk_buttons_and_leds.h>
#include <sidewalk_version.h>
#include <nordic_dfu.h>
#if defined(CONFIG_BOOTLOADER_MCUBOOT)
#include <zephyr/dfu/mcuboot.h>
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

#if defined(CONFIG_NORDIC_QSPI_NOR)
#include <zephyr/device.h>
#include <zephyr/pm/device.h>

#define EXTERNAL_FLASH DT_CHOSEN(nordic_pm_ext_flash)
#endif

LOG_MODULE_REGISTER(main, CONFIG_SIDEWALK_LOG_LEVEL);

static app_ctx_t app_context;

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
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
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

#if defined(CONFIG_NORDIC_QSPI_NOR)
	const struct device *const qspi_dev = DEVICE_DT_GET(EXTERNAL_FLASH);

	if (device_is_ready(qspi_dev)) {
		pm_device_action_run(qspi_dev, PM_DEVICE_ACTION_SUSPEND);
	}
#endif
}

int main(void)
{
	PRINT_SIDEWALK_VERSION();

	switch (application_to_start()) {
	case SIDEWALK_APPLICATION: {
		app_setup();
		if (app_thread_init(&app_context)) {
			LOG_ERR("Failed to start Sidewalk thread");
		}
		break;
	};
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	case DFU_APPLICATION: {
		const int ret = nordic_dfu_ble_start();
		LOG_INF("DFU service started, return value %d", ret);
		break;
	}
#endif
	default:
		LOG_ERR("Unknown application to start.");
	}

	return 0;
}
