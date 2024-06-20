/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sidewalk.h>
#include <nordic_dfu.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(sidewalk_fsm_event_handlers, CONFIG_SIDEWALK_FSM_EVENTS_LOG_LEVEL);

void sid_dfu_event_nordic_dfu_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	int err = -ENOTSUP;
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	err = nordic_dfu_ble_stop();
#endif
	if (err) {
		LOG_ERR("dfu stop err %d", err);
	}

	smf_set_state(SMF_CTX(sm), &sm->sid_states[STATE_SIDEWALK]);
}
