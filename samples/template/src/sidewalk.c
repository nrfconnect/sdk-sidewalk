/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app_mfg_config.h>
#include <sid_pal_common_ifc.h>
#include <sidewalk.h>
#include <nordic_dfu.h>
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/logging/log.h>
#if !defined(CONFIG_APP_BLE_ONLY)
#include <app_subGHz_config.h>
#endif

#ifdef CONFIG_SIDEWALK_LINK_MASK_BLE
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1)
#elif CONFIG_SIDEWALK_LINK_MASK_FSK
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_2)
#elif CONFIG_SIDEWALK_LINK_MASK_LORA
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1 | SID_LINK_TYPE_3)
#else
#define DEFAULT_LM (uint32_t)(SID_LINK_TYPE_1)
#endif

LOG_MODULE_REGISTER(sidewalk_app, CONFIG_SIDEWALK_LOG_LEVEL);

static struct k_thread sid_thread;
K_THREAD_STACK_DEFINE(sid_thread_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);

typedef struct sm_s {
	struct smf_ctx ctx;
	struct k_msgq msgq;
	sidewalk_event_t event;
	sidewalk_ctx_t *sid;
} sm_t;

enum state {
	STATE_SIDEWALK_COMMON,
	STATE_SIDEWALK_INIT,
	STATE_SIDEWALK_NOT_READY,
	STATE_SIDEWALK_READY,
	STATE_DFU,
};

static void state_common_run(void *o);
static void state_init_entry(void *o);
static void state_init_run(void *o);
static void state_ready_run(void *o);
static void state_ready_entry(void *o);
static void state_not_ready_run(void *o);
static void state_dfu_entry(void *o);
static void state_dfu_run(void *o);

static const struct smf_state sid_states[] = {
	[STATE_SIDEWALK_COMMON] = SMF_CREATE_STATE(NULL, state_common_run, NULL, NULL),
	[STATE_SIDEWALK_INIT] = SMF_CREATE_STATE(state_init_entry, state_init_run, NULL,
						 &sid_states[STATE_SIDEWALK_COMMON]),
	[STATE_SIDEWALK_READY] = SMF_CREATE_STATE(state_ready_entry, state_ready_run, NULL,
						  &sid_states[STATE_SIDEWALK_COMMON]),
	[STATE_SIDEWALK_NOT_READY] = SMF_CREATE_STATE(NULL, state_not_ready_run, NULL,
						      &sid_states[STATE_SIDEWALK_COMMON]),
	[STATE_DFU] = SMF_CREATE_STATE(state_dfu_entry, state_dfu_run, NULL, NULL),
};

static uint8_t
	__aligned(4) sid_msgq_buff[CONFIG_SIDEWALK_THREAD_QUEUE_SIZE * sizeof(sidewalk_event_t)];
static sm_t sid_sm;
static sidewalk_msg_t send_msg_buffer;
static atomic_t send_msg_buffer_busy = ATOMIC_INIT(false);
static bool send_msg_pending;

static void state_common_run(void *o)
{
	sm_t *sm = (sm_t *)o;
	sid_error_t e = SID_ERROR_NONE;

	switch (sm->event) {
	case SID_EVENT_SIDEWALK:
		e = sid_process(sm->sid->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
		break;
	case SID_EVENT_STATE_ERROR:
		LOG_ERR("sid state err %d", (int)sid_get_error(sm->sid->handle));
		break;
	case SID_EVENT_FACTORY_RESET:
		e = sid_set_factory_reset(sm->sid->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
		break;
	case SID_EVENT_LINK_SWITCH:
		static uint32_t new_link_mask = DEFAULT_LM;

		switch (sm->sid->config.link_mask) {
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
		sm->sid->config.link_mask = new_link_mask;

		LOG_INF("Sidewalk link switch to %s", (SID_LINK_TYPE_3 & new_link_mask) ? "LoRa" :
						      (SID_LINK_TYPE_2 & new_link_mask) ? "FSK" :
											  "BLE");

		if (sm->sid->handle != NULL) {
			e = sid_deinit(sm->sid->handle);
			if (e) {
				LOG_ERR("sid deinit err %d", (int)e);
			}
		}

		e = sid_init(&sm->sid->config, &sm->sid->handle);
		if (e) {
			LOG_ERR("sid init err %d", (int)e);
		}

		e = sid_start(sm->sid->handle, sm->sid->config.link_mask);
		if (e) {
			LOG_ERR("sid start err %d", (int)e);
		}
		break;
	case SID_EVENT_NORDIC_DFU:
		e = sid_deinit(sm->sid->handle);
		if (e) {
			LOG_ERR("sid deinit err %d", (int)e);
		}
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_DFU]);
		break;
	case SID_EVENT_STATE_READY:
	case SID_EVENT_STATE_NOT_READY:
	case SID_EVENT_SEND_MSG:
	case SID_EVENT_CONNECT:
		/* handled in child states */
		break;
	}
}

static void state_init_entry(void *o)
{
	sm_t *sm = (sm_t *)o;
	sid_error_t e = SID_ERROR_NONE;

	if (!sm->sid->config.link_mask) {
		sm->sid->config.link_mask = DEFAULT_LM;
	}

	LOG_INF("Sidewalk link switch to %s",
		(SID_LINK_TYPE_3 & sm->sid->config.link_mask) ? "LoRa" :
		(SID_LINK_TYPE_2 & sm->sid->config.link_mask) ? "FSK" :
								"BLE");

	platform_parameters_t platform_parameters = {
		.mfg_store_region.addr_start = APP_MFG_CFG_FLASH_START,
		.mfg_store_region.addr_end = APP_MFG_CFG_FLASH_END,
#if !defined(CONFIG_APP_BLE_ONLY)
		.platform_init_parameters.radio_cfg =
			(radio_sx126x_device_config_t *)get_radio_cfg(),
#endif
	};

	e = sid_platform_init(&platform_parameters);
	if (SID_ERROR_NONE != e) {
		LOG_ERR("Sidewalk Platform Init err: %d", e);
		return;
	}

	if (app_mfg_cfg_is_valid()) {
		LOG_ERR("The mfg.hex version mismatch");
		LOG_ERR("Check if the file has been generated and flashed properly");
		LOG_ERR("START ADDRESS: 0x%08x", APP_MFG_CFG_FLASH_START);
		LOG_ERR("SIZE: 0x%08x", APP_MFG_CFG_FLASH_SIZE);
		return;
	}

	e = sid_init(&sm->sid->config, &sm->sid->handle);
	if (e) {
		LOG_ERR("sid init err %d", (int)e);
	}
	e = sid_start(sm->sid->handle, sm->sid->config.link_mask);
	if (e) {
		LOG_ERR("sid start err %d", (int)e);
	}
}

static void state_init_run(void *o)
{
	sm_t *sm = (sm_t *)o;

	switch (sm->event) {
	case SID_EVENT_STATE_READY:
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK_READY]);
		break;
	case SID_EVENT_STATE_NOT_READY:
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK_NOT_READY]);
		break;
	case SID_EVENT_CONNECT:
	case SID_EVENT_SEND_MSG:
		LOG_INF("Operation not supported, Sidewalk not initialized");
		break;
	case SID_EVENT_SIDEWALK:
	case SID_EVENT_STATE_ERROR:
	case SID_EVENT_FACTORY_RESET:
	case SID_EVENT_LINK_SWITCH:
	case SID_EVENT_NORDIC_DFU:
		/* handled in common state */
		break;
	}
}

static void state_not_ready_run(void *o)
{
	sm_t *sm = (sm_t *)o;

	switch (sm->event) {
	case SID_EVENT_STATE_READY:
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK_READY]);
		break;
	case SID_EVENT_SEND_MSG:
		send_msg_pending = true;
	case SID_EVENT_CONNECT:
		sid_error_t e = sid_ble_bcn_connection_request(sm->sid->handle, true);
		if (e) {
			LOG_ERR("conn req err %d", (int)e);
		}
		break;
	case SID_EVENT_STATE_NOT_READY:
		break;
	case SID_EVENT_SIDEWALK:
	case SID_EVENT_STATE_ERROR:
	case SID_EVENT_FACTORY_RESET:
	case SID_EVENT_LINK_SWITCH:
	case SID_EVENT_NORDIC_DFU:
		/* handled in common state */
		break;
	}
}

static void state_ready_entry(void *o)
{
	if (send_msg_pending) {
		send_msg_pending = false;
		sidewalk_event_send(SID_EVENT_SEND_MSG);
	}
}

static void state_ready_run(void *o)
{
	sm_t *sm = (sm_t *)o;
	sid_error_t e = SID_ERROR_NONE;

	switch (sm->event) {
	case SID_EVENT_STATE_NOT_READY:
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK_NOT_READY]);
		break;
	case SID_EVENT_SEND_MSG:
		e = sid_put_msg(sm->sid->handle, &send_msg_buffer.msg, &send_msg_buffer.desc);
		(void)atomic_clear(&send_msg_buffer_busy);
		if (e) {
			LOG_ERR("sid send err %d", (int)e);
		}
		LOG_INF("sid send (type: %d, id: %u)", (int)send_msg_buffer.desc.type,
			send_msg_buffer.desc.id);
		break;
	case SID_EVENT_STATE_READY:
	case SID_EVENT_CONNECT:
		break;
	case SID_EVENT_SIDEWALK:
	case SID_EVENT_STATE_ERROR:
	case SID_EVENT_FACTORY_RESET:
	case SID_EVENT_LINK_SWITCH:
	case SID_EVENT_NORDIC_DFU:
		/* handled in common state */
		break;
	}
}

static void state_dfu_entry(void *o)
{
	sm_t *sm = (sm_t *)o;

	LOG_INF("Entering into DFU mode");
	int err = -ENOTSUP;
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	err = nordic_dfu_ble_start();
#endif
	if (err) {
		LOG_ERR("dfu start err %d", err);
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK_INIT]);
		return;
	}
}

static void state_dfu_run(void *o)
{
	sm_t *sm = (sm_t *)o;

	switch (sm->event) {
	case SID_EVENT_NORDIC_DFU:
		int err = -ENOTSUP;
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
		err = nordic_dfu_ble_stop();
#endif
		if (err) {
			LOG_ERR("dfu stop err %d", err);
		}

		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK_INIT]);
		break;
	case SID_EVENT_SEND_MSG:
	case SID_EVENT_CONNECT:
	case SID_EVENT_FACTORY_RESET:
	case SID_EVENT_LINK_SWITCH:
		LOG_WRN("Operation not supported in DFU mode");
		break;
	case SID_EVENT_SIDEWALK:
	case SID_EVENT_STATE_READY:
	case SID_EVENT_STATE_NOT_READY:
	case SID_EVENT_STATE_ERROR:
		LOG_INF("Invalid operation, Sidewalk is not running");
		break;
	}
}

static void sid_thread_entry(void *context, void *unused, void *unused2)
{
	ARG_UNUSED(unused);
	ARG_UNUSED(unused2);

	sid_sm.sid = (sidewalk_ctx_t *)context;

	k_msgq_init(&sid_sm.msgq, sid_msgq_buff, sizeof(sidewalk_event_t),
		    CONFIG_SIDEWALK_THREAD_QUEUE_SIZE);
	smf_set_initial(SMF_CTX(&sid_sm), &sid_states[STATE_SIDEWALK_INIT]);

	while (1) {
		int err = k_msgq_get(&sid_sm.msgq, &sid_sm.event, K_FOREVER);
		if (!err) {
			if (smf_run_state(SMF_CTX(&sid_sm))) {
				LOG_ERR("sm termination");
				break;
			}
		} else {
			LOG_ERR("msgq err %d", err);
		}
	}

	LOG_ERR("Sidewalk thread ends. You should never see this message.");
}

void sidewalk_start(sidewalk_ctx_t *context)
{
	(void)k_thread_create(&sid_thread, sid_thread_stack,
			      K_THREAD_STACK_SIZEOF(sid_thread_stack), sid_thread_entry, context,
			      NULL, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	(void)k_thread_name_set(&sid_thread, "sidewalk");
}

int sidewalk_msg_set(sidewalk_msg_t *msg_in)
{
	atomic_t busy = atomic_set(&send_msg_buffer_busy, true);
	if (busy) {
		return -EBUSY;
	}

	memcpy(&send_msg_buffer, msg_in, sizeof(sidewalk_msg_t));
	return 0;
}

int sidewalk_event_send(sidewalk_event_t event)
{
	return k_msgq_put(&sid_sm.msgq, (void *)&event, K_NO_WAIT);
}
