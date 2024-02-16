/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_BUTTONS_H
#define APP_BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

#if defined(CONFIG_BOARD_THINGY53_NRF5340_CPUAPP)
#define APP_BUTTONS_MAX 1
#else
#define APP_BUTTONS_MAX 4
#endif

enum demo_btn_id {
	DEMO_BTN_ID_0,
	DEMO_BTN_ID_1,
	DEMO_BTN_ID_2,
	DEMO_BTN_ID_3,
};

/**
 * @brief Clear button pending flag.
 *
 */
void app_btn_pending_flag_clear(void);

/**
 * @brief Get the value of pending flag.
 *
 */
bool app_btn_pending_flag_get(void);

/**
 * @brief Set bit in notify mask for button ID.
 *
 * @param btn_id button ID.
 */
void app_btn_notify_mask_bit_set(uint8_t btn_id);

/**
 * @brief This function tests whether bit assigned to button ID is set or not.
 *
 * @param btn_id button ID
 * @return true if bit is set, false otherwise.
 */
bool app_btn_notify_mask_bit_is_set(uint8_t btn_id);

/**
 * @brief Clear bit in notify mask for button ID.
 *
 * @param btn_id button ID.
 */
void app_btn_notify_mask_bit_clear(uint8_t btn_id);

/**
 * @brief Clear bit in press mask for button ID.
 *
 * @param btn_id button ID.
 */
void app_btn_press_mask_bit_clear(uint8_t btn_id);

/**
 * @brief This function tests whether bit assigned to button ID is set or not.
 *
 * @param btn_id button ID.
 * @return true if bit is set, false otherwise.
 */
bool app_btn_press_mask_bit_is_set(uint8_t btn_id);

/**
 * @brief Get address to buttons array.
 *
 * @return address to button array.
 */
uint8_t *app_btn_id_array_get(void);

/**
 * @brief Handler to set button as pending.
 *
 * @param btn_id Button ID.
 */
void app_btn_event_handler(uint32_t btn_id);

#endif /* APP_BUTTONS_H */
