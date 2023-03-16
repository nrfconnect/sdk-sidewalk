/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SM_TIMERS_H
#define SM_TIMERS_H

#include <zephyr/kernel.h>

#define DEMO_CAPABILITY_PERIOD_MS           5000
#define CONNECT_LINK_TYPE_1_DELAY_MS        60000
#define CONNECT_LINK_TYPE_1_INIT_DELAY_MS   5000
#define DEMO_NOTIFY_SENSOR_DATA_PERIOD_MS   15000
#define BUTTON_PRESS_CHECK_PERIOD_SECS      30
#define PROFILE_CHECK_TIMER_DELAY_MS        60000

/**
 * @brief Set time period and run timer for capability notification.
 *
 * @param delay expected delay.
 */
void sm_cap_timer_set_and_run(k_timeout_t delay);

/**
 * @brief Set time period and run timer for buttons.
 *
 * @param delay expected delay.
 * @param update_delay force update delay.
 */
void sm_btn_press_timer_set_and_run(k_timeout_t delay, bool update_delay);

/**
 * @brief Set time period and run timer for device profile.
 *
 * @param delay expected delay.
 */
void sm_device_profile_timer_set_and_run(k_timeout_t delay);

/**
 * @brief Device profile timer stop.
 *
 */
void sm_device_profile_timer_stop();

#endif /* SM_TIMERS_H */
