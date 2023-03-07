/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#ifndef SID_THREAD_H
#define SID_THREAD_H

#include <zephyr/kernel.h>
#include <sid_error.h>

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



#endif /* SID_THREAD_H */
