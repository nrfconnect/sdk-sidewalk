/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sidewalk.h>
#include <sid_pal_common_ifc.h>
#include <sid_hal_memory_ifc.h>
#include <json_printer/sidTypes2Json.h>
#include <sidewalk_dfu/nordic_dfu.h>
#include <app_mfg_config.h>
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
#include <settings_utils.h>
#endif
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <app_subGHz_config.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
#include <sbdt/file_transfer.h>
#include <sid_bulk_data_transfer_api.h>
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
#include <sidewalk_dfu/nordic_dfu_img.h>
#include <zephyr/dfu/mcuboot.h>
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
#include <stdio.h> // print hash only
#include <sid_pal_crypto_ifc.h> // print hash only
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER */
#ifdef CONFIG_SID_END_DEVICE_CLI
#include <cli/app_dut.h>
#endif /* CONFIG_SID_END_DEVICE_CLI */
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
#include <settings_utils.h>
#endif /* CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK */

#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

LOG_MODULE_REGISTER(sidewalk_app, CONFIG_SIDEWALK_LOG_LEVEL);

static struct k_thread sid_thread;
K_THREAD_STACK_DEFINE(sid_thread_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);

sys_slist_t pending_message_list = SYS_SLIST_STATIC_INIT(&pending_message_list);
K_MUTEX_DEFINE(pending_message_list_mutex);

void push_message_buffer(sidewalk_msg_t *msg)
{
	int mutex_err = k_mutex_lock(&pending_message_list_mutex, K_FOREVER);
	if (mutex_err != 0) {
		LOG_ERR("Failed to lock mutex for message list");
		return;
	}
	sys_slist_append(&pending_message_list, &msg->node);
	k_mutex_unlock(&pending_message_list_mutex);
}

sidewalk_msg_t *get_message_buffer(uint16_t message_id)
{
	sidewalk_msg_t *pending_message;
	sidewalk_msg_t *iterator;
	int mutex_err =
		k_mutex_lock(&pending_message_list_mutex, k_is_in_isr() ? K_NO_WAIT : K_FOREVER);
	if (mutex_err != 0) {
		LOG_ERR("Failed to lock mutex for message list");
		return NULL;
	}
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE (&pending_message_list, pending_message, iterator, node) {
		if (pending_message->desc.id == message_id) {
			if (sys_slist_find_and_remove(&pending_message_list,
						      &pending_message->node) == false) {
				LOG_ERR("Failed to remove pending message from list");
			};
			k_mutex_unlock(&pending_message_list_mutex);
			return pending_message;
		}
	}
	k_mutex_unlock(&pending_message_list_mutex);
	return NULL;
}

static void state_sidewalk_run(void *o);
static void state_sidewalk_entry(void *o);
static void state_sidewalk_exit(void *o);
static void state_dfu_entry(void *o);
static void state_dfu_run(void *o);

static uint8_t __aligned(4)
	sid_msgq_buff[CONFIG_SIDEWALK_THREAD_QUEUE_SIZE * sizeof(sidewalk_ctx_event_t)];
static sm_t sid_sm = { .sid_states = {
			       [STATE_SIDEWALK] =
				       SMF_CREATE_STATE(state_sidewalk_entry, state_sidewalk_run,
							state_sidewalk_exit, NULL, NULL),
			       [STATE_DFU] = SMF_CREATE_STATE(state_dfu_entry, state_dfu_run, NULL,
							      NULL, NULL),
		       } };

static void state_sidewalk_entry(void *o)
{
	LOG_INF("Enter Sidewalk state");
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
}

static bool validate_event(sm_t *sm, enum sidewalk_fsm_states state)
{
	if (sm == NULL) {
		LOG_ERR("State machine object is NULL");
		return false;
	}
	if (sm->application_event.event_handler.call[state] == NULL) {
		LOG_DBG("Can not handle event [%s] without valid handler",
			sm->application_event.event_handler.name ?
				sm->application_event.event_handler.name :
				"UNKNOWN");
		return false;
	}
	return true;
}

static void state_sidewalk_run(void *o)
{
	sm_t *sm = (sm_t *)o;
	sidewalk_ctx_event_t *event_to_process = &sm->application_event;
	if (!validate_event(sm, STATE_SIDEWALK)) {
		return;
	}
	LOG_DBG("Handle event %s", sm->application_event.event_handler.name ?
					   sm->application_event.event_handler.name :
					   "UNKNOWN");
	event_to_process->event_handler.call[STATE_SIDEWALK](event_to_process->ctx, sm);
}

static void state_sidewalk_exit(void *o)
{
	LOG_INF("Exit Sidewalk state");
	int mutex_err = k_mutex_lock(&pending_message_list_mutex, K_FOREVER);
	if (mutex_err != 0) {
		LOG_ERR("Failed to lock mutex for message list");
		return;
	}
	sys_snode_t *list_element = sys_slist_get(&pending_message_list);

	while (list_element != NULL) {
		sidewalk_msg_t *message = SYS_SLIST_CONTAINER(list_element, message, node);
		sid_hal_free(message->msg.data);
		sid_hal_free(message);

		list_element = sys_slist_get(&pending_message_list);
	}
	k_mutex_unlock(&pending_message_list_mutex);
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
		smf_set_state(SMF_CTX(sm), &sm->sid_states[STATE_SIDEWALK]);
		return;
	}
}

static void state_dfu_run(void *o)
{
	sm_t *sm = (sm_t *)o;
	sidewalk_ctx_event_t *event_to_process = &sm->application_event;

	if (!validate_event(sm, STATE_DFU)) {
		return;
	}
	event_to_process->event_handler.call[STATE_DFU](event_to_process->ctx, sm);
}

static void sid_thread_entry(void *context, void *sidewalk_started, void *unused)
{
	ARG_UNUSED(unused);

	sid_sm.sid = (sidewalk_ctx_t *)context;

	k_msgq_init(&sid_sm.msgq, sid_msgq_buff, sizeof(sidewalk_ctx_event_t),
		    CONFIG_SIDEWALK_THREAD_QUEUE_SIZE);
	smf_set_initial(SMF_CTX(&sid_sm), &sid_sm.sid_states[STATE_SIDEWALK]);

	k_sem_give((struct k_sem *)sidewalk_started);
	while (1) {
		int err = k_msgq_get(&sid_sm.msgq, &sid_sm.application_event, K_FOREVER);
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
	struct k_sem sidewalk_started;
	k_sem_init(&sidewalk_started, 0, 1);
	(void)k_thread_create(&sid_thread, sid_thread_stack,
			      K_THREAD_STACK_SIZEOF(sid_thread_stack), sid_thread_entry, context,
			      &sidewalk_started, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, 0,
			      K_NO_WAIT);
	(void)k_thread_name_set(&sid_thread, "sidewalk");
	k_sem_take(&sidewalk_started, K_FOREVER);
}

int sidewalk_event_send(sidewalk_event_t event, void *ctx)
{
	sidewalk_ctx_event_t ctx_event = {
		.event_handler = event,
		.ctx = ctx,
	};
	const int result = k_msgq_put(&sid_sm.msgq, (void *)&ctx_event, K_NO_WAIT);
	LOG_DBG("Pushed event %s, k_msg_put returned %d", event.name, result);

	return result;
}
