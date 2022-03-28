/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_pal_uptime_ifc.h>
#include <sid_pal_log_ifc.h>
#include <sys_clock.h>
#include "zephyr_time.h"

#ifdef CONFIG_CLOCK_CONTROL_NRF_ACCURACY
#define TIMER_RTC_MAX_PPM_TO_COMPENSATE CONFIG_CLOCK_CONTROL_NRF_ACCURACY
#else
#define TIMER_RTC_MAX_PPM_TO_COMPENSATE 500
#endif

sid_error_t sid_pal_uptime_now(struct sid_timespec *result)
{
	if (!result) {
		return SID_ERROR_NULL_POINTER;
	}

	uint64_t uptime_ns = zephyr_uptime_ns();
	result->tv_sec = (sid_time_t)(uptime_ns / NSEC_PER_SEC);
	result->tv_nsec = (uint32_t)(uptime_ns % NSEC_PER_SEC);

	return SID_ERROR_NONE;
}

void sid_pal_uptime_set_xtal_ppm(int16_t ppm)
{
	SID_PAL_LOG_WARNING("%s - not implemented.", __func__);
}

int16_t sid_pal_uptime_get_xtal_ppm(void)
{
	return TIMER_RTC_MAX_PPM_TO_COMPENSATE;
}
