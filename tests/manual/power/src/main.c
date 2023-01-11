/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include "sid_pal_storage_kv_ifc.h"
#include <sidewalk_thread.h>
#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <buttons.h>
#include <zephyr/logging/log.h>

#if CONFIG_BOOTLOADER_MCUBOOT
#include <zephyr/dfu/mcuboot.h>
#endif

#include <zephyr/storage/flash_map.h>
#include <state_notifier.h>

#include <sidewalk_version.h>

#if CONFIG_SIDEWALK_DFU_SERVICE_USB
#include <zephyr/usb/usb_device.h>
#endif

LOG_MODULE_REGISTER(sid_template, CONFIG_SIDEWALK_LOG_LEVEL);

volatile struct notifier_state current_app_state = {};

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
#else
	button_set_action(DK_BTN2, sidewalk_event, EVENT_TYPE_SET_DEVICE_PROFILE);
#endif
	button_set_action(DK_BTN3, sidewalk_event, EVENT_TYPE_SEND_HELLO);
	button_set_action_short_press(DK_BTN4, sidewalk_event, EVENT_TYPE_SET_BATTERY_LEVEL);
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

static inline void erase_sidewalk_settings(void)
{
	LOG_INF("Clear sidewalk storage partition");
	const struct flash_area *fa;
	int rc = flash_area_open(FIXED_PARTITION_ID(sidewalk_storage), &fa);

	if (rc < 0) {
		LOG_ERR("failed to open flash area");
	} else {
		if (flash_area_get_device(fa) == NULL ||
		    flash_area_erase(fa, 0, fa->fa_size) < 0) {
			LOG_ERR("failed to erase flash area");
		}
		flash_area_close(fa);
	}
}

static void state_change_handler_power_test(const struct notifier_state *state)
{
	current_app_state = *state;
}

void wait_for_registered()
{
	uint32_t time_waited = 0;

	while (!current_app_state.registered) {
		k_sleep(K_MSEC(1));
		time_waited++;
	}
	LOG_INF("Device registered after %d ms", time_waited);
}

void wait_for_not_sending()
{
	uint32_t time_waited = 0;

	while (current_app_state.sending) {
		k_sleep(K_MSEC(1));
		time_waited++;
	}
}

void wait_for_connected()
{
	uint32_t time_waited = 0;

	while (!current_app_state.connected) {
		k_sleep(K_MSEC(1));
		time_waited++;
	}
}

void wait_for_time_sync()
{
	uint32_t time_waited = 0;

	while (!current_app_state.time_sync) {
		k_sleep(K_MSEC(1));
		time_waited++;
	}
	LOG_INF("Device synced after %d ms", time_waited);
}

static inline void perform_power_test(void)
{
	wait_for_registered();
	wait_for_time_sync();

	#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	sidewalk_thread_message_q_write(EVENT_TYPE_SEND_HELLO);
	wait_for_time_sync();
	wait_for_connected();
	#endif

	for (int i = 0; i < CONFIG_MESSAGES_TO_SEND; i++) {

	#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
		if (!current_app_state.connected) {
			sidewalk_thread_message_q_write(EVENT_TYPE_CONNECTION_REQUEST);
			wait_for_connected();
		}
	#endif
		wait_for_not_sending();
		sidewalk_thread_message_q_write(EVENT_TYPE_SEND_HELLO);
		k_sleep(K_MSEC(CONFIG_DELAY_BETWEN_MESSAGES));
	}
}

void main(void)
{

	LOG_INF("APPLICATION FOR POWER TESTS");
	erase_sidewalk_settings();

	extern struct notifier_ctx global_state_notifier;

	subscribe_for_state_change(&global_state_notifier, state_change_handler_power_test);

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

	perform_power_test();
}
