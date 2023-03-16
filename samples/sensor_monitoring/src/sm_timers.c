/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sm_timers.h>
#include <sm_events.h>
#include <sm_states.h>
#include <sm_utils.h>

static void cap_timer_cb(struct k_timer *timer_id);
static void btn_press_timer_cb(struct k_timer *timer_id);
static void device_profile_timer_cb(struct k_timer *timer_id);

K_TIMER_DEFINE(cap_timer, cap_timer_cb, NULL);
K_TIMER_DEFINE(btn_press_timer, btn_press_timer_cb, NULL);
K_TIMER_DEFINE(device_profile_timer, device_profile_timer_cb, NULL);

static void cap_timer_cb(struct k_timer *timer_id)
{
	ARG_UNUSED(timer_id);
	k_timeout_t delay = K_MSEC(DEMO_CAPABILITY_PERIOD_MS);
	enum event_type event = EVENT_NOTIFICATION_TIMER_FIRED;

	if (BUILT_IN_LM == SID_LINK_TYPE_1) {
		if (!sm_is_sidewalk_ready()) {
			delay = K_MSEC(CONNECT_LINK_TYPE_1_DELAY_MS);
			event = EVENT_CONNECT_LINK_TYPE_1;
		}
	}

	if (sm_app_state_get() == DEMO_APP_STATE_NOTIFY_SENSOR_DATA &&
	    sm_is_sidewalk_ready()) {
		delay = K_MSEC(DEMO_NOTIFY_SENSOR_DATA_PERIOD_MS);
	}
	k_timer_start(&cap_timer, delay, Z_TIMEOUT_NO_WAIT);

	sm_main_task_msg_q_write(event);
}

static void btn_press_timer_cb(struct k_timer *timer_id)
{
	ARG_UNUSED(timer_id);
	sm_main_task_msg_q_write(EVENT_BUTTON_PRESS_TIMER_FIRED);
}

static void device_profile_timer_cb(struct k_timer *timer_id)
{
	ARG_UNUSED(timer_id);
	sm_main_task_msg_q_write(EVENT_SET_DEVICE_PROFILE_LINK_TYPE_2);
}

void sm_cap_timer_set_and_run(k_timeout_t delay)
{
	k_timer_start(&cap_timer, delay, Z_TIMEOUT_NO_WAIT);
}

void sm_btn_press_timer_set_and_run(k_timeout_t delay, bool update_delay)
{
	if (!update_delay && (k_timer_remaining_get(&btn_press_timer) > 0)) {
		return;
	}
	k_timer_start(&btn_press_timer, delay, Z_TIMEOUT_NO_WAIT);
}

void sm_device_profile_timer_set_and_run(k_timeout_t delay)
{
	k_timer_start(&device_profile_timer, delay, Z_TIMEOUT_NO_WAIT);
}

void sm_device_profile_timer_stop()
{
	k_timer_stop(&device_profile_timer);
}
