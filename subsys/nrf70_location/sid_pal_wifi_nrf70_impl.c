/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_pal_wifi_ifc.h>
#include <sid_error.h>

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include "nrf70_wifi_scan.h"

LOG_MODULE_REGISTER(sid_pal_wifi_nrf70, CONFIG_SIDEWALK_LOG_LEVEL);

static struct sid_pal_wifi_config wifi_config;
static struct nrf70_wifi_scan_ap scan_aps[SID_WIFI_MAX_RESULTS];
static struct sid_pal_wifi_payload last_payload;

static bool scan_busy;    /* a scan request is in progress (delayed or running) */
static bool scan_started; /* the net_mgmt scan has actually been issued */

/* Safe to call from any thread. */
static void request_process_event(uint32_t delay_ms)
{
	if (wifi_config.on_wifi_event) {
		wifi_config.on_wifi_event(wifi_config.ctx, SID_PAL_WIFI_INTERNAL, delay_ms);
	}
}

/* Runs on the net_mgmt event thread. */
static void scan_done_wake(void)
{
	request_process_event(0);
}

static sid_error_t start_scan_now(void)
{
	int ret = nrf70_wifi_scan_start(scan_aps, ARRAY_SIZE(scan_aps), scan_done_wake);

	if (ret) {
		LOG_ERR("nRF70 Wi-Fi PAL: failed to start scan (%d)", ret);
		return SID_ERROR_IO_ERROR;
	}

	scan_started = true;

	return SID_ERROR_NONE;
}

static void finish_scan(size_t stored, int status)
{
	memset(&last_payload, 0, sizeof(last_payload));

	if (status != 0) {
		LOG_ERR("nRF70 Wi-Fi scan failed (status %d)", status);
	} else if (stored == 0) {
		LOG_WRN("nRF70 Wi-Fi scan found no access points");
	} else {
		/* Already sorted by descending RSSI and capped by the scanner. */
		uint8_t n = (uint8_t)MIN(stored, (size_t)SID_WIFI_MAX_RESULTS);

		for (uint8_t i = 0; i < n; i++) {
			/* PAL contract stores RSSI as unsigned. */
			last_payload.results[i].rssi = (uint8_t)scan_aps[i].rssi;
			memcpy(last_payload.results[i].mac, scan_aps[i].mac,
			       SID_WIFI_MAC_ADDRESS_LENGTH);
		}
		last_payload.nbr_results = n;
		LOG_INF("nRF70 Wi-Fi PAL: collected %u AP(s)", n);
	}

	scan_busy = false;
	scan_started = false;

	if (wifi_config.on_wifi_event) {
		wifi_config.on_wifi_event(wifi_config.ctx, SID_PAL_WIFI_SCAN_COMPLETE, 0);
	}
}

sid_error_t sid_pal_wifi_init(struct sid_pal_wifi_config *config)
{
	if (!config) {
		return SID_ERROR_NULL_POINTER;
	}

	wifi_config = *config;

	LOG_INF("nRF70 Wi-Fi PAL initialized");
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_deinit(void)
{
	nrf70_wifi_scan_release();
	scan_busy = false;
	scan_started = false;
	memset(&wifi_config, 0, sizeof(wifi_config));
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_process_event(uint8_t event_id)
{
	ARG_UNUSED(event_id);

	if (!scan_busy) {
		return SID_ERROR_NONE;
	}

	if (!scan_started) {
		/* The requested scan delay has elapsed. */
		if (start_scan_now() != SID_ERROR_NONE) {
			finish_scan(0, -EIO);
		}
		return SID_ERROR_NONE;
	}

	size_t stored = 0;
	size_t total = 0;
	int status = 0;

	int ret = nrf70_wifi_scan_get_result(&stored, &total, &status);

	if (ret == 0) {
		/* Not done yet; wait for the scan-done event. */
		return SID_ERROR_NONE;
	}

	if (ret < 0) {
		finish_scan(0, ret);
		return SID_ERROR_NONE;
	}

	LOG_INF("nRF70 Wi-Fi PAL: scan complete (%u of %u AP(s))",
		(unsigned)stored, (unsigned)total);
	finish_scan(stored, status);
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_schedule_scan(uint32_t scan_delay_s)
{
	if (!wifi_config.on_wifi_event) {
		return SID_ERROR_UNINITIALIZED;
	}

	if (scan_busy) {
		LOG_WRN("nRF70 Wi-Fi PAL: scan already in progress");
		return SID_ERROR_BUSY;
	}

	scan_busy = true;
	scan_started = false;

	if (scan_delay_s == 0) {
		sid_error_t err = start_scan_now();

		if (err != SID_ERROR_NONE) {
			scan_busy = false;
			return err;
		}
		LOG_INF("nRF70 Wi-Fi PAL: scan scheduled");
	} else {
		/* process_event() runs start_scan_now() once the delay has elapsed. */
		LOG_INF("nRF70 Wi-Fi PAL: scan scheduled in %u s", scan_delay_s);
		request_process_event(scan_delay_s * 1000U);
	}

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_cancel_scan(void)
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_get_scan_payload(struct sid_pal_wifi_payload *wifi_scan_result)
{
	if (!wifi_scan_result) {
		return SID_ERROR_NULL_POINTER;
	}

	if (last_payload.nbr_results == 0) {
		return SID_ERROR_INSUFFICIENT_RESULTS;
	}

	*wifi_scan_result = last_payload;

	LOG_INF("nRF70 Wi-Fi PAL: reporting %u AP(s) to Sidewalk",
		wifi_scan_result->nbr_results);
	return SID_ERROR_NONE;
}
