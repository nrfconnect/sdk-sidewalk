/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include "sid_api.h"
#include "sid_error.h"
#include <sidewalk_thread.h>
#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <buttons.h>
#include <zephyr/logging/log.h>

#if CONFIG_BOOTLOADER_MCUBOOT
#include <zephyr/dfu/mcuboot.h>
#endif

#include <state_notifier.h>

#include <sidewalk_version.h>

#if CONFIG_SIDEWALK_DFU_SERVICE_USB
#include <zephyr/usb/usb_device.h>
#endif

#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(sid_template, CONFIG_SIDEWALK_LOG_LEVEL);

bool factory_reset_bypass = false;

int alpha_handle_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg);
int alpha_handle_export(int (*cb)(const char *name, const void *value, size_t val_len));

struct settings_handler alph_handler = {
	.name = "test",
	.h_get = NULL,
	.h_set = alpha_handle_set,
	.h_commit = NULL,
	.h_export = alpha_handle_export
};

K_SEM_DEFINE(registered_sem, 0, 1);
K_SEM_DEFINE(not_sending_sem, 0, 1);
K_SEM_DEFINE(working_sem, 0, 1);
K_SEM_DEFINE(connected_sem, 0, 1);
K_SEM_DEFINE(time_sync_sem, 0, 1);
K_SEM_DEFINE(link_sem, 0, 1);

volatile struct notifier_state current_app_state = {};

int alpha_handle_set(const char *name, size_t len, settings_read_cb read_cb,
		     void *cb_arg)
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

int alpha_handle_export(int (*cb)(const char *name,
				  const void *value, size_t val_len))
{
	(void)cb("test/factory_reset_bypass", &factory_reset_bypass, sizeof(factory_reset_bypass));
	return 0;
}

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

static void state_change_handler_power_test(const struct notifier_state *state)
{
	current_app_state = *state;
	if (current_app_state.registered) {
		k_sem_give(&registered_sem);
	}
	if (current_app_state.sending == 0) {
		k_sem_give(&not_sending_sem);
	}
	if (current_app_state.connected) {
		k_sem_give(&connected_sem);
	}
	if (current_app_state.time_sync) {
		k_sem_give(&time_sync_sem);
	}
	if (current_app_state.working) {
		k_sem_give(&working_sem);
	}
	if (current_app_state.link) {
		k_sem_give(&link_sem);
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
		k_sem_take(&not_sending_sem, K_FOREVER);
	}
}

void wait_for_connected()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.connected) {
		k_sem_take(&connected_sem, K_FOREVER);
	}
}

void wait_for_time_sync()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.time_sync) {
		k_sem_take(&time_sync_sem, K_FOREVER);
	}
}

void wait_for_link()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.link) {
		k_sem_take(&link_sem, K_FOREVER);
	}
}

static inline void perform_power_test(void)
{
	LOG_INF("%s", __func__);
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
			wait_for_registered();
			wait_for_time_sync();
			sidewalk_thread_message_q_write(EVENT_TYPE_CONNECTION_REQUEST);
			wait_for_connected();
		}
	#endif
		wait_for_connected();
		wait_for_not_sending();
		sidewalk_thread_message_q_write(EVENT_TYPE_SEND_HELLO);
		k_sleep(K_MSEC(CONFIG_DELAY_BETWEN_MESSAGES));
	}
}

void main(void)
{
	extern struct notifier_ctx global_state_notifier;

	if(!subscribe_for_state_change(&global_state_notifier, state_change_handler_power_test))
	{
		__ASSERT(false, "failed to initialize the state watch, is the CONFIG_STATE_NOTIFIER_HANDLER_MAX too low ?");
	}

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
		sidewalk_thread_message_q_write(EVENT_TYPE_FACTORY_RESET);
		k_sleep(K_FOREVER);
	}
	factory_reset_bypass = false;
	settings_save();
	settings_commit();

	perform_power_test();
}
