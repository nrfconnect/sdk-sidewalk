/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <cli/app_dut.h>
#include <sidewalk.h>
#include <sid_900_cfg.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_cli, CONFIG_SIDEWALK_LOG_LEVEL);

static uint32_t dut_ctx_get_uint32(void *ctx)
{
	if (!ctx) {
		LOG_ERR("Invalid context!");
		return 0;
	}
	uint32_t ctx_val = *((uint32_t *)ctx);
	sidewalk_data_free(ctx);
	return ctx_val;
}

void app_dut_event_process(sidewalk_ctx_event_t event, sidewalk_ctx_t *sid)
{
	switch ((cli_event_t)event.id) {
	case DUT_EVENT_INIT: {
		sid->config.link_mask = dut_ctx_get_uint32(event.ctx);
		sid_error_t e = sid_init(&sid->config, &sid->handle);
		LOG_INF("sid_init returned %d", e);
	} break;
	case DUT_EVENT_DEINIT: {
		if (event.ctx) {
			sidewalk_data_free(event.ctx);
			LOG_WRN("Unexpected context on event %d", event.id);
		};
		sid_error_t e = sid_deinit(sid->handle);
		LOG_INF("sid_deinit returned %d", e);
	} break;
	case DUT_EVENT_START: {
		uint32_t link_mask = dut_ctx_get_uint32(event.ctx);
		sid_error_t e = sid_start(sid->handle, link_mask);
		LOG_INF("sid_start returned %d", e);
	} break;
	case DUT_EVENT_STOP: {
		uint32_t link_mask = dut_ctx_get_uint32(event.ctx);
		sid_error_t e = sid_stop(sid->handle, link_mask);
		LOG_INF("sid_stop returned %d", e);
	} break;
	case DUT_EVENT_SET_CONN_REQ: {
		uint32_t event_req = dut_ctx_get_uint32(event.ctx);
		bool conn_req = (event_req == 1U);
		sid_error_t e = sid_ble_bcn_connection_request(sid->handle, conn_req);
		LOG_INF("sid_conn_request returned %d", e);
	} break;
	case DUT_EVENT_GET_MTU: {
		uint32_t link_mask = dut_ctx_get_uint32(event.ctx);
		size_t mtu = 0;
		sid_error_t e = sid_get_mtu(sid->handle, link_mask, &mtu);
		LOG_INF("sid_get_mtu returned %d, MTU: %d", e, mtu);
	} break;
	case DUT_EVENT_GET_TIME: {
		uint32_t format = dut_ctx_get_uint32(event.ctx);
		struct sid_timespec curr_time = { 0 };
		sid_error_t e = sid_get_time(sid->handle, format, &curr_time);
		LOG_INF("sid_get_time returned %d, SEC: %d NSEC: %d", e, curr_time.tv_sec,
			curr_time.tv_nsec);
	} break;
	case DUT_EVENT_SET_DEST_ID: {
		uint32_t id = dut_ctx_get_uint32(event.ctx);
		sid_error_t e = sid_set_msg_dest_id(sid->handle, id);
		LOG_INF("sid_set_msg_dest_id returned %d", e);
	} break;
	case DUT_EVENT_GET_OPTION: {
		if (!event.ctx) {
			LOG_ERR("Invalid context!");
			break;
		}
		sidewalk_option_t *p_lp_get = (sidewalk_option_t *)event.ctx;
		enum sid_option opt = p_lp_get->option;
		switch (opt) {
		case SID_OPTION_900MHZ_GET_DEVICE_PROFILE: {
			struct sid_device_profile dev_cfg = { 0 };
			enum sid_device_profile_id *p_id =
				(enum sid_device_profile_id *)p_lp_get->data;
			if (p_id) {
				dev_cfg.unicast_params.device_profile_id = *p_id;
				sidewalk_data_free(p_id);
			}

			sid_error_t e =
				sid_option(sid->handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
					   &dev_cfg, sizeof(struct sid_device_profile));

			if (IS_LINK2_PROFILE_ID(dev_cfg.unicast_params.device_profile_id) ||
			    IS_LINK3_PROFILE_ID(dev_cfg.unicast_params.device_profile_id)) {
				if (dev_cfg.unicast_params.device_profile_id ==
				    SID_LINK2_PROFILE_2) {
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
		case SID_OPTION_GET_MSG_POLICY_FILTER_DUPLICATES: {
			uint8_t data = 0;
			sid_error_t e = sid_option(sid->handle, opt, &data, sizeof(data));
			LOG_INF("sid_option returned %d; Filter Duplicates: %d", e, data);
		} break;
		default:
			LOG_INF("sid_option %d not supported", opt);
		}

		sidewalk_data_free(p_lp_get);
	} break;
	case DUT_EVENT_GET_STATUS: {
		LOG_INF("EVENT SID STATUS: State: %d, Reg: %d, Time: %d, Link_Mask: %x",
			sid->last_status.state, sid->last_status.detail.registration_status,
			sid->last_status.detail.time_sync_status,
			sid->last_status.detail.link_status_mask);
		LOG_INF("EVENT SID STATUS LINK MODE: LORA: %x, FSK: %x, BLE: %x",
			sid->last_status.detail.supported_link_modes[2],
			sid->last_status.detail.supported_link_modes[1],
			sid->last_status.detail.supported_link_modes[0]);
	} break;
	}
}
