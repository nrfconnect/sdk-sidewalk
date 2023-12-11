/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);

#define EVENT_BTN_PRESS BIT(0)

static void state_unregistered_run(void *o);
static void state_registered_run(void *o);
static void state_time_synced_run(void *o);
static void state_ready_run(void *o);

enum app_state { STATE_UNREGISTERED, STATE_REGISTERED, STATE_TIME_SYNCED, STATE_READY };

static const struct smf_state app_states[] = {
	[STATE_UNREGISTERED] = SMF_CREATE_STATE(NULL, state_unregistered_run, NULL),
	[STATE_REGISTERED] = SMF_CREATE_STATE(NULL, state_registered_run, NULL),
	[STATE_TIME_SYNCED] = SMF_CREATE_STATE(NULL, state_time_synced_run, NULL),
	[STATE_READY] = SMF_CREATE_STATE(NULL, state_ready_run, NULL),
};

struct s_object {
	struct smf_ctx ctx;
	struct k_event smf_event;
	int32_t events;
} s_obj;

static void state_unregistered_run(void *o)
{
	struct s_object *s = (struct s_object *)o;

	LOG_INF("state: unregistered");

	smf_set_state(SMF_CTX(s), &app_states[STATE_REGISTERED]);
}

static void state_registered_run(void *o)
{
	struct s_object *s = (struct s_object *)o;

	LOG_INF("state: registered");

	smf_set_state(SMF_CTX(s), &app_states[STATE_TIME_SYNCED]);
}

static void state_time_synced_run(void *o)
{
	struct s_object *s = (struct s_object *)o;

	LOG_INF("state: time synced");

	if (s->events & EVENT_BTN_PRESS) {
		smf_set_state(SMF_CTX(s), &app_states[STATE_READY]);
		k_event_clear(&s->smf_event, EVENT_BTN_PRESS);
	}
}

static void state_ready_run(void *o)
{
	struct s_object *s = (struct s_object *)o;

	LOG_INF("state: ready");

	if (s->events & EVENT_BTN_PRESS) {
		LOG_INF("Success!");
		k_event_clear(&s->smf_event, EVENT_BTN_PRESS);
	}
}

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	if (buttons & DK_BTN1_MSK) {
		LOG_INF("button 1");
		k_event_post(&s_obj.smf_event, EVENT_BTN_PRESS);
	}
}

int main(void)
{
	LOG_INF("Hello World! %s", CONFIG_BOARD);

	if (dk_buttons_init(button_changed)) {
		LOG_ERR("Cannot init buttons");
	}

	k_event_init(&s_obj.smf_event);
	smf_set_initial(SMF_CTX(&s_obj), &app_states[STATE_UNREGISTERED]);
	while (1) {
		s_obj.events = k_event_wait(&s_obj.smf_event, EVENT_BTN_PRESS, false, K_MSEC(1000));
		if (smf_run_state(SMF_CTX(&s_obj))) {
			break;
		}
	}

	return 0;
}
