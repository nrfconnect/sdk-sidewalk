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
 */
struct nrf70_wifi_scan_ap {
	int8_t rssi;
	uint8_t mac[WIFI_MAC_ADDR_LEN];
};

/**
 * Completion callback, invoked once from the net_mgmt event thread when the
 * scan finishes. Must be lightweight.
 */
typedef void (*nrf70_wifi_scan_done_cb_t)(void);

/**
 * Start an asynchronous net_mgmt Wi-Fi scan.
 *
 * Returns immediately; the @p max_results strongest APs are collected into
 * @p results (descending RSSI) and @p on_done is called when the scan finishes.
 * Only one scan may be in flight, and @p results must stay valid until it
 * completes or nrf70_wifi_scan_release() is called.
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
 * Collect the results of a scan started by nrf70_wifi_scan_start(), typically
 * after the completion callback has fired.
 *
 * @param[out] stored Number of APs kept in the results array. May be NULL.
 * @param[out] total  Total number of APs reported by the driver. May be NULL.
 * @param[out] status Driver scan status (0 = success). Valid when done. May be NULL.
 *
 * @return 1 if the scan has completed, 0 if still running, negative errno on error.
 */
int nrf70_wifi_scan_get_result(size_t *stored, size_t *total, int *status);

/**
 * Stop listening for scan events and drop the reference to the results array.
 */
void nrf70_wifi_scan_release(void);

#ifdef __cplusplus
}
#endif

#endif /* NRF70_WIFI_SCAN_H */
