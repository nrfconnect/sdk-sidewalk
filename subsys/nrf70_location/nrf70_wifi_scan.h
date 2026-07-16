/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NRF70_WIFI_SCAN_H
#define NRF70_WIFI_SCAN_H

#include <stdint.h>
#include <stddef.h>
#include <zephyr/net/wifi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A single access point discovered by a net_mgmt Wi-Fi scan.
 *
 * Only the fields the Sidewalk Wi-Fi PAL payload actually carries (BSSID and
 * signal strength) are kept; SSID/channel/security are intentionally dropped to
 * keep the collection buffer small.
 */
struct nrf70_wifi_scan_ap {
	int8_t rssi;
	uint8_t mac[WIFI_MAC_ADDR_LEN];
	uint8_t mac_len;
};

/**
 * Completion callback, invoked once from the net_mgmt event thread when the
 * scan finishes. It must be lightweight (e.g. post an event); the actual
 * results are read later with nrf70_wifi_scan_get_result().
 */
typedef void (*nrf70_wifi_scan_done_cb_t)(void);

/**
 * Start an asynchronous net_mgmt Wi-Fi scan.
 *
 * Returns immediately after handing the request to the driver. Results stream
 * in on the net_mgmt event thread and are kept as the @p max_results strongest
 * access points (descending RSSI) in @p results. When the scan finishes,
 * @p on_done is called (from the net_mgmt thread) so the caller can wake up and
 * collect the results with nrf70_wifi_scan_get_result().
 *
 * Only one scan may be in flight at a time. @p results must stay valid until
 * the scan completes or nrf70_wifi_scan_abort() is called.
 *
 * @param[out] results     Array to keep the strongest APs (must be non-NULL).
 * @param[in]  max_results Capacity of @p results.
 * @param[in]  on_done     Completion callback (may be NULL).
 *
 * @return 0 on success, negative errno on failure.
 */
int nrf70_wifi_scan_start(struct nrf70_wifi_scan_ap *results, size_t max_results,
			  nrf70_wifi_scan_done_cb_t on_done);

/**
 * Collect the results of a scan started by nrf70_wifi_scan_start().
 *
 * Intended to be called after the completion callback has fired.
 *
 * @param[out] stored Number of APs kept in the results array. May be NULL.
 * @param[out] total  Total number of APs reported by the driver. May be NULL.
 * @param[out] status Driver scan status (0 = success). Valid when done. May be NULL.
 *
 * @return 1 if the scan has completed, 0 if still running, negative errno on error.
 */
int nrf70_wifi_scan_get_result(size_t *stored, size_t *total, int *status);

/**
 * Stop tracking an in-flight scan and release the net_mgmt callback.
 *
 * NOTE: Zephyr's Wi-Fi mgmt API has no scan-cancel request, so a scan already
 * issued to the driver cannot be stopped in hardware - it keeps running to
 * completion in the background. This only detaches the results listener and
 * frees the collection context, so the caller may reuse its results buffer.
 */
void nrf70_wifi_scan_abort(void);

#ifdef __cplusplus
}
#endif

#endif /* NRF70_WIFI_SCAN_H */
