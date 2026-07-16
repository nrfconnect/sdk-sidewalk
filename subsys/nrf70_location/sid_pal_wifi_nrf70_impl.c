/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_pal_wifi_ifc.h>
#include <sid_pal_gnss_ifc.h>
#include <sid_error.h>

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include "nrf70_wifi_scan.h"

LOG_MODULE_REGISTER(sid_pal_wifi_nrf70, CONFIG_SIDEWALK_LOG_LEVEL);

#define WIFI_SCAN_TIMEOUT_MS CONFIG_SIDEWALK_WIFI_LOCATION_NRF70_SCAN_TIMEOUT_MS

static struct sid_pal_wifi_config wifi_config;

/* Access points kept for the last/ongoing scan, capped at the payload size. */
static struct nrf70_wifi_scan_ap scan_aps[SID_WIFI_MAX_RESULTS];

/* Results of the last completed scan, consumed by get_scan_payload(). */
static struct sid_pal_wifi_payload last_payload;

static bool scan_busy;    /* a scan request is in progress (delayed or running) */
static bool scan_started; /* the net_mgmt scan has actually been issued */
static int64_t scan_deadline;

/* Ask the library to call sid_pal_wifi_process_event() after delay_ms.
 * Safe to call from any thread: on_wifi_event posts to the Sidewalk event
 * queue. */
static void request_process_event(uint32_t delay_ms)
{
	if (wifi_config.on_wifi_event) {
		wifi_config.on_wifi_event(wifi_config.ctx, SID_PAL_WIFI_INTERNAL, delay_ms);
	}
}

/* Scanner completion callback - runs on the net_mgmt event thread. */
static void scan_done_wake(void)
{
	request_process_event(0);
}

/* Issue the net_mgmt scan and arm the backstop timeout. Runs on the library
 * thread (schedule_scan or, for a delayed scan, process_event). */
static sid_error_t start_scan_now(void)
{
	int ret = nrf70_wifi_scan_start(scan_aps, ARRAY_SIZE(scan_aps), scan_done_wake);

	if (ret) {
		LOG_ERR("nRF70 Wi-Fi PAL: failed to start scan (%d)", ret);
		return SID_ERROR_IO_ERROR;
	}

	scan_started = true;
	scan_deadline = k_uptime_get() + WIFI_SCAN_TIMEOUT_MS;

	/* Backstop only: real completion is signalled by scan_done_wake(). */
	request_process_event(WIFI_SCAN_TIMEOUT_MS);
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
		/* scan_aps is already sorted by descending RSSI and capped at
		 * SID_WIFI_MAX_RESULTS by the scanner. */
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

/* ---------------------------------------------------------------------------
 * Wi-Fi PAL
 * ------------------------------------------------------------------------- */

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
	nrf70_wifi_scan_abort();
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
		/* The requested scan delay has elapsed; start the scan now. */
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
		/* Woken but scan not done yet: only act if the backstop timeout
		 * has elapsed, otherwise wait for the real completion wake. */
		if (k_uptime_get() >= scan_deadline) {
			LOG_ERR("nRF70 Wi-Fi scan timed out");
			nrf70_wifi_scan_abort();
			finish_scan(0, -ETIMEDOUT);
		}
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
		/* Defer the scan: ask the library to call process_event() after
		 * the requested delay, where start_scan_now() will run. */
		LOG_INF("nRF70 Wi-Fi PAL: scan scheduled in %u s", scan_delay_s);
		request_process_event(scan_delay_s * 1000U);
	}

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_cancel_scan(void)
{
	/* Best-effort cancel. A scan that has not started yet (a deferred scan
	 * still waiting for its delay to elapse) is cancelled cleanly. A scan
	 * already running in the driver cannot be stopped in hardware (Zephyr
	 * has no scan-cancel request); we just detach the listener so its
	 * eventual completion is ignored and the PAL is freed for a new scan. */
	if (scan_busy) {
		nrf70_wifi_scan_abort();
		scan_busy = false;
		scan_started = false;
	}
	return SID_ERROR_NONE;
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

/* ---------------------------------------------------------------------------
 * GNSS PAL - not available on this platform.
 *
 * These stubs are required because, when this PAL replaces the
 * sx126x_location stub, the Sidewalk library still references the full GNSS
 * PAL surface. All functions report NOSUPPORT so the GNSS method degrades
 * gracefully.
 *
 * When SIDEWALK_SUBGHZ_RADIO_LR1110 is selected the LR1110 provides the real
 * GNSS PAL, so these stubs are compiled out to avoid duplicate sid_pal_gnss_*
 * symbols (the nRF70 keeps providing the Wi-Fi PAL alongside it).
 * ------------------------------------------------------------------------- */
#if !defined(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)

sid_error_t sid_pal_gnss_init(struct sid_pal_gnss_config *config)
{
	ARG_UNUSED(config);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_process_event(uint8_t event_id)
{
	ARG_UNUSED(event_id);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_schedule_scan(uint32_t scan_delay_s)
{
	ARG_UNUSED(scan_delay_s);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_cancel_scan(void)
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_get_scan_payload(struct sid_pal_gnss_payload *gnss_scan_group)
{
	ARG_UNUSED(gnss_scan_group);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_alm_demod_start(void)
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_deinit(void)
{
	return SID_ERROR_NOSUPPORT;
}

#endif /* !CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */
