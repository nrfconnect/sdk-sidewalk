/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_error.h>
#include <cli/app_dut.h>
#include <sidewalk.h>
#include <sid_900_cfg.h>
#include <sid_hal_memory_ifc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <app_mfg_config.h>
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
#include <sbdt/dfu_file_transfer.h>
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
#include <json_printer/sidTypes2str.h>
LOG_MODULE_REGISTER(sid_cli, CONFIG_SIDEWALK_LOG_LEVEL);

static uint32_t dut_ctx_get_uint32(void *ctx)
{
	if (!ctx) {
		LOG_ERR("Invalid context!");
		return 0;
	}
	uint32_t ctx_val = *((uint32_t *)ctx);
	return ctx_val;
}

void dut_event_init(sidewalk_ctx_t *sid, void *ctx)
{
	sid->config.link_mask = dut_ctx_get_uint32(ctx);
	if (app_mfg_cfg_is_valid()) {
		LOG_ERR("The mfg.hex version mismatch");
		LOG_ERR("Check if the file has been generated and flashed properly");
		LOG_ERR("START ADDRESS: 0x%08x", APP_MFG_CFG_FLASH_START);
		LOG_ERR("SIZE: 0x%08x", APP_MFG_CFG_FLASH_SIZE);
		return;
	}
	sid_error_t e = sid_init(&sid->config, &sid->handle);
	LOG_INF("sid_init returned %d (%s)", e, SID_ERROR_T_STR(e));
	if (e != SID_ERROR_NONE) {
		return;
	}
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	app_file_transfer_demo_init(sid->handle);
#endif
}
void dut_event_deinit(sidewalk_ctx_t *sid, void *ctx)
{
	if (ctx) {
		LOG_WRN("Unexpected context");
	};
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	app_file_transfer_demo_deinit(sid->handle);
#endif
	sid_error_t e = sid_deinit(sid->handle);
	LOG_INF("sid_deinit returned %d (%s)", (int)e, SID_ERROR_T_STR(e));
}
void dut_event_start(sidewalk_ctx_t *sid, void *ctx)
{
	uint32_t link_mask = dut_ctx_get_uint32(ctx);
	sid_error_t e = sid_start(sid->handle, link_mask);
	LOG_INF("sid_start returned %d (%s)", (int)e, SID_ERROR_T_STR(e));
}
void dut_event_stop(sidewalk_ctx_t *sid, void *ctx)
{
	uint32_t link_mask = dut_ctx_get_uint32(ctx);
	sid_error_t e = sid_stop(sid->handle, link_mask);
	LOG_INF("sid_stop returned %d (%s)", (int)e, SID_ERROR_T_STR(e));
}
void dut_event_get_mtu(sidewalk_ctx_t *sid, void *ctx)
{
	uint32_t link_mask = dut_ctx_get_uint32(ctx);
	size_t mtu = 0;
	sid_error_t e = sid_get_mtu(sid->handle, (enum sid_link_type)link_mask, &mtu);
	LOG_INF("sid_get_mtu returned %d (%s), MTU: %d", e, SID_ERROR_T_STR(e), mtu);
}
void dut_event_get_time(sidewalk_ctx_t *sid, void *ctx)
{
	uint32_t format = dut_ctx_get_uint32(ctx);
	struct sid_timespec curr_time = { 0 };
	sid_error_t e = sid_get_time(sid->handle, (enum sid_time_format)format, &curr_time);
	LOG_INF("sid_get_time returned %d (%s), SEC: %d NSEC: %d", e, SID_ERROR_T_STR(e),
		curr_time.tv_sec, curr_time.tv_nsec);
}
void dut_event_get_status(sidewalk_ctx_t *sid, void *ctx)
{
	LOG_INF("Device %sregistered, Time Sync %s, Link status: {BLE: %s, FSK: %s, LoRa: %s}",
		(SID_STATUS_REGISTERED == sid->last_status.detail.registration_status) ? "Is " :
											 "Un",
		(SID_STATUS_TIME_SYNCED == sid->last_status.detail.time_sync_status) ? "Success" :
										       "Fail",
		(sid->last_status.detail.link_status_mask & SID_LINK_TYPE_1) ? "Up" : "Down",
		(sid->last_status.detail.link_status_mask & SID_LINK_TYPE_2) ? "Up" : "Down",
		(sid->last_status.detail.link_status_mask & SID_LINK_TYPE_3) ? "Up" : "Down");

	for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
		enum sid_link_mode mode =
			(enum sid_link_mode)sid->last_status.detail.supported_link_modes[i];

		if (mode) {
			LOG_INF("Link mode on %s = {Cloud: %s, Mobile: %s}",
				(SID_LINK_TYPE_1_IDX == i) ? "BLE" :
				(SID_LINK_TYPE_2_IDX == i) ? "FSK" :
				(SID_LINK_TYPE_3_IDX == i) ? "LoRa" :
							     "unknow",
				(mode & SID_LINK_MODE_CLOUD) ? "True" : "False",
				(mode & SID_LINK_MODE_MOBILE) ? "True" : "False");
		}
	}
}
void dut_event_get_option(sidewalk_ctx_t *sid, void *ctx)
{
	sidewalk_option_t *p_option = (sidewalk_option_t *)ctx;
	if (!p_option) {
		LOG_ERR("Invalid context!");
		return;
	}
	enum sid_option opt = p_option->option;
	switch (opt) {
	case SID_OPTION_GET_MSG_POLICY_FILTER_DUPLICATES: {
		uint8_t data = 0;
		sid_error_t e = sid_option(sid->handle, opt, &data, sizeof(data));
		LOG_INF("sid_option returned %d (%s); Filter Duplicates: %d", e, SID_ERROR_T_STR(e),
			data);
	} break;
	case SID_OPTION_GET_LINK_CONNECTION_POLICY: {
		uint8_t data = 0;
		sid_error_t e = sid_option(sid->handle, opt, &data, sizeof(data));
		LOG_INF("sid_option returned %d (%s); Link Connect Policy: %d", e,
			SID_ERROR_T_STR(e), data);
	} break;
	case SID_OPTION_GET_LINK_POLICY_MULTI_LINK_POLICY: {
		uint8_t data = 0;
		sid_error_t e = sid_option(sid->handle, opt, &data, sizeof(data));
		LOG_INF("sid_option returned %d (%s); Link Multi Link Policy: %d", e,
			SID_ERROR_T_STR(e), data);
	} break;
	case SID_OPTION_GET_STATISTICS: {
		struct sid_statistics stats = { 0 };
		sid_error_t e = sid_option(sid->handle, opt, &stats, sizeof(stats));
		LOG_INF("sid_option returned %d (%s); tx: %d, acks_sent %d, tx_fail: %d, retries: %d, dups: %d, acks_recv: %d rx: %d",
			e, SID_ERROR_T_STR(e), stats.msg_stats.tx, stats.msg_stats.acks_sent,
			stats.msg_stats.tx_fail, stats.msg_stats.retries,
			stats.msg_stats.duplicates, stats.msg_stats.acks_recv, stats.msg_stats.rx);
	} break;
	case SID_OPTION_GET_LINK_POLICY_AUTO_CONNECT_PARAMS: {
		struct sid_link_auto_connect_params params = { 0 };
		memcpy(&params.link_type, p_option->data, sizeof(uint32_t));
		sid_error_t e = sid_option(sid->handle, opt, (void *)&params, sizeof(params));
		LOG_INF("sid_option returned %d (%s); AC Policy, link %d, enable %d priority %d timeout %d",
			e, SID_ERROR_T_STR(e), params.link_type, params.enable, params.priority,
			params.connection_attempt_timeout_seconds);
	} break;
	case SID_OPTION_900MHZ_GET_DEVICE_PROFILE: {
		struct sid_device_profile dev_cfg = { 0 };
		if (p_option->data) {
			dev_cfg.unicast_params.device_profile_id =
				*((enum sid_device_profile_id *)p_option->data);
		}

		sid_error_t e = sid_option(sid->handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
					   &dev_cfg, sizeof(struct sid_device_profile));

		if (IS_LINK2_PROFILE_ID(dev_cfg.unicast_params.device_profile_id) ||
		    IS_LINK3_PROFILE_ID(dev_cfg.unicast_params.device_profile_id)) {
			if (dev_cfg.unicast_params.device_profile_id == SID_LINK2_PROFILE_2) {
				LOG_INF("sid_option returned %d (%s); Link_profile ID: %d Wndw_cnt: %d Rx_Int = %d",
					e, SID_ERROR_T_STR(e),
					dev_cfg.unicast_params.device_profile_id,
					dev_cfg.unicast_params.rx_window_count,
					dev_cfg.unicast_params.unicast_window_interval
						.sync_rx_interval_ms);
			} else {
				LOG_INF("sid_option returned %d (%s); Link_profile ID: %d Wndw_cnt: %d",
					e, SID_ERROR_T_STR(e),
					dev_cfg.unicast_params.device_profile_id,
					dev_cfg.unicast_params.rx_window_count);
			}
		}
	} break;
	default:
		LOG_INF("sid_option %d not supported", opt);
	}
}
void dut_event_set_option(sidewalk_ctx_t *sid, void *ctx)
{
	sidewalk_option_t *p_option = (sidewalk_option_t *)ctx;
	if (!p_option) {
		LOG_ERR("Invalid context!");
		return;
	}

	sid_error_t e =
		sid_option(sid->handle, p_option->option, p_option->data, p_option->data_len);
	LOG_INF("sid_option returned %d (%s)", e, SID_ERROR_T_STR(e));
}
void dut_event_set_dest_id(sidewalk_ctx_t *sid, void *ctx)
{
	uint32_t id = dut_ctx_get_uint32(ctx);
	sid_error_t e = sid_set_msg_dest_id(sid->handle, id);
	LOG_INF("sid_set_msg_dest_id returned %d (%s)", e, SID_ERROR_T_STR(e));
}
void dut_event_conn_req(sidewalk_ctx_t *sid, void *ctx)
{
	uint32_t event_req = dut_ctx_get_uint32(ctx);
	bool conn_req = (event_req == 1U);
	sid_error_t e = sid_ble_bcn_connection_request(sid->handle, conn_req);
	LOG_INF("sid_conn_request returned %d (%s)", e, SID_ERROR_T_STR(e));
}
