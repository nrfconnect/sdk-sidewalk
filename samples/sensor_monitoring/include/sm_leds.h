/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SM_LEDS_H
#define SM_LEDS_H

#include <sm_task.h>

/**
 * @brief The function processes a LED request.
 *
 * @param app_context application context.
 * @param state pointer to serialized request data.
 */
void sm_leds_action_request_process(app_context_t *app_context, struct sid_parse_state *state);

/**
 * @brief Get number of LED id array's cells.
 *
 * @return number of LED id array's cells = number of LEDs.
 */
size_t sm_leds_id_array_size_get();

/**
 * @brief Get LED id array's address.
 *
 * @return pointer to LEDs id array.
 */
uint8_t *sm_leds_id_array_get();

/**
 * @brief Initialize LED's
 *
 * @return 0 if success, otherwise negative error code.
 */
int sm_leds_init();

#endif /* SM_LEDS_H */
