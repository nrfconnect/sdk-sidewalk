/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_TIMER_TYPES_H
#define SID_PAL_TIMER_TYPES_H

#include <zephyr/kernel.h>
#include <zephyr/sys/dlist.h>
#include <sid_time_types.h>

typedef struct sid_pal_timer_impl_t sid_pal_timer_t;

/**
 * @brief Timer callback type
 *
 * @note The callback is allowed to execute absolute minimum amount of work and return as soon as possible
 * @note Implementer of the callback should consider the callback is executed from ISR context
 */
typedef void (*sid_pal_timer_cb_t)(void *arg, sid_pal_timer_t *originator);

/**
 * @brief Timer storage type
 *
 * @note This is the implementor defined storage type for timers.
 */
struct sid_pal_timer_impl_t {
	struct sid_timespec alarm;
	struct sid_timespec period;
	sys_dnode_t node;
	sid_pal_timer_cb_t callback;
	void *callback_arg;
	const struct sid_timespec *tolerance;
};

#endif
