#ifndef SID_PAL_TIMER_TYPES_H
#define SID_PAL_TIMER_TYPES_H

#include <zephyr.h>

typedef struct sid_pal_timer_impl_t sid_pal_timer_t;

typedef void(*sid_pal_timer_cb_t)(void * arg, sid_pal_timer_t * originator);

struct sid_pal_timer_impl_t {
    struct k_timer timer;
    sid_pal_timer_cb_t callback;
    void * callback_arg;
    atomic_t is_armed;
    bool is_periodic;
    bool is_initialized;
};

#endif
