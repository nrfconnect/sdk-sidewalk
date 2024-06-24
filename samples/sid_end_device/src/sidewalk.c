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

sys_slist_t pending_message_list = SYS_SLIST_STATIC_INIT(&pending_message_list);
K_MUTEX_DEFINE(pending_message_list_mutex);

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

static void state_sidewalk_run(void *o);
static void state_sidewalk_entry(void *o);
static void state_sidewalk_exit(void *o);
static void state_dfu_entry(void *o);
static void state_dfu_run(void *o);

static const struct smf_state sid_states[] = {
	[STATE_SIDEWALK] = SMF_CREATE_STATE(state_sidewalk_entry, state_sidewalk_run,
					    state_sidewalk_exit, NULL, NULL),
	[STATE_DFU] = SMF_CREATE_STATE(state_dfu_entry, state_dfu_run, NULL, NULL, NULL),
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

#if CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
	if (sm->sid->config.link_mask & SID_LINK_TYPE_1) {
		enum sid_link_connection_policy set_policy =
			SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;

		e = sid_option(sm->sid->handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &set_policy,
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

		e = sid_option(sm->sid->handle, SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS,
			       &ac_params, sizeof(ac_params));
		if (e) {
			LOG_ERR("sid option multi link policy err %d", (int)e);
		}
	}
#endif /* CONFIG_SID_END_DEVICE_AUTO_CONN_REQ */

#endif /* CONFIG_SIDEWALK_AUTO_START */

#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	int dfu_err = boot_write_img_confirmed();
	if (dfu_err) {
		LOG_ERR("img confirm fail %d", dfu_err);
	}
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
	app_file_transfer_demo_init(((sm_t *)o)->sid->handle);
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER */
}

static void state_sidewalk_run(void *o)
{
	sm_t *sm = (sm_t *)o;
	sid_error_t e = SID_ERROR_NONE;

	switch (sm->event.id) {
	case SID_EVENT_SIDEWALK: {
		e = sid_process(sm->sid->handle);
		if (e) {
			LOG_ERR("sid process err %d", (int)e);
		}
	} break;
	case SID_EVENT_FACTORY_RESET: {
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
		(void)settings_utils_link_mask_set(0);
#endif /* CONFIG_SIDEWALK_PERSISTENT_LINK_MASK */
		e = sid_set_factory_reset(sm->sid->handle);
		if (e) {
			LOG_ERR("sid factory reset err %d", (int)e);
		}
	} break;
	case SID_EVENT_LINK_SWITCH: {
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
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
			app_file_transfer_demo_deinit(sm->sid->handle);
#endif
			e = sid_deinit(sm->sid->handle);
			if (e) {
				LOG_ERR("sid deinit err %d", (int)e);
			}
		}

		e = sid_init(&sm->sid->config, &sm->sid->handle);
		if (e) {
			LOG_ERR("sid init err %d", (int)e);
		}
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
		app_file_transfer_demo_init(sm->sid->handle);
#endif
		e = sid_start(sm->sid->handle, sm->sid->config.link_mask);
		if (e) {
			LOG_ERR("sid start err %d", (int)e);
		}
#if CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
		if (sm->sid->config.link_mask & SID_LINK_TYPE_1) {
			enum sid_link_connection_policy set_policy =
				SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;

			e = sid_option(sm->sid->handle, SID_OPTION_SET_LINK_CONNECTION_POLICY,
				       &set_policy, sizeof(set_policy));
			if (e) {
				LOG_ERR("sid option multi link manager err %d", (int)e);
			}

			struct sid_link_auto_connect_params ac_params = {
				.link_type = SID_LINK_TYPE_1,
				.enable = true,
				.priority = 0,
				.connection_attempt_timeout_seconds = 30
			};

			e = sid_option(sm->sid->handle,
				       SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS, &ac_params,
				       sizeof(ac_params));
			if (e) {
				LOG_ERR("sid option multi link policy err %d", (int)e);
			}
		}
#endif /* CONFIG_SID_END_DEVICE_AUTO_CONN_REQ */
	} break;
	case SID_EVENT_NORDIC_DFU: {
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
		app_file_transfer_demo_deinit(sm->sid->handle);
#endif
		e = sid_deinit(sm->sid->handle);
		if (e) {
			LOG_ERR("sid deinit err %d", (int)e);
		}
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_DFU]);
	} break;
	case SID_EVENT_NEW_STATUS: {
		struct sid_status *p_status = (struct sid_status *)sm->event.ctx;
		if (!p_status) {
			LOG_ERR("sid new status is NULL");
			break;
		}

		memcpy(&sm->sid->last_status, p_status, sizeof(struct sid_status));
		sid_hal_free(p_status);
	} break;
	case SID_EVENT_SEND_MSG: {
		sidewalk_msg_t *p_msg = (sidewalk_msg_t *)sm->event.ctx;
		if (!p_msg) {
			LOG_ERR("sid send msg is NULL");
			break;
		}

		e = sid_put_msg(sm->sid->handle, &p_msg->msg, &p_msg->desc);
		if (e) {
			LOG_ERR("sid send err %d", (int)e);
		}
		LOG_DBG("sid send (type: %d, id: %u)", (int)p_msg->desc.type, p_msg->desc.id);
		int mutex_err = k_mutex_lock(&pending_message_list_mutex, K_FOREVER);
		if (mutex_err != 0) {
			LOG_ERR("Failed to lock mutex for message list");
			break;
		}
		sys_slist_append(&pending_message_list, &p_msg->node);
		k_mutex_unlock(&pending_message_list_mutex);
	} break;
	case SID_EVENT_CONNECT: {
		if (!(sm->sid->config.link_mask & SID_LINK_TYPE_1)) {
			LOG_ERR("Can not request connection - BLE not enabled");
			return;
		}
		sid_error_t e = sid_ble_bcn_connection_request(sm->sid->handle, true);
		if (e) {
			LOG_ERR("sid conn req err %d", (int)e);
		}
	} break;
	case SID_EVENT_FILE_TRANSFER: {
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
		sidewalk_transfer_t *transfer = (sidewalk_transfer_t *)sm->event.ctx;
		if (!transfer) {
			LOG_ERR("File transfer event data is NULL");
			break;
		}

		LOG_INF("Received file Id %d; buffer size %d; file offset %d", transfer->file_id,
			transfer->data_size, transfer->file_offset);

		// print data hash
		uint8_t hash_out[32];
		sid_pal_hash_params_t params = { .algo = SID_PAL_HASH_SHA256,
						 .data = transfer->data,
						 .data_size = transfer->data_size,
						 .digest = hash_out,
						 .digest_size = sizeof(hash_out) };

		sid_error_t e = sid_pal_crypto_hash(&params);
		if (e != SID_ERROR_NONE) {
			LOG_ERR("Failed to hash received file transfer with error %s",
				SID_ERROR_T_STR(e));
		} else {
#define HEX_PRINTER(a, ...) "%02X"
#define HEX_PRINTER_ARG(a, ...) hash_out[a]
			char hex_str[sizeof(hash_out) * 2 + 1] = { 0 };
			snprintf(hex_str, sizeof(hex_str), LISTIFY(32, HEX_PRINTER, ()),
				 LISTIFY(32, HEX_PRINTER_ARG, (, )));
			LOG_INF("SHA256: %s", hex_str);
		}

#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
		int err = nordic_dfu_img_write(transfer->file_offset, transfer->data,
					       transfer->data_size);

		if (err) {
			LOG_ERR("Fail to write img %d", err);
			err = nordic_dfu_img_cancel();
			if (err) {
				LOG_ERR("Fail to complete dfu %d", err);
			}
			e = sid_bulk_data_transfer_cancel(
				sm->sid->handle, transfer->file_id,
				SID_BULK_DATA_TRANSFER_REJECT_REASON_FILE_TOO_BIG);
			if (e != SID_ERROR_NONE) {
				LOG_ERR("sbdt cancel ret %s", SID_ERROR_T_STR(e));
			}

			sid_hal_free(transfer);
			break;
		}
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
		const struct sid_bulk_data_transfer_buffer sbdt_buffer = {
			.data = transfer->data,
			.size = transfer->data_size,
		};
		e = sid_bulk_data_transfer_release_buffer(sm->sid->handle, transfer->file_id,
							  &sbdt_buffer);
		if (e != SID_ERROR_NONE) {
			LOG_ERR("sbdt release ret %s", SID_ERROR_T_STR(e));
		}

		// free event context
		sid_hal_free(transfer);
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER */
	} break;
	case SID_EVENT_REBOOT: {
		LOG_INF("Rebooting...");
		LOG_PANIC();
		sys_reboot(SYS_REBOOT_WARM);
	} break;
	case SID_EVENT_LAST:
		break;
	}

#ifdef CONFIG_SID_END_DEVICE_CLI
	if (sm->event.id >= SID_EVENT_LAST) {
		app_dut_event_process(sm->event, sm->sid);
	}
#endif /* CONFIG_SID_END_DEVICE_CLI */
}

static void state_sidewalk_exit(void *o)
{
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
		smf_set_state(SMF_CTX(sm), &sid_states[STATE_SIDEWALK]);
		return;
	}
}

static void state_dfu_run(void *o)
{
	sm_t *sm = (sm_t *)o;
	int err = -ENOTSUP;
	switch (sm->event.id) {
	case SID_EVENT_NORDIC_DFU:
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
	case SID_EVENT_FILE_TRANSFER:
	case SID_EVENT_REBOOT:
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

int sidewalk_event_send(sidewalk_event_t event, void *ctx)
{
	sidewalk_ctx_event_t ctx_event = {
		.id = event,
		.ctx = ctx,
	};

	const int result = k_msgq_put(&sid_sm.msgq, (void *)&ctx_event, K_NO_WAIT);
	LOG_DBG("sidewalk_event_send event = %d (%s), context = %p, k_msgq_put result %d", event,
		SIDEWALK_EVENT_T_STR(event), ctx, result);
	return result;
}
