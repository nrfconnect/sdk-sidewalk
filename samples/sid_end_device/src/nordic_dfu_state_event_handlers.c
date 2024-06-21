/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sidewalk.h>
#include <nordic_dfu.h>
#include <zephyr/logging/log.h>
#include <sid_hal_memory_ifc.h>

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

void sid_dfu_event_new_status_handler(void *ctx, void *state_machine)
{
	if (!ctx) {
		LOG_ERR("sid new status is NULL, expected data");
		return;
	}
	sid_hal_free(ctx);
}

void sid_dfu_event_send_message_handler(void *ctx, void *state_machine)
{
	sidewalk_msg_t *p_msg = (sidewalk_msg_t *)ctx;
	sidewalk_msg_t *message = get_message_buffer(p_msg->desc.id);

	if (message == NULL) {
		LOG_INF("Buffer already free, nothing to od");
		return;
	}
	if (message != p_msg) {
		LOG_ERR("Returned different buffer than expected");
	}

	sid_hal_free(message->msg.data);
	sid_hal_free(message);
}
