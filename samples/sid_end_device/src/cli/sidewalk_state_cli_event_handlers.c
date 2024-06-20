/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sidewalk.h>
#include <sid_900_cfg.h>
#include <cli/app_shell_events.h>
#include <zephyr/logging/log.h>
#include <sid_hal_memory_ifc.h>
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
#include <file_transfer.h>
#endif

LOG_MODULE_DECLARE(sidewalk_fsm_event_handlers, CONFIG_SIDEWALK_FSM_EVENTS_LOG_LEVEL);

static uint32_t dut_ctx_get_uint32(void *ctx);
static void dut_option_set(sidewalk_option_t *p_option, struct sid_handle *handle);
static void dut_option_get(sidewalk_option_t *p_option, struct sid_handle *handle);

/* --- Event handlers ---*/

void dut_sidewalk_event_init_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	sm->sid->config.link_mask = dut_ctx_get_uint32(ctx);
	sid_error_t e = sid_init(&sm->sid->config, &sm->sid->handle);
	LOG_INF("sid_init returned %d", e);
	if (e != SID_ERROR_NONE) {
		return;
	}
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
	app_file_transfer_demo_init(sm->sid->handle);
#endif
}

void dut_sidewalk_event_deinit_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	if (ctx) {
		sid_hal_free(ctx);
		LOG_ERR("Did not expect context");
	};
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER
	app_file_transfer_demo_deinit(sm->sid->handle);
#endif
	sid_process(sm->sid->handle);
	sid_error_t e = sid_deinit(sm->sid->handle);
	LOG_INF("sid_deinit returned %d", e);
}

void dut_sidewalk_event_start_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	uint32_t link_mask = dut_ctx_get_uint32(ctx);
	sid_error_t e = sid_start(sm->sid->handle, link_mask);
	LOG_INF("sid_start returned %d", e);
}

void dut_sidewalk_event_stop_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	uint32_t link_mask = dut_ctx_get_uint32(ctx);
	sid_process(sm->sid->handle);
	sid_error_t e = sid_stop(sm->sid->handle, link_mask);
	LOG_INF("sid_stop returned %d", e);
}

void dut_sidewalk_event_get_mtu_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	uint32_t link_mask = dut_ctx_get_uint32(ctx);
	size_t mtu = 0;
	sid_error_t e = sid_get_mtu(sm->sid->handle, (enum sid_link_type)link_mask, &mtu);
	LOG_INF("sid_get_mtu returned %d, MTU: %d", e, mtu);
}

void dut_sidewalk_event_get_time_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	uint32_t format = dut_ctx_get_uint32(ctx);
	struct sid_timespec curr_time = { 0 };
	sid_error_t e = sid_get_time(sm->sid->handle, (enum sid_time_format)format, &curr_time);
	LOG_INF("sid_get_time returned %d, SEC: %d NSEC: %d", e, curr_time.tv_sec,
		curr_time.tv_nsec);
}

void dut_sidewalk_event_get_status_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	LOG_INF("Device %sregistered, Time Sync %s, Link status: {BLE: %s, FSK: %s, LoRa: %s}",
		(SID_STATUS_REGISTERED == sm->sid->last_status.detail.registration_status) ? "Is " :
											     "Un",
		(SID_STATUS_TIME_SYNCED == sm->sid->last_status.detail.time_sync_status) ?
			"Success" :
			"Fail",
		(sm->sid->last_status.detail.link_status_mask & SID_LINK_TYPE_1) ? "Up" : "Down",
		(sm->sid->last_status.detail.link_status_mask & SID_LINK_TYPE_2) ? "Up" : "Down",
		(sm->sid->last_status.detail.link_status_mask & SID_LINK_TYPE_3) ? "Up" : "Down");

	for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
		enum sid_link_mode mode =
			(enum sid_link_mode)sm->sid->last_status.detail.supported_link_modes[i];

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

void dut_sidewalk_event_get_option_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	dut_option_get((sidewalk_option_t *)ctx, sm->sid->handle);
}

void dut_sidewalk_event_set_option_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	dut_option_set((sidewalk_option_t *)ctx, sm->sid->handle);
}

void dut_sidewalk_event_set_dest_id_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	uint32_t id = dut_ctx_get_uint32(ctx);
	sid_error_t e = sid_set_msg_dest_id(sm->sid->handle, id);
	LOG_INF("sid_set_msg_dest_id returned %d", e);
}

void dut_sidewalk_event_set_conn_req_handler(void *ctx, void *state_machine)
{
	sm_t *sm = (sm_t *)state_machine;
	uint32_t event_req = dut_ctx_get_uint32(ctx);
	bool conn_req = (event_req == 1U);
	sid_error_t e = sid_ble_bcn_connection_request(sm->sid->handle, conn_req);
	LOG_INF("sid_conn_request returned %d", e);
}

/* --- Helper functions --- */
static uint32_t dut_ctx_get_uint32(void *ctx)
{
	if (!ctx) {
		LOG_ERR("Invalid context!");
		return 0;
	}
	uint32_t ctx_val = *((uint32_t *)ctx);
	sid_hal_free(ctx);
	return ctx_val;
}

static void dut_option_set(sidewalk_option_t *p_option, struct sid_handle *handle)
{
	if (!p_option) {
		LOG_ERR("Invalid context!");
		return;
	}

	sid_error_t e = sid_option(handle, p_option->option, p_option->data, p_option->data_len);
	if (p_option->data) {
		sid_hal_free(p_option->data);
	}
	sid_hal_free(p_option);

	LOG_INF("sid_option returned %d", e);
}

static void dut_option_get(sidewalk_option_t *p_option, struct sid_handle *handle)
{
	if (!p_option) {
		LOG_ERR("Invalid context!");
		return;
	}

	enum sid_option opt = p_option->option;
	switch (opt) {
	case SID_OPTION_GET_MSG_POLICY_FILTER_DUPLICATES: {
		uint8_t data = 0;
		sid_error_t e = sid_option(handle, opt, &data, sizeof(data));
		LOG_INF("sid_option returned %d; Filter Duplicates: %d", e, data);
	} break;
	case SID_OPTION_GET_LINK_CONNECTION_POLICY: {
		uint8_t data = 0;
		sid_error_t e = sid_option(handle, opt, &data, sizeof(data));
		LOG_INF("sid_option returned %d; Link Connect Policy: %d", e, data);
	} break;
	case SID_OPTION_GET_LINK_POLICY_MULTI_LINK_POLICY: {
		uint8_t data = 0;
		sid_error_t e = sid_option(handle, opt, &data, sizeof(data));
		LOG_INF("sid_option returned %d; Link Multi Link Policy: %d", e, data);
	} break;
	case SID_OPTION_GET_STATISTICS: {
		struct sid_statistics stats = { 0 };
		sid_error_t e = sid_option(handle, opt, &stats, sizeof(stats));
		LOG_INF("sid_option returned %d; tx: %d, acks_sent %d, tx_fail: %d, retries: %d, dups: %d, acks_recv: %d rx: %d",
			e, stats.msg_stats.tx, stats.msg_stats.acks_sent, stats.msg_stats.tx_fail,
			stats.msg_stats.retries, stats.msg_stats.duplicates,
			stats.msg_stats.acks_recv, stats.msg_stats.rx);
	} break;
	case SID_OPTION_GET_LINK_POLICY_AUTO_CONNECT_PARAMS: {
		struct sid_link_auto_connect_params params = { 0 };
		memcpy(&params.link_type, p_option->data, sizeof(uint32_t));
		sid_error_t e = sid_option(handle, opt, (void *)&params, sizeof(params));
		LOG_INF("sid_option returned %d; AC Policy, link %d, enable %d priority %d timeout %d",
			e, params.link_type, params.enable, params.priority,
			params.connection_attempt_timeout_seconds);
	} break;
	case SID_OPTION_900MHZ_GET_DEVICE_PROFILE: {
		struct sid_device_profile dev_cfg = { 0 };
		if (p_option->data) {
			dev_cfg.unicast_params.device_profile_id =
				*((enum sid_device_profile_id *)p_option->data);
		}

		sid_error_t e = sid_option(handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE, &dev_cfg,
					   sizeof(struct sid_device_profile));

		if (IS_LINK2_PROFILE_ID(dev_cfg.unicast_params.device_profile_id) ||
		    IS_LINK3_PROFILE_ID(dev_cfg.unicast_params.device_profile_id)) {
			if (dev_cfg.unicast_params.device_profile_id == SID_LINK2_PROFILE_2) {
				LOG_INF("sid_option returned %d; Link_profile ID: %d Wndw_cnt: %d Rx_Int = %d",
					e, dev_cfg.unicast_params.device_profile_id,
					dev_cfg.unicast_params.rx_window_count,
					dev_cfg.unicast_params.unicast_window_interval
						.sync_rx_interval_ms);
			} else {
				LOG_INF("sid_option returned %d; Link_profile ID: %d Wndw_cnt: %d",
					e, dev_cfg.unicast_params.device_profile_id,
					dev_cfg.unicast_params.rx_window_count);
			}
		}
	} break;
	default:
		LOG_INF("sid_option %d not supported", opt);
	}

	if (p_option->data) {
		sid_hal_free(p_option->data);
	}
	sid_hal_free(p_option);
}
