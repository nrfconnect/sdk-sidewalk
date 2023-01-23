/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <sidewalk_thread.h>
#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <buttons.h>
#include <zephyr/logging/log.h>

#if CONFIG_BOOTLOADER_MCUBOOT
#include <zephyr/dfu/mcuboot.h>
#endif

#include <sidewalk_version.h>

#if CONFIG_SIDEWALK_DFU_SERVICE_USB
#include <zephyr/usb/usb_device.h>
#endif

LOG_MODULE_REGISTER(sid_template, CONFIG_SIDEWALK_LOG_LEVEL);

void assert_post_action(const char *file, unsigned int line)
{
	LOG_ERR("Assert in %s:%d", file, line);

	k_panic();
}

void sidewalk_event(uint32_t event_nr)
{
	sidewalk_thread_message_q_write((enum event_type)event_nr);
}

static int board_init(void)
{
	button_set_action_long_press(DK_BTN1, sidewalk_event, EVENT_TYPE_FACTORY_RESET);
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	button_set_action(DK_BTN2, sidewalk_event, EVENT_TYPE_CONNECTION_REQUEST);
	button_set_action_short_press(DK_BTN4, sidewalk_event, EVENT_TYPE_SET_BATTERY_LEVEL);
#else
	button_set_action(DK_BTN2, sidewalk_event, EVENT_TYPE_SET_DEVICE_PROFILE);
	button_set_action_short_press(DK_BTN4, sidewalk_event, EVENT_TYPE_SEND_HELLO);
#endif
	button_set_action(DK_BTN3, sidewalk_event, EVENT_TYPE_SEND_HELLO);
	button_set_action_long_press(DK_BTN4, sidewalk_event, EVENT_TYPE_NORDIC_DFU);
	int err = buttons_init();

	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
		return err;
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return err;
	}

#if CONFIG_SIDEWALK_DFU_SERVICE_USB
	err = usb_enable(NULL);
	if (err != 0) {
		LOG_ERR("Failed to enable USB");
		return err;
	}
#endif

	return 0;
}

void main(void)
{
	if (0 != board_init()) {
		return;
	}

	LOG_INF("Sidewalk example started!");

	PRINT_SIDEWALK_VERSION();

	sidewalk_thread_enable();

	#if CONFIG_BOOTLOADER_MCUBOOT
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
