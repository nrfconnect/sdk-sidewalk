/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BUTTONS_INTERNAL_H
#define BUTTONS_INTERNAL_H

#include <stdint.h>

typedef enum {
	BUTTON_ACTION_SHORT_PRESS,
	BUTTON_ACTION_LONG_PRESS,
	BUTTON_ACTION_LAST,
} button_action_t;

/** @brief Simulate button press in software.
 *
 *  @param button_nr Pointer to bt_a2dp_endpoint structure.
 */
void button_pressed(uint8_t button_nr, button_action_t action);

#endif /* BUTTONS_INTERNAL_H */
