/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_LEDS_H
#define APP_LEDS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(CONFIG_BOARD_THINGY53_NRF5340_CPUAPP)
#define APP_LEDS_MAX 3
#else
#define APP_LEDS_MAX 4
#endif
#define LED_ACTION_REPONSE_PAYLOAD_SIZE_MAX 32

enum leds_id_t {
	LED_ID_0 = 0,
	LED_ID_1 = 1,
	LED_ID_2 = 2,
	LED_ID_3 = 3,
	LED_ID_LAST,
};

/**
 * @brief Turns on all LEDs.
 */
void app_led_turn_on_all(void);

/**
 * @brief Turns of all LEDs.
 */
void app_led_turn_off_all(void);

/**
 * @brief Turns on LED.
 *
 * @param id - number of LED id to turn on.
 */
void app_led_turn_on(enum leds_id_t id);

/**
 * @brief Turns off LED.
 *
 * @param id - number of LED id to turn off.
 */
void app_led_turn_off(enum leds_id_t id);

/**
 * @brief Get LED state.
 *
 * @param id - number of LED id to get state.
 * @return LED state. true means on, false means off.
 */
bool app_led_is_on(enum leds_id_t id);

/**
 * @brief Get LED id array's address.
 *
 * @return pointer to LEDs id array.
 */
uint8_t *app_led_id_array_get();

/**
 * @brief Initialize LED's
 *
 * @return 0 if success, otherwise negative error code.
 */
int app_led_init();

#endif /* APP_LEDS_H */
