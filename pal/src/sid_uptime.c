#include <sid_pal_uptime_ifc.h>
#include <kernel.h>

#include <sid_pal_log_ifc.h>

#ifdef CONFIG_CLOCK_CONTROL_NRF_ACCURACY
#define TIMER_RTC_MAX_PPM_TO_COMPENSATE CONFIG_CLOCK_CONTROL_NRF_ACCURACY
#else
#define TIMER_RTC_MAX_PPM_TO_COMPENSATE 500
#endif

static int64_t zephyr_uptime_ns(void){
	int64_t uptime_ticks;
	uptime_ticks = k_uptime_ticks();
	return k_ticks_to_ns_floor64(uptime_ticks);
}

sid_error_t sid_pal_uptime_now(struct sid_timespec *result)
{
	if (!result) {
		return SID_ERROR_NULL_POINTER;
	}

	int64_t uptime_ns = zephyr_uptime_ns();
	result->tv_sec = (sid_time_t)(uptime_ns / NSEC_PER_SEC);
	result->tv_nsec = (uint32_t)uptime_ns;

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
