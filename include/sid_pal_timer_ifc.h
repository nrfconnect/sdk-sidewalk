#ifndef SID_PAL_TIMER_IFC_H
#define SID_PAL_TIMER_IFC_H

#include <sid_error.h>
#include <sid_pal_timer_types.h>
#include <sid_time_types.h>

#include <stdbool.h>

typedef enum {
	SID_PAL_TIMER_PRIO_CLASS_PRECISE,
	SID_PAL_TIMER_PRIO_CLASS_LOWPOWER,
} sid_pal_timer_prio_class_t;


sid_error_t sid_pal_timer_init(sid_pal_timer_t *timer, sid_pal_timer_cb_t event_callback, void *event_callback_arg);
sid_error_t sid_pal_timer_deinit(sid_pal_timer_t *timer);
sid_error_t sid_pal_timer_arm(sid_pal_timer_t *timer, sid_pal_timer_prio_class_t type,
			      const struct sid_timespec *when, const struct sid_timespec *period);
sid_error_t sid_pal_timer_cancel(sid_pal_timer_t *timer);
bool sid_pal_timer_is_armed(const sid_pal_timer_t *timer);
sid_error_t sid_pal_timer_facility_init(void *arg);
void sid_pal_timer_event_callback(void *arg, const struct sid_timespec *now);

#endif
