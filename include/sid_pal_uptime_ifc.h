#ifndef SID_PAL_UPTIME_IFC_H
#define SID_PAL_UPTIME_IFC_H

#include <sid_error.h>
#include <sid_time_types.h>

#include <stdint.h>

sid_error_t sid_pal_uptime_now(struct sid_timespec *time);
void sid_pal_uptime_set_xtal_ppm(int16_t ppm);
int16_t sid_pal_uptime_get_xtal_ppm(void);

#endif
