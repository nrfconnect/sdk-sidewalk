/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "json_printer/sidTypes2str.h"
#include <sidewalk.h>
#include <sid_error.h>
#include <app_mfg_config.h>
#include <sid_pal_common_ifc.h>
#include <sid_hal_memory_ifc.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <app_subGHz_config.h>
#include <sid_pal_radio_ifc.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/reboot.h>
#include <json_printer/sidTypes2Json.h>
#include <sidewalk_dfu/nordic_dfu.h>
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
#include <settings_utils.h>
#endif /* CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK */

#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
#include <sbdt/dfu_file_transfer.h>
#include <zephyr/dfu/mcuboot.h>
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */

LOG_MODULE_REGISTER(sidewalk_events, CONFIG_SIDEWALK_LOG_LEVEL);

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
static sid_pal_radio_rx_packet_t radio_rx_packet;

static void radio_event_notifier(sid_pal_radio_events_t event)
{
	LOG_DBG("Radio event %d", event);
}

static void radio_irq_handler(void)
{
	LOG_DBG("Radio IRQ");
}
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */

// private
void sidewalk_event_process(sidewalk_ctx_t *sid, void *ctx)
{
	if (sid == NULL || sid->handle == NULL) {
		LOG_INF("sidewalk need to be started first.");
		return;
	}
	sid_error_t e = sid_process(sid->handle);
	if (e) {
		LOG_ERR("sid process err  %d (%s)", (int)e, SID_ERROR_T_STR(e));
	}
}

void sidewalk_event_platform_init(sidewalk_ctx_t *sid, void *ctx)
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
		LOG_ERR("Sidewalk Platform Init err:  %d (%s)", (int)e, SID_ERROR_T_STR(e));
		return;
	}
	if (app_mfg_cfg_is_empty()) {
		LOG_ERR("The mfg.hex version mismatch");
		LOG_ERR("Check if the file has been generated and flashed properly");
		LOG_ERR("START ADDRESS: 0x%08x", APP_MFG_CFG_FLASH_START);
		LOG_ERR("SIZE: 0x%08x", APP_MFG_CFG_FLASH_SIZE);
		return;
	}

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
	int32_t err = 0;
	err = sid_pal_radio_init(radio_event_notifier, radio_irq_handler, &radio_rx_packet);
	if (err) {
		LOG_ERR("radio init err %d", err);
	}
	err = sid_pal_radio_sleep(0);
	if (err) {
		LOG_ERR("radio sleep err %d", err);
	}
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
}

void sidewalk_event_autostart(sidewalk_ctx_t *sid, void *ctx)
{
	if (sid->handle != NULL) {
		LOG_INF("Sidewlak is already running");
		return;
	}
	if (app_mfg_cfg_is_empty()) {
		LOG_ERR("The mfg.hex version mismatch");
		LOG_ERR("Check if the file has been generated and flashed properly");
		LOG_ERR("START ADDRESS: 0x%08x", APP_MFG_CFG_FLASH_START);
		LOG_ERR("SIZE: 0x%08x", APP_MFG_CFG_FLASH_SIZE);
		return;
	}
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
	sid_error_t e = sid_init(&sid->config, &sid->handle);
	if (e) {
		LOG_ERR("sid init err %d (%s)", (int)e, SID_ERROR_T_STR(e));
		return;
	}

	e = sid_start(sid->handle, sid->config.link_mask);
	if (e) {
		LOG_ERR("sid start err %d (%s)", (int)e, SID_ERROR_T_STR(e));
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
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	int dfu_err = boot_write_img_confirmed();
	if (dfu_err) {
		LOG_ERR("img confirm fail %d", dfu_err);
	}
	app_file_transfer_demo_init(sid->handle);
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
}

void sidewalk_event_factory_reset(sidewalk_ctx_t *sid, void *ctx)
{
	if (sid == NULL || sid->handle == NULL) {
		LOG_INF("sidewalk need to be started first.");
		return;
	}
#ifdef CONFIG_SID_END_DEVICE_PERSISTENT_LINK_MASK
	(void)settings_utils_link_mask_set(0);
#endif /* CONFIG_SIDEWALK_PERSISTENT_LINK_MASK */
	sid_error_t e = sid_set_factory_reset(sid->handle);
	if (e) {
		LOG_ERR("sid factory reset err  %d (%s)", (int)e, SID_ERROR_T_STR(e));
	}
}

void sidewalk_event_new_status(sidewalk_ctx_t *sid, void *ctx)
{
	struct sid_status *p_status = (struct sid_status *)ctx;
	if (!p_status) {
		LOG_ERR("sid new status is NULL");
		return;
	}

	memcpy(&sid->last_status, p_status, sizeof(struct sid_status));
}
void sidewalk_event_send_msg(sidewalk_ctx_t *sid, void *ctx)
{
	if (sid == NULL || sid->handle == NULL) {
		LOG_INF("sidewalk need to be started first.");
		return;
	}
	sidewalk_msg_t *p_msg = (sidewalk_msg_t *)ctx;
	if (!p_msg) {
		LOG_ERR("sid send msg is NULL");
		return;
	}

	sid_error_t e = sid_put_msg(sid->handle, &p_msg->msg, &p_msg->desc);
	if (e) {
		LOG_ERR("sid send err %d", (int)e);
		return;
	}
	LOG_DBG("sid send (type: %d, id: %u)", (int)p_msg->desc.type, p_msg->desc.id);
}
void sidewalk_event_connect(sidewalk_ctx_t *sid, void *ctx)
{
	if (sid == NULL || sid->handle == NULL) {
		LOG_INF("sidewalk need to be started first.");
		return;
	}
	if (!(sid->config.link_mask & SID_LINK_TYPE_1)) {
		LOG_ERR("Can not request connection - BLE not enabled");
		return;
	}
	sid_error_t e = sid_ble_bcn_connection_request(sid->handle, true);
	if (e) {
		LOG_ERR("sid conn req err %d (%s)", (int)e, SID_ERROR_T_STR(e));
	}
}
void sidewalk_event_link_switch(sidewalk_ctx_t *sid, void *ctx)
{
	if (sid == NULL || sid->handle == NULL) {
		LOG_INF("Can not change link if sidewalk was not started yet");
		return;
	}
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
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
		app_file_transfer_demo_deinit(sid->handle);
#endif
		(void)sid_process(sid->handle);
		sid_error_t e = sid_deinit(sid->handle);
		if (e) {
			LOG_ERR("sid deinit err %d (%s)", (int)e, SID_ERROR_T_STR(e));
		}
	}

	sid_error_t e = sid_init(&sid->config, &sid->handle);
	if (e) {
		LOG_ERR("sid init err %d (%s)", (int)e, SID_ERROR_T_STR(e));
	}
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	app_file_transfer_demo_init(sid->handle);
#endif
	e = sid_start(sid->handle, sid->config.link_mask);
	if (e) {
		LOG_ERR("sid start err %d (%s)", (int)e, SID_ERROR_T_STR(e));
	}
#if CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
	if (sid->config.link_mask & SID_LINK_TYPE_1) {
		enum sid_link_connection_policy set_policy =
			SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;

		e = sid_option(sid->handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &set_policy,
			       sizeof(set_policy));
		if (e) {
			LOG_ERR("sid option multi link manager err %d (%s)", (int)e,
				SID_ERROR_T_STR(e));
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
			LOG_ERR("sid option multi link policy err %d (%s)", (int)e,
				SID_ERROR_T_STR(e));
		}
	}
#endif /* CONFIG_SID_END_DEVICE_AUTO_CONN_REQ */
}

void sidewalk_event_exit(sidewalk_ctx_t *sid, void *ctx)
{
	if (sid == NULL || sid->handle == NULL) {
		LOG_INF("Sidewalk is already deinitialized");
		return;
	}
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	app_file_transfer_demo_deinit(sid->handle);
#endif
	(void)sid_process(sid->handle);
	(void)sid_deinit(sid->handle);
}

void sidewalk_event_reboot(sidewalk_ctx_t *sid, void *ctx)
{
	LOG_INF("Rebooting...");
	LOG_PANIC();
	sys_reboot(SYS_REBOOT_WARM);
}
