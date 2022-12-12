/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

#define BUTTONS_LONG_PRESS_TIMEOUT_SEC (2UL)

typedef void (*btn_handler_t)(uint32_t);

#define BUTTON_ACT_DEF(_handler, _arg) ((button_act_t){ .handler = _handler, .arg = _arg })

#define BUTTON_ACT_NONE BUTTON_ACT_DEF(NULL, 0)

/** @brief Set button short and long press with the same actions.
 *
 *  The long action press time is defined in @ref BUTTONS_LONG_PRESS_TIMEOUT_MS
 *  This function can be run either before and after @ref buttons_init 	
 *
 *  @param button_nr button number.
 *  @param handler handler for button action.
 *  @param param parameter for handler.
 *
 *  @return 0 in case of success, -ENOENT when no such button nr.
 */
int button_set_action(uint8_t button_nr, btn_handler_t handler, uint32_t param);

/** @brief Set button long press actions.
 *
 *  The long action press time is defined in @ref BUTTONS_LONG_PRESS_TIMEOUT_MS
 *  This function can be run either before and after @ref buttons_init 	
 *
 *  @param button_nr button number.
 *  @param handler handler for button action.
 *  @param param parameter for handler.
 *
 *  @return 0 in case of success, -ENOENT when no such button nr.
 */
int button_set_action_long_press(uint8_t button_nr, btn_handler_t handler, uint32_t param);

/** @brief Set button short press actions.
 *
 *  This function can be run either before and after @ref buttons_init 	
 *
 *  @param button_nr button number.
 *  @param handler handler for button action.
 *  @param param parameter for handler.
 *
 *  @return 0 in case of success, -ENOENT when no such button nr.
 */
int button_set_action_short_press(uint8_t button_nr, btn_handler_t handler, uint32_t param);

/** @brief Initialize buttons actions.
 *
 * @return 0 in case of success. Otherwise, a (negative) error code is returned.
 */
int buttons_init(void);

/** @brief Deinitialize buttons actions.
 *
 * @return 0 in case of success. Otherwise, a (negative) error code is returned.
 */
int buttons_deinit(void);

#endif /* BUTTONS_H */
