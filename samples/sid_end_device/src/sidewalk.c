/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app_mfg_config.h>
#include <sid_pal_common_ifc.h>
#include <sidewalk.h>
#include <nordic_dfu.h>
#ifdef CONFIG_SID_END_DEVICE_CLI
#include <cli/app_dut.h>
#endif
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
#include <settings_utils.h>
#endif
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
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
K_HEAP_DEFINE(data_heap, CONFIG_SID_END_DEVICE_EVENT_HEAP_SIZE);

typedef struct sm_s {
	struct smf_ctx ctx;
	struct k_msgq msgq;
	sidewalk_ctx_event_t event;
	sidewalk_ctx_t *sid;
} sm_t;

enum state {
	STATE_SIDEWALK,
	STATE_DFU,
};

#ifdef CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
static void *pending_msg_ctx;
#endif

static void state_sidewalk_run(void *o);
static void state_sidewalk_entry(void *o);
static void state_dfu_entry(void *o);
static void state_dfu_run(void *o);

static const struct smf_state sid_states[] = {
	[STATE_SIDEWALK] = SMF_CREATE_STATE(state_sidewalk_entry, state_sidewalk_run, NULL),
	[STATE_DFU] = SMF_CREATE_STATE(state_dfu_entry, state_dfu_run, NULL),
};

static uint8_t __aligned(4)
	sid_msgq_buff[CONFIG_SIDEWALK_THREAD_QUEUE_SIZE * sizeof(sidewalk_ctx_event_t)];
static sm_t sid_sm;

static void state_sidewalk_entry(void *o)
{
	platform_parameters_t platform_parameters = {
		.mfg_store_region.addr_start = APP_MFG_CFG_FLASH_START,
		.mfg_store_region.addr_end = APP_MFG_CFG_FLASH_END,
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
		.platform_init_parameters.radio_cfg =
			(radio_sx126x_device_config_t *)get_radio_cfg(),
#endif
	};

	sid_error_t e = sid_platform_init(&platform_parameters);
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

#ifdef CONFIG_SID_END_DEVICE_AUTO_START
	sm_t *sm = (sm_t *)o;

#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
	int err = settings_utils_link_mask_get(&sm->sid->config.link_mask);
	if (err <= 0) {
		LOG_WRN("Link mask get failed %d", err);
		sm->sid->config.link_mask = 0;
		settings_utils_link_mask_set(DEFAULT_LM);
	}
#endif /* CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK */

	if (!sm->sid->config.link_mask) {
		sm->sid->config.link_mask = DEFAULT_LM;
	}

	LOG_INF("Sidewalk link switch to %s",
		(SID_LINK_TYPE_3 & sm->sid->config.link_mask) ? "LoRa" :
		(SID_LINK_TYPE_2 & sm->sid->config.link_mask) ? "FSK" :
								"BLE");

	e = sid_init(&sm->sid->config, &sm->sid->handle);
	if (e) {
		LOG_ERR("sid init err %d", (int)e);
	}
	e = sid_start(sm->sid->handle, sm->sid->config.link_mask);
	if (e) {
		LOG_ERR("sid start err %d", (int)e);
	}
#endif /* CONFIG_SID_END_DEVICE_AUTO_START */
}

static void state_sidewalk_run(void *o)
{
	sm_t *sm = (sm_t *)o;
	sid_error_t e = SID_ERROR_NONE;

	switch (sm->event.id) {
	case SID_EVENT_SIDEWALK:
		e = sid_process(sm->sid->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
		break;
	case SID_EVENT_FACTORY_RESET:
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
		(void)settings_utils_link_mask_set(0);
#endif
		e = sid_set_factory_reset(sm->sid->handle);
		if (e) {
			LOG_ERR("sid factory reset err %d", (int)e);
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
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
		int err = settings_utils_link_mask_set(new_link_mask);
		if (err) {
			LOG_ERR("New link mask set err %d", err);
		}
#endif /* CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK */

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
	case SID_EVENT_NEW_STATUS:
		struct sid_status *p_status = (struct sid_status *)sm->event.ctx;
		if (!p_status) {
			LOG_ERR("sid new status is NULL");
			break;
		}

		memcpy(&sm->sid->last_status, p_status, sizeof(struct sid_status));
		sidewalk_data_free(p_status);

#ifdef CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
		if (pending_msg_ctx) {
			sidewalk_event_send(SID_EVENT_SEND_MSG, pending_msg_ctx);
		}
#endif /* CONFIG_SID_END_DEVICE_AUTO_CONN_REQ */
		break;
	case SID_EVENT_SEND_MSG:
		sidewalk_msg_t *p_msg = (sidewalk_msg_t *)sm->event.ctx;
		if (!p_msg) {
			LOG_ERR("sid send msg is NULL");
			break;
		}

#ifdef CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
		bool is_link_up = sm->sid->last_status.detail.link_status_mask & SID_LINK_TYPE_ANY;
		if (!is_link_up && !pending_msg_ctx) {
			pending_msg_ctx = p_msg;
			sidewalk_event_send(SID_EVENT_CONNECT, NULL);
			break;
		}
#endif /* CONFIG_SID_END_DEVICE_AUTO_CONN_REQ */

		e = sid_put_msg(sm->sid->handle, &p_msg->msg, &p_msg->desc);
		if (e) {
			LOG_ERR("sid send err %d", (int)e);
		}
		LOG_DBG("sid send (type: %d, id: %u)", (int)p_msg->desc.type, p_msg->desc.id);
		sidewalk_data_free(p_msg->msg.data);
		sidewalk_data_free(p_msg);

#ifdef CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
		if (is_link_up && pending_msg_ctx) {
			pending_msg_ctx = NULL;
		}
#endif /* CONFIG_SID_END_DEVICE_AUTO_CONN_REQ */
		break;
	case SID_EVENT_CONNECT:
		sid_error_t e = sid_ble_bcn_connection_request(sm->sid->handle, true);
		if (e) {
			LOG_ERR("sid conn req err %d", (int)e);
		}
		break;
	case SID_EVENT_LAST:
		break;
	}

#ifdef CONFIG_SID_END_DEVICE_CLI
	if (sm->event.id >= SID_EVENT_LAST) {
		app_dut_event_process(sm->event, sm->sid);
	}
#endif /* CONFIG_SID_END_DEVICE_CLI */
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
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK]);
		return;
	}
}

static void state_dfu_run(void *o)
{
	sm_t *sm = (sm_t *)o;

	switch (sm->event.id) {
	case SID_EVENT_NORDIC_DFU:
		int err = -ENOTSUP;
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
		err = nordic_dfu_ble_stop();
#endif
		if (err) {
			LOG_ERR("dfu stop err %d", err);
		}

		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK]);
		break;
	case SID_EVENT_NEW_STATUS:
	case SID_EVENT_SEND_MSG:
	case SID_EVENT_CONNECT:
	case SID_EVENT_FACTORY_RESET:
	case SID_EVENT_LINK_SWITCH:
	case SID_EVENT_SIDEWALK:
		LOG_INF("Operation not supported in DFU mode");
		break;
	case SID_EVENT_LAST:
		break;
	}
}

static void sid_thread_entry(void *context, void *unused, void *unused2)
{
	ARG_UNUSED(unused);
	ARG_UNUSED(unused2);

	sid_sm.sid = (sidewalk_ctx_t *)context;

	k_msgq_init(&sid_sm.msgq, sid_msgq_buff, sizeof(sidewalk_ctx_event_t),
		    CONFIG_SIDEWALK_THREAD_QUEUE_SIZE);
	smf_set_initial(SMF_CTX(&sid_sm), &sid_states[STATE_SIDEWALK]);

	while (1) {
		int err = k_msgq_get(&sid_sm.msgq, &sid_sm.event, K_FOREVER);
		if (!err) {
			if (smf_run_state(SMF_CTX(&sid_sm))) {
				LOG_ERR("Sidewalk state machine termination");
				break;
			}
		} else {
			LOG_ERR("Sidewalk msgq err %d", err);
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

void *sidewalk_data_alloc(size_t ctx_size)
{
	return k_heap_alloc(&data_heap, ctx_size, K_NO_WAIT);
}

void sidewalk_data_free(void *ctx)
{
	k_heap_free(&data_heap, ctx);
}

int sidewalk_event_send(sidewalk_event_t event, void *ctx)
{
	sidewalk_ctx_event_t ctx_event = {
		.id = event,
		.ctx = ctx,
	};

	return k_msgq_put(&sid_sm.msgq, (void *)&ctx_event, K_NO_WAIT);
}
