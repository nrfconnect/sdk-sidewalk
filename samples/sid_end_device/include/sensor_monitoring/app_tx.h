/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_TX_H
#define APP_TX_H

#include <stdint.h>
#include <zephyr/smf.h>

enum app_tx_fsm_states {
	STATE_APP_INIT,
	STATE_APP_NOTIFY_CAPABILITY,
	STATE_APP_NOTIFY_DATA,

	STATE_APP_LAST
};

typedef struct app_sm_s app_sm_t;

typedef void (*app_tx_event_handler)(app_sm_t *sm);
typedef struct app_tx_event_s {
	const char *name;
	app_tx_event_handler call[STATE_APP_LAST];
} app_tx_event_t;

typedef struct app_sm_s {
	struct smf_ctx ctx;
	struct k_msgq msgq;
	app_tx_event_t event;
	struct smf_state app_states[STATE_APP_LAST];
} app_sm_t;

#define APP_TX_EVENT(event_name, event_handlers...)                                                \
	(app_tx_event_t)                                                                           \
	{                                                                                          \
		.name = #event_name, .call = event_handlers                                        \
	}

void init_event_time_sync_success(app_sm_t *sm);

void notify_capability_event_capability_success(app_sm_t *sm);
void notify_capability_event_notify_sensor(app_sm_t *sm);
void notify_capability_not_supported(app_sm_t *sm);

void notify_data_event_time_sync_fail(app_sm_t *sm);
void notify_data_event_notify_sensor(app_sm_t *sm);
void notify_data_event_notify_button(app_sm_t *sm);
void notify_data_event_resp_led_on(app_sm_t *sm);
void notify_data_event_resp_led_off(app_sm_t *sm);

#define APP_EVENT_TIME_SYNC_SUCCESS                                                                \
	APP_TX_EVENT(APP_EVENT_TIME_SYNC_SUCCESS,                                                  \
		     {                                                                             \
			     [STATE_APP_INIT] = init_event_time_sync_success,                      \
		     })
#define APP_EVENT_TIME_SYNC_FAIL                                                                   \
	APP_TX_EVENT(APP_EVENT_TIME_SYNC_FAIL,                                                     \
		     { [STATE_APP_NOTIFY_DATA] = notify_data_event_time_sync_fail })
#define APP_EVENT_CAPABILITY_SUCCESS                                                               \
	APP_TX_EVENT(APP_EVENT_CAPABILITY_SUCCESS,                                                 \
		     { [STATE_APP_NOTIFY_CAPABILITY] =                                             \
			       notify_capability_event_capability_success })
#define APP_EVENT_NOTIFY_SENSOR                                                                    \
	APP_TX_EVENT(APP_EVENT_NOTIFY_SENSOR,                                                      \
		     { [STATE_APP_NOTIFY_CAPABILITY] = notify_capability_event_notify_sensor,      \
		       [STATE_APP_NOTIFY_DATA] = notify_data_event_notify_sensor })
#define APP_EVENT_NOTIFY_BUTTON                                                                    \
	APP_TX_EVENT(APP_EVENT_NOTIFY_BUTTON,                                                      \
		     { [STATE_APP_NOTIFY_CAPABILITY] = notify_capability_not_supported,            \
		       [STATE_APP_NOTIFY_DATA] = notify_data_event_notify_button })
#define APP_EVENT_RESP_LED_ON                                                                      \
	APP_TX_EVENT(APP_EVENT_RESP_LED_ON,                                                        \
		     { [STATE_APP_NOTIFY_CAPABILITY] = notify_capability_not_supported,            \
		       [STATE_APP_NOTIFY_DATA] = notify_data_event_resp_led_on })
#define APP_EVENT_RESP_LED_OFF                                                                     \
	APP_TX_EVENT(APP_EVENT_RESP_LED_OFF,                                                       \
		     { [STATE_APP_NOTIFY_CAPABILITY] = notify_capability_not_supported,            \
		       [STATE_APP_NOTIFY_DATA] = notify_data_event_resp_led_off })

int app_tx_event_send(app_tx_event_t event);

void app_tx_last_link_mask_set(uint32_t link_mask);

void app_tx_task(void *dummy1, void *dummy2, void *dummy3);

#endif /* APP_TX_H */
