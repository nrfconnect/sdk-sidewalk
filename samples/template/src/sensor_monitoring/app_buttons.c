/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sensor_monitoring/app_buttons.h>
#include <zephyr/sys/atomic.h>

static uint8_t button_id_arr[] = { 0, 1, 2, 3 };
static atomic_t button_press_mask = ATOMIC_INIT(0);
static uint32_t button_notify_mask;
static uint32_t button_press_time_in_sec_id_arr[APP_BUTTONS_MAX];
static atomic_t button_event_pending_processing = ATOMIC_INIT(false);

void app_btn_event_handler(uint32_t btn_id)
{
	if (btn_id >= APP_BUTTONS_MAX) {
		return;
	}

	if (!atomic_test_bit(&button_press_mask, button_id_arr[btn_id])) {
		atomic_set_bit(&button_press_mask, button_id_arr[btn_id]);
	}

	atomic_set(&button_event_pending_processing, true);
}

void app_btn_pending_flag_clear(void)
{
	atomic_clear(&button_event_pending_processing);
}

bool app_btn_pending_flag_get(void)
{
	return (bool)atomic_get(&button_event_pending_processing);
}

void app_btn_notify_mask_bit_set(uint8_t btn_id)
{
	if (btn_id < APP_BUTTONS_MAX) {
		WRITE_BIT(button_notify_mask, button_id_arr[btn_id], true);
	}
}

void app_btn_notify_mask_bit_clear(uint8_t btn_id)
{
	if (btn_id < APP_BUTTONS_MAX) {
		WRITE_BIT(button_notify_mask, button_id_arr[btn_id], false);
	}
}

bool app_btn_notify_mask_bit_is_set(uint8_t btn_id)
{
	if (btn_id < APP_BUTTONS_MAX) {
		return (button_notify_mask & BIT(button_id_arr[btn_id]));
	}
	return false;
}

void app_btn_press_mask_bit_clear(uint8_t btn_id)
{
	if (btn_id < APP_BUTTONS_MAX) {
		atomic_clear_bit(&button_press_mask, button_id_arr[btn_id]);
	}
}

bool app_btn_press_mask_bit_is_set(uint8_t btn_id)
{
	if (btn_id < APP_BUTTONS_MAX) {
		return atomic_test_bit(&button_press_mask, button_id_arr[btn_id]);
	}
	return false;
}

uint8_t *app_btn_id_array_get(void)
{
	return button_id_arr;
}

void demo_btn_press_time_set(uint8_t btn_id, uint32_t sec)
{
	if (btn_id < APP_BUTTONS_MAX) {
		button_press_time_in_sec_id_arr[btn_id] = sec;
	}
}

uint32_t demo_btn_press_time_get(uint8_t btn_id)
{
	if (btn_id < APP_BUTTONS_MAX) {
		return button_press_time_in_sec_id_arr[btn_id];
	}
	return 0;
}
