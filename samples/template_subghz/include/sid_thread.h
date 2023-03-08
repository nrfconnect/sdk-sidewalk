/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#ifndef SID_THREAD_H
#define SID_THREAD_H

#include <zephyr/kernel.h>
#include <sid_error.h>
#include <sid_api.h>

typedef struct sid_thread_ctx  {
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
 */
struct sid_config *get_sidewalk_config(void);

#define LM_2_STR(_lm)   (_lm == SID_LINK_TYPE_1 ? "BLE" :    \
			 ((_lm == SID_LINK_TYPE_2) ? "FSK" : \
			  ((_lm == SID_LINK_TYPE_3) ? "LoRa" : "INVALID")))

#endif /* SID_THREAD_H */
