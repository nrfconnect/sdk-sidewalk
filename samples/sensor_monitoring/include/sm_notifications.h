/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SM_NOTIFICATIONS_H
#define SM_NOTIFICATIONS_H

#include <sm_task.h>

/**
 * @brief Send capability notification.
 *
 * @param app_context application context.
 */
void sm_notify_capability(app_context_t *app_context);

/**
 * @brief Send notification from sensors.
 *
 * @param app_context application context.
 * @param button_pressed flag inform is notification comes from button event.
 */
void sm_notify_sensor_data(app_context_t *app_context, bool button_pressed);

/**
 * @brief Check button press notification.
 *
 * @param app_context application context.
 */
void sm_check_button_press_notify(app_context_t *app_context);

#endif /* SM_NOTIFICATIONS_H */
