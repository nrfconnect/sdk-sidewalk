/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 */

#include <sid_pal_wifi_ifc.h>
#include <sid_pal_log_ifc.h>
#include <smtc_modem_geolocation_api.h>
#include <smtc_modem.h>
#include <string.h>

#include "lr1110_location_priv.h"

#define LBM_STACK_ID 0

sid_error_t sid_pal_wifi_init(struct sid_pal_wifi_config *config)
{
	location_state.wifi_cfg = *config;
	return sid_pal_common_location_init();
}

sid_error_t sid_pal_wifi_process_event(uint8_t event_id)
{
	return sid_pal_common_process_event();
}

sid_error_t sid_pal_wifi_deinit(void)
{
	sid_pal_common_location_deinit();
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_schedule_scan(uint32_t scan_delay_s)
{
	if (location_state.is_busy) {
		SID_PAL_LOG_INFO("Location modem is busy, cannot schedule scan");
		return SID_ERROR_BUSY;
	}

	if (!location_state.is_init) {
		SID_PAL_LOG_INFO("PAL is uninitialized");
		return SID_ERROR_UNINITIALIZED;
	}

	location_state.is_busy = true;
	smtc_modem_wifi_scan(LBM_STACK_ID, scan_delay_s);
	sid_pal_common_process_event();

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_cancel_scan(void)
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_get_scan_payload(struct sid_pal_wifi_payload *wifi_scan_payload)
{
	uint8_t ap_idx = location_state.wifi_scan_done_data.nbr_results;

	if (ap_idx < 1) {
		return SID_ERROR_INSUFFICIENT_RESULTS;
	}

	for (uint8_t i = 0; i < location_state.wifi_scan_done_data.nbr_results; i++) {
		wifi_scan_payload->results[i].rssi =
			location_state.wifi_scan_done_data.results[i].rssi;
		memcpy(&(wifi_scan_payload->results[i].mac),
		       location_state.wifi_scan_done_data.results[i].mac_address,
		       SID_WIFI_MAC_ADDRESS_LENGTH);
	}

	wifi_scan_payload->nbr_results = location_state.wifi_scan_done_data.nbr_results;
	return SID_ERROR_NONE;
}
