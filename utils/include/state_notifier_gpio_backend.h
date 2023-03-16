/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef STATE_NOTIFIER_GPIO_BACKEND_H
#define STATE_NOTIFIER_GPIO_BACKEND_H

#include <state_notifier.h>

void state_watch_init_gpio(struct notifier_ctx *ctx);

#endif // STATE_NOTIFIER_GPIO_BACKEND_H
