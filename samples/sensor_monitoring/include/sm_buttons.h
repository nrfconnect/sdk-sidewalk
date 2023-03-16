/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SM_BUTTONS_H
#define SM_BUTTONS_H

#include <sm_task.h>

#define DEMO_BUTTONS_MAX        4

/**
 * @brief Action response process.
 *
 * @param state pointer to parse state.
 */
void sm_buttons_action_response_process(struct sid_parse_state *state);

/**
 * @brief Clear button pending flag.
 *
 */
void sm_buttons_pendig_flag_clear();

/**
 * @brief Set bit in notify mask for button ID.
 *
 * @param btn_id button ID.
 */
void sm_buttons_notify_mask_bit_set(uint8_t btn_id);

/**
 * @brief This function tests whether bit assigned to button ID is set or not.
 *
 * @param btn_id button ID
 * @return true if bit is set, false otherwise.
 */
bool sm_buttons_notify_mask_bit_is_set(uint8_t btn_id);

/**
 * @brief Button notify mask get.
 *
 * @return notify mask.
 */
uint32_t sm_buttons_notify_mask_get();

/**
 * @brief Clear bit in notify mask for button ID.
 *
 * @param btn_id button ID.
 */
void sm_buttons_notify_mask_bit_clear(uint8_t btn_id);

/**
 * @brief Clear bit in press mask for button ID.
 *
 * @param btn_id button ID.
 */
void sm_buttons_press_mask_bit_clear(uint8_t btn_id);

/**
 * @brief This function tests whether bit assigned to button ID is set or not.
 *
 * @param btn_id button ID.
 * @return true if bit is set, false otherwise.
 */
bool sm_buttons_press_mask_bit_is_set(uint8_t btn_id);

/**
 * @brief Get address to buttons array.
 *
 * @return address to button array.
 */
uint8_t *sm_buttons_id_array_get();

/**
 * @brief Set button press time.
 *
 * @param btn_id button ID.
 * @param sec time in seconds.
 */
void sm_buttons_press_time_set(uint8_t btn_id, sid_time_t sec);

/**
 * @brief Get button press time.
 *
 * @param btn_id Button ID.
 * @return time in seconds.
 */
sid_time_t sm_buttons_press_time_get(uint8_t btn_id);

/**
 * @brief Configure and initialize buttons.
 *
 * @return 0 if success, otherwise negative error code.
 */
int sm_buttons_init();

#endif /* SM_BUTTONS_H */
