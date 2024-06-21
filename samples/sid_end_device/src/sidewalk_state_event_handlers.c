/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sidewalk.h>
#include <sid_hal_memory_ifc.h>
#include <zephyr/logging/log.h>
#include <sid_error.h>
#include <settings_utils.h>
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
#include <file_transfer.h>
#endif
#include <sidTypes2str.h>

LOG_MODULE_REGISTER(sidewalk_fsm_event_handlers, CONFIG_SIDEWALK_FSM_EVENTS_LOG_LEVEL);

void sidewalk_event_process(void *event_ctx, sm_t *sm)
{
	ARG_UNUSED(event_ctx);
	sidewalk_ctx_t *sid = sm->sid;
	sid_error_t e = SID_ERROR_NONE;
	e = sid_process(sid->handle);
	if (e) {
		LOG_ERR("sid process err %d", (int)e);
	}
}

void sidewalk_event_autoconnect(void *event_ctx, sm_t *sm)
{
	ARG_UNUSED(event_ctx);
#ifdef CONFIG_SID_END_DEVICE_AUTO_START
	sidewalk_ctx_t *sid = sm->sid;
	sid_error_t e = SID_ERROR_NONE;
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
	int err = settings_utils_link_mask_get(&sid->config.link_mask);
	if (err <= 0) {
		LOG_WRN("Link mask get failed %d", err);
		sid->config.link_mask = 0;
		settings_utils_link_mask_set(DEFAULT_LM);
	}
#endif /* CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK */

	if (!sid->config.link_mask) {
		sid->config.link_mask = DEFAULT_LM;
	}

	LOG_INF("Sidewalk link switch to %s", (SID_LINK_TYPE_3 & sid->config.link_mask) ? "LoRa" :
					      (SID_LINK_TYPE_2 & sid->config.link_mask) ? "FSK" :
											  "BLE");

	e = sid_init(&sid->config, &sid->handle);
	if (e) {
		LOG_ERR("sid init err %d", (int)e);
	}
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
	app_file_transfer_demo_init(sid->handle);
#endif
	LOG_INF("Start called");
	e = sid_start(sid->handle, sid->config.link_mask);
	if (e) {
		LOG_ERR("sid start err %d", (int)e);
	}

#if CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
	if (sid->config.link_mask & SID_LINK_TYPE_1) {
		enum sid_link_connection_policy set_policy =
			SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;

		e = sid_option(sid->handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &set_policy,
			       sizeof(set_policy));
		if (e) {
			LOG_ERR("sid option multi link manager err %d", (int)e);
		}

		struct sid_link_auto_connect_params ac_params = {
			.link_type = SID_LINK_TYPE_1,
			.enable = true,
			.priority = 0,
			.connection_attempt_timeout_seconds = 30
		};

		e = sid_option(sid->handle, SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS,
			       &ac_params, sizeof(ac_params));
		if (e) {
			LOG_ERR("sid option multi link policy err %d", (int)e);
		}
	}
#endif /* CONFIG_SID_END_DEVICE_AUTO_CONN_REQ */

#endif /* CONFIG_SIDEWALK_AUTO_START */
}

void sidewalk_event_factory_reset(void *event_ctx, sm_t *sm)
{
	ARG_UNUSED(event_ctx);
	sidewalk_ctx_t *sid = sm->sid;
	sid_error_t e = SID_ERROR_NONE;
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
	(void)settings_utils_link_mask_set(0);
#endif /* CONFIG_SIDEWALK_PERSISTENT_LINK_MASK */
	e = sid_set_factory_reset(sid->handle);
	if (e) {
		LOG_ERR("sid factory reset err %d", (int)e);
	}
}

void sidewalk_event_link_switch(void *event_ctx, sm_t *sm)
{
	ARG_UNUSED(event_ctx);
	sidewalk_ctx_t *sid = sm->sid;
	sid_error_t e = SID_ERROR_NONE;
	static uint32_t new_link_mask = DEFAULT_LM;

	switch (sid->config.link_mask) {
	case SID_LINK_TYPE_1:
		new_link_mask = SID_LINK_TYPE_2;
		break;
	case SID_LINK_TYPE_2:
		new_link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_3;
		break;
	default:
		new_link_mask = SID_LINK_TYPE_1;
		break;
	}
	sid->config.link_mask = new_link_mask;

	LOG_INF("Sidewalk link switch to %s", (SID_LINK_TYPE_3 & new_link_mask) ? "LoRa" :
					      (SID_LINK_TYPE_2 & new_link_mask) ? "FSK" :
										  "BLE");
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
	int err = settings_utils_link_mask_set(new_link_mask);
	if (err) {
		LOG_ERR("New link mask set err %d", err);
	}
#endif /* CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK */

	if (sid->handle != NULL) {
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
		app_file_transfer_demo_deinit(sid->handle);
#endif
		e = sid_deinit(sid->handle);
		if (e) {
			LOG_ERR("sid deinit err %d", (int)e);
		}
	}

	e = sid_init(&sid->config, &sid->handle);
	if (e) {
		LOG_ERR("sid init err %d", (int)e);
	}
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
	app_file_transfer_demo_init(sid->handle);
#endif
	e = sid_start(sid->handle, sid->config.link_mask);
	if (e) {
		LOG_ERR("sid start err %d", (int)e);
	}
#if CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
	if (sid->config.link_mask & SID_LINK_TYPE_1) {
		enum sid_link_connection_policy set_policy =
			SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;

		e = sid_option(sid->handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &set_policy,
			       sizeof(set_policy));
		if (e) {
			LOG_ERR("sid option multi link manager err %d", (int)e);
		}

		struct sid_link_auto_connect_params ac_params = {
			.link_type = SID_LINK_TYPE_1,
			.enable = true,
			.priority = 0,
			.connection_attempt_timeout_seconds = 30
		};

		e = sid_option(sid->handle, SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS,
			       &ac_params, sizeof(ac_params));
		if (e) {
			LOG_ERR("sid option multi link policy err %d", (int)e);
		}
	}
#endif /* CONFIG_SID_END_DEVICE_AUTO_CONN_REQ */
}

void sidewalk_event_nordic_dfu(void *event_ctx, sm_t *sm)
{
	ARG_UNUSED(event_ctx);
	sidewalk_ctx_t *sid = sm->sid;
	sid_error_t e = SID_ERROR_NONE;
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
	app_file_transfer_demo_deinit(sid->handle);
#endif
	sid_process(sid->handle);
	e = sid_deinit(sid->handle);
	if (e) {
		LOG_ERR("sid deinit err %d", (int)e);
	}
	smf_set_state(SMF_CTX(sm), &sm->sid_states[STATE_DFU]);
}

void sidewalk_event_new_status(void *event_ctx, sm_t *sm)
{
	sidewalk_ctx_t *sid = sm->sid;
	struct sid_status *p_status = (struct sid_status *)event_ctx;
	if (!p_status) {
		LOG_ERR("sid new status is NULL");
		return;
	}

	memcpy(&sid->last_status, p_status, sizeof(struct sid_status));
	sid_hal_free(p_status);
}

void sidewalk_event_send_message(void *event_ctx, sm_t *sm)
{
	sid_error_t e = SID_ERROR_NONE;
	sidewalk_ctx_t *sid = sm->sid;
	sidewalk_msg_t *p_msg = (sidewalk_msg_t *)event_ctx;
	if (!p_msg) {
		LOG_ERR("sid send msg is NULL");
		return;
	}

	e = sid_put_msg(sid->handle, &p_msg->msg, &p_msg->desc);
	if (e) {
		LOG_ERR("sid send err %d (%s)", (int)e, SID_ERROR_T_STR(e));
	}
	LOG_DBG("sid send (type: %d, id: %u)", (int)p_msg->desc.type, p_msg->desc.id);
	push_message_buffer(p_msg);
}

void sidewalk_event_connect(void *event_ctx, sm_t *sm)
{
	ARG_UNUSED(event_ctx);
	sidewalk_ctx_t *sid = sm->sid;
	sid_error_t e = SID_ERROR_NONE;
	if (!(sid->config.link_mask & SID_LINK_TYPE_1)) {
		LOG_ERR("Can not request connection - BLE not enabled");
		return;
	}
	e = sid_ble_bcn_connection_request(sid->handle, true);
	if (e) {
		LOG_ERR("sid conn req err %d (%s)", (int)e, SID_ERROR_T_STR(e));
	}
}
