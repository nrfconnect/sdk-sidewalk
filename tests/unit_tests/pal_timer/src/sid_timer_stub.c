/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_timer_stub.c
 *  @brief Stubs.
 */

#include <sid_time_types.h>
#include <sid_time_ops.h>

void sid_time_normalize(struct sid_timespec *time)
{
	if (SID_TIME_NSEC_PER_SEC > time->tv_nsec) {
		return;
	}

	while (time->tv_nsec >= SID_TIME_NSEC_PER_SEC) {
		time->tv_sec += 1;
		time->tv_nsec -= SID_TIME_NSEC_PER_SEC;
	}
}

void sid_time_add(struct sid_timespec *time_1, const struct sid_timespec *time_2)
{
	time_1->tv_sec += time_2->tv_sec;
	time_1->tv_nsec += time_2->tv_nsec;

	sid_time_normalize(time_1);
}

void sid_time_sub(struct sid_timespec *time_1, const struct sid_timespec *time_2)
{
	struct sid_timespec *tmp_time = (struct sid_timespec *)time_2;

	sid_time_normalize(tmp_time);

	if (time_1->tv_nsec < tmp_time->tv_nsec) {
		time_1->tv_sec -= 1;
		time_1->tv_nsec += SID_TIME_NSEC_PER_SEC;
	}

	time_1->tv_sec -= tmp_time->tv_sec;
	time_1->tv_nsec -= tmp_time->tv_nsec;

	sid_time_normalize(time_1);
}

bool sid_time_gt(const struct sid_timespec *time_1, const struct sid_timespec *time_2)
{
	if ((time_1->tv_sec > time_2->tv_sec) ||
	    (time_1->tv_sec == time_2->tv_sec && time_1->tv_nsec > time_2->tv_nsec)) {
		return true;
	}

	return false;
}

bool sid_time_is_infinity(const struct sid_timespec *time)
{
	if (time->tv_sec == SID_TIME_INFINITY.tv_sec &&
	    time->tv_nsec == SID_TIME_INFINITY.tv_nsec) {
		return true;
	}
	return false;
}
