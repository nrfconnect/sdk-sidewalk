/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_timer.c
 *  @brief Timer interface implementation.
 */

#include <sid_pal_timer_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <sid_pal_critical_region_ifc.h>
#include <sid_time_ops.h>
#include <stdint.h>
#include <zephyr.h>

struct sid_pal_timer_ctx {
	sys_dlist_t list;
};

static const struct sid_timespec tolerance_lowpower = { .tv_sec = 1, .tv_nsec = 0 };
static const struct sid_timespec tolerance_precise = { .tv_sec = 0, .tv_nsec = 0 };

static struct sid_pal_timer_ctx sid_pal_timer_ctx = { .list = SYS_DLIST_STATIC_INIT(&sid_pal_timer_ctx.list), };

static void sid_timer_start(const struct sid_timespec *sid_time);

static const struct sid_timespec *sid_pal_timer_get_tolerance(sid_pal_timer_prio_class_t type)
{
	const struct sid_timespec *tolerance = NULL;

	switch (type) {
	case SID_PAL_TIMER_PRIO_CLASS_PRECISE:
		tolerance = &tolerance_precise;
		break;

	case SID_PAL_TIMER_PRIO_CLASS_LOWPOWER:
		tolerance = &tolerance_lowpower;
		break;
	}

	SID_PAL_ASSERT(tolerance);

	return tolerance;
}

static bool sid_pal_timer_list_in_list(sid_pal_timer_t *timer)
{
	SID_PAL_ASSERT(timer);
	bool result = true;

	sid_pal_enter_critical_region();
	if ((0 == timer->node.next) && (0 == timer->node.prev)) {
		result = false;
	}
	sid_pal_exit_critical_region();

	return result;
}

static void sid_pal_timer_list_delete(sid_pal_timer_t *timer)
{
	SID_PAL_ASSERT(timer);

	sid_pal_enter_critical_region();
	if (!((0 == timer->node.next) && (0 == timer->node.prev))) {
		sys_dlist_remove(&timer->node);
	}
	sid_pal_exit_critical_region();
}

static void sid_pal_timer_list_insert(struct sid_pal_timer_ctx *ctx, sid_pal_timer_t *timer)
{
	SID_PAL_ASSERT(ctx && timer);
	bool reschedule_required = true;

	sid_pal_enter_critical_region();
	sys_dnode_t *node = sys_dlist_peek_head(&ctx->list);
	while (node) {
		sid_pal_timer_t *element = CONTAINER_OF(node, __typeof__(*element), node);
		if (sid_time_gt(&element->alarm, &timer->alarm)) {
			struct sid_timespec diff = element->alarm;
			sid_time_sub(&diff, &timer->alarm);
			if (!sid_time_gt(&diff, timer->tolerance)) {
				reschedule_required = false;
				timer->alarm = element->alarm;
			}
			sys_dlist_append(&element->node, &timer->node);
			break;
		}
		reschedule_required = false;
		node = sys_dlist_peek_next_no_check(&ctx->list, node);
	}

	if (!node) {
		sys_dlist_append(&ctx->list, &timer->node);
	}
	if (reschedule_required) {
		sid_timer_start(&timer->alarm);
	}
	sid_pal_exit_critical_region();
}

static void sid_pal_timer_list_fetch(struct sid_pal_timer_ctx *ctx,
				     const struct sid_timespec *non_gt_than,
				     sid_pal_timer_t **timer)
{
	SID_PAL_ASSERT(ctx && non_gt_than && timer);
	*timer = NULL;

	sid_pal_enter_critical_region();
	sid_pal_timer_t *result = SYS_DLIST_PEEK_HEAD_CONTAINER(&ctx->list, result, node);

	if (result && !sid_time_gt(&result->alarm, non_gt_than)) {
		*timer = result;
		sys_dlist_remove(&result->node);
	}
	sid_pal_exit_critical_region();
}

static void sid_pal_timer_list_get_next_schedule(struct sid_pal_timer_ctx *ctx, struct sid_timespec *schedule)
{
	SID_PAL_ASSERT(ctx && schedule);
	*schedule = SID_TIME_INFINITY;

	sid_pal_enter_critical_region();
	sid_pal_timer_t *result = SYS_DLIST_PEEK_HEAD_CONTAINER(&ctx->list, result, node);
	if (result) {
		*schedule = result->alarm;
	}
	sid_pal_exit_critical_region();
}

sid_error_t sid_pal_timer_init(sid_pal_timer_t *timer_storage, sid_pal_timer_cb_t event_callback,
			       void *event_callback_arg)
{
	if (!timer_storage || !event_callback) {
		return SID_ERROR_INVALID_ARGS;
	}

	timer_storage->callback = event_callback;
	timer_storage->callback_arg = event_callback_arg;
	timer_storage->alarm = SID_TIME_INFINITY;
	timer_storage->period = SID_TIME_INFINITY;
	sys_dnode_init(&timer_storage->node);

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_deinit(sid_pal_timer_t *timer_storage)
{
	if (!timer_storage) {
		return SID_ERROR_INVALID_ARGS;
	}

	sid_pal_timer_list_delete(timer_storage);
	timer_storage->callback = NULL;
	timer_storage->callback_arg = NULL;
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_arm(sid_pal_timer_t *timer_storage,
			      sid_pal_timer_prio_class_t type,
			      const struct sid_timespec *when,
			      const struct sid_timespec *period)
{
	if (!timer_storage || !when) {
		return SID_ERROR_INVALID_ARGS;
	}

	if (sid_pal_timer_is_armed(timer_storage)) {
		return SID_ERROR_INVALID_ARGS;
	}
	if (!period) {
		period = &SID_TIME_INFINITY;
	}

	timer_storage->alarm = *when;
	timer_storage->period = *period;
	timer_storage->tolerance = sid_pal_timer_get_tolerance(type);
	sid_pal_timer_list_insert(&sid_pal_timer_ctx, timer_storage);
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_cancel(sid_pal_timer_t *timer_storage)
{
	if (!timer_storage) {
		return SID_ERROR_INVALID_ARGS;
	}

	sid_pal_timer_list_delete(timer_storage);
	return SID_ERROR_NONE;
}

bool sid_pal_timer_is_armed(const sid_pal_timer_t *timer_storage)
{
	if (!timer_storage) {
		return false;
	}
	sid_pal_timer_t *timer = (sid_pal_timer_t *)timer_storage;
	return sid_pal_timer_list_in_list(timer);
}

void sid_pal_timer_event_callback(void *arg, const struct sid_timespec *now)
{
	ARG_UNUSED(arg);
	sid_pal_timer_t *timer = NULL;

	do {
		sid_pal_timer_list_fetch(&sid_pal_timer_ctx, now, &timer);
		if (!timer) {
			break;
		}
		if (!sid_time_is_infinity(&timer->period)) {
			sid_time_add(&timer->alarm, &timer->period);

			sid_pal_timer_list_insert(&sid_pal_timer_ctx, timer);
		}
		if (timer->callback) {
			timer->callback(timer->callback_arg, (sid_pal_timer_t *)timer);
		}
	} while (1);

	struct sid_timespec next_schedule;
	sid_pal_timer_list_get_next_schedule(&sid_pal_timer_ctx, &next_schedule);
	sid_timer_start(&next_schedule);
}

static void sid_timer_handler(struct k_timer *timer_data)
{
	ARG_UNUSED(timer_data);
	struct sid_timespec handle_time;
	sid_pal_uptime_now(&handle_time);
	sid_pal_timer_event_callback(NULL, &handle_time);
}

K_TIMER_DEFINE(sid_timer, sid_timer_handler, NULL);

static void sid_timer_start(const struct sid_timespec *sid_time)
{
	k_ticks_t timer_duration;

	timer_duration = (k_ticks_t)k_ns_to_ticks_ceil64(MAX((uint64_t)sid_time->tv_nsec, 0));
	timer_duration += (k_ticks_t)k_ms_to_ticks_ceil64(MAX((uint64_t)sid_time->tv_sec * MSEC_PER_SEC, 0));
	k_timer_start(&sid_timer,
		      Z_TIMEOUT_TICKS(Z_TICK_ABS(timer_duration)),
		      K_NO_WAIT);
}
