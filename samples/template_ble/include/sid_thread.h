/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#ifndef SID_THREAD_H
#define SID_THREAD_H

#include <zephyr/kernel.h>
#include <sid_api.h>
#include <sid_error.h>

typedef struct sid_thread_ctx {
	struct k_work_q sidewalk_work_q;
	struct sid_handle *sidewalk_handle;
	struct sid_config sidewalk_config;
	struct k_work sidewalk_event_work;
} sid_thread_ctx_t;

/**
 * @brief Start Sidewalk work queue thread
 * 
 * @return struct k_work_q* work queue dedicated for Sidewalk calls
 */
struct k_work_q* sid_thread_init(void);

/**
 * @brief Get Sidewalk handler
 * 
 */
struct sid_handle** get_sidewalk_handle(void);

/**
 * @brief Get Sidewalk configuration
 * 
 * @return Configuration pointer or NULL in case of fail
 */
struct sid_config *get_sidewalk_config(void);

#endif /* SID_THREAD_H */
