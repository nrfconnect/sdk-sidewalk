/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_timer.c
 *  @brief Timer interface implementation.
 */

#include <sid_pal_timer_ifc.h>
#include <stdint.h>
#include <zephyr.h>

#define TIMER_ARMED     0x01

/**
 * @brief Timer handler is executed each time the timer expires.
 *
 * @param timer_data timer object address.
 */
static void timer_handler(struct k_timer *timer_data)
{
	sid_pal_timer_t *timer = NULL;

	if (!timer_data) {
		return;
	}

	timer = (sid_pal_timer_t *)k_timer_user_data_get(timer_data);

	if (!timer) {
		return;
	}

	if (!timer->is_periodic) {
		atomic_clear(&timer->is_armed);
	}

	if (timer->callback) {
		timer->callback(timer->callback_arg, (sid_pal_timer_t *)timer);
	}
}

/**
 * @brief Convert the sidewalk time to the kernel time format.
 *
 * @param sid_time pointer to sidewalk time format.
 * @param type Priority class specifier for the timer to be armed
 * @return k_timeout_t kernel time format.
 */
static k_timeout_t convert_time(const struct sid_timespec *sid_time, sid_pal_timer_prio_class_t type)
{
	k_timeout_t time;

	if (SID_PAL_TIMER_PRIO_CLASS_LOWPOWER == type) {
		// TODO
		time.ticks = (k_ticks_t)k_ns_to_ticks_ceil64(MAX(sid_time->tv_nsec, 0));
		time.ticks += (k_ticks_t)k_ms_to_ticks_ceil64(MAX(sid_time->tv_sec * MSEC_PER_SEC, 0));
	} else {
		time.ticks = (k_ticks_t)k_ns_to_ticks_ceil64(MAX(sid_time->tv_nsec, 0));
		time.ticks += (k_ticks_t)k_ms_to_ticks_ceil64(MAX(sid_time->tv_sec * MSEC_PER_SEC, 0));
	}
	return time;
}

sid_error_t sid_pal_timer_init(sid_pal_timer_t *timer_storage,
			       sid_pal_timer_cb_t event_callback, void *event_callback_arg)
{
	if (!timer_storage || !event_callback) {
		return SID_ERROR_NULL_POINTER;
	}

	if (timer_storage->is_initialized) {
		return SID_ERROR_NONE;
	}

	timer_storage->callback = event_callback;
	timer_storage->callback_arg = event_callback_arg;
	timer_storage->is_armed = ATOMIC_INIT(0);
	timer_storage->is_periodic = false;
	timer_storage->is_initialized = true;

	k_timer_init(&timer_storage->timer, timer_handler, NULL);
	k_timer_user_data_set(&timer_storage->timer, (void *)timer_storage);

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_deinit(sid_pal_timer_t *timer_storage)
{
	sid_error_t erc = SID_ERROR_NONE;

	if (!timer_storage) {
		return SID_ERROR_NULL_POINTER;
	}

	erc = sid_pal_timer_cancel(timer_storage);
	timer_storage->is_initialized = false;
	timer_storage->callback = NULL;
	timer_storage->callback_arg = NULL;

	return erc;
}

sid_error_t sid_pal_timer_arm(sid_pal_timer_t *timer_storage,
			      sid_pal_timer_prio_class_t type,
			      const struct sid_timespec *when,
			      const struct sid_timespec *period)
{
	k_timeout_t zero_time = K_NSEC(0);
	k_timeout_t timer_duration;
	k_timeout_t timer_period;

	if (!timer_storage || !when) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!timer_storage->is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	if ((SID_PAL_TIMER_PRIO_CLASS_LOWPOWER < type) ||
	    (SID_PAL_TIMER_PRIO_CLASS_PRECISE > type)) {
		return SID_ERROR_PARAM_OUT_OF_RANGE;
	}

	if (sid_pal_timer_is_armed(timer_storage)) {
		return SID_ERROR_INVALID_ARGS;
	}

	if (!period) {
		timer_period = K_FOREVER;
	} else {
		timer_period = convert_time(period, type);
		if (!K_TIMEOUT_EQ(timer_period, zero_time)) {
			timer_storage->is_periodic = true;
		}
	}

	timer_duration = convert_time(when, type);
	atomic_set(&timer_storage->is_armed, TIMER_ARMED);
	k_timer_start(&timer_storage->timer, timer_duration, timer_period);

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_cancel(sid_pal_timer_t *timer_storage)
{
	if (!timer_storage) {
		return SID_ERROR_NULL_POINTER;
	}

	if (!timer_storage->is_initialized) {
		return SID_ERROR_UNINITIALIZED;
	}

	k_timer_stop(&timer_storage->timer);
	atomic_clear(&timer_storage->is_armed);
	timer_storage->is_periodic = false;

	return SID_ERROR_NONE;
}

bool sid_pal_timer_is_armed(const sid_pal_timer_t *timer_storage)
{
	if (!timer_storage || !timer_storage->is_initialized) {
		return false;
	}

	return (TIMER_ARMED == atomic_get(&timer_storage->is_armed));
}
