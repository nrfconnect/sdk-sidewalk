/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef SID_THREAD_H
#define SID_THREAD_H

#include <zephyr/kernel.h>
#include <sid_error.h>

#define SIDEWALK_DUT_WORK_Q_STACK_SIZE KB(10)

struct app_context {
	struct sid_handle **sidewalk_handle;
	struct k_work sidewalk_event_work;
};

sid_error_t sid_thread_init(void);
struct sid_config *sid_thread_get_config();
struct app_context *sid_thread_get_app_context();

#endif /* SID_THREAD_H */
