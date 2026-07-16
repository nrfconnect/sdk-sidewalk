/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nrf70_wifi_scan.h"

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nrf70_wifi_scan, CONFIG_SIDEWALK_LOG_LEVEL);

#define WIFI_SCAN_EVENTS (NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE)

static struct net_mgmt_event_callback scan_cb;
static bool cb_registered;
static nrf70_wifi_scan_done_cb_t scan_done_cb;

static struct {
	struct nrf70_wifi_scan_ap *results;
	size_t max_results;
	size_t stored;
	size_t total;
	int status;
} scan_ctx;

static atomic_t scan_done = ATOMIC_INIT(0);

static struct net_if *wifi_iface(void)
{
	struct net_if *iface = net_if_get_first_wifi();

	return iface ? iface : net_if_get_default();
}

/* Insert an AP into the results array, keeping it sorted by descending RSSI and
 * capped at max_results (weakest dropped). */
static void scan_result_insert(int8_t rssi, const uint8_t *mac, uint8_t mac_len)
{
	struct nrf70_wifi_scan_ap *r = scan_ctx.results;
	size_t max = scan_ctx.max_results;
	size_t pos;

	if (!r || max == 0) {
		return;
	}

	if (scan_ctx.stored < max) {
		pos = scan_ctx.stored++;
	} else if (rssi > r[max - 1].rssi) {
		pos = max - 1;
	} else {
		return;
	}

	for (; pos > 0 && r[pos - 1].rssi < rssi; pos--) {
		r[pos] = r[pos - 1];
	}

	memset(&r[pos], 0, sizeof(r[pos]));
	r[pos].rssi = rssi;
	r[pos].mac_len = MIN(mac_len, (uint8_t)WIFI_MAC_ADDR_LEN);
	memcpy(r[pos].mac, mac, r[pos].mac_len);
}

static void scan_result_handle(struct net_mgmt_event_callback *cb)
{
	const struct wifi_scan_result *entry = (const struct wifi_scan_result *)cb->info;

	scan_ctx.total++;
	scan_result_insert(entry->rssi, entry->mac, entry->mac_length);
}

static void scan_done_handle(struct net_mgmt_event_callback *cb)
{
	const struct wifi_status *status = (const struct wifi_status *)cb->info;

	scan_ctx.status = status->status;
	atomic_set(&scan_done, 1);

	if (scan_done_cb) {
		scan_done_cb();
	}
}

static void scan_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event,
				    struct net_if *iface)
{
	ARG_UNUSED(iface);

	switch (mgmt_event) {
	case NET_EVENT_WIFI_SCAN_RESULT:
		scan_result_handle(cb);
		break;
	case NET_EVENT_WIFI_SCAN_DONE:
		scan_done_handle(cb);
		break;
	default:
		break;
	}
}

int nrf70_wifi_scan_start(struct nrf70_wifi_scan_ap *results, size_t max_results,
			  nrf70_wifi_scan_done_cb_t on_done)
{
	struct net_if *iface = wifi_iface();
	int ret;

	if (!results || max_results == 0) {
		return -EINVAL;
	}
	if (!iface) {
		return -ENODEV;
	}
	if (!net_if_is_admin_up(iface)) {
		return -ENETDOWN;
	}

	scan_ctx.results = results;
	scan_ctx.max_results = max_results;
	scan_ctx.stored = 0;
	scan_ctx.total = 0;
	scan_ctx.status = 0;
	scan_done_cb = on_done;
	atomic_set(&scan_done, 0);

	net_mgmt_init_event_callback(&scan_cb, scan_mgmt_event_handler, WIFI_SCAN_EVENTS);
	net_mgmt_add_event_callback(&scan_cb);
	cb_registered = true;

	struct wifi_scan_params params = {
#if defined(CONFIG_SIDEWALK_WIFI_LOCATION_NRF70_SCAN_ACTIVE)
		.scan_type = WIFI_SCAN_TYPE_ACTIVE,
#elif defined(CONFIG_SIDEWALK_WIFI_LOCATION_NRF70_SCAN_PASSIVE)
		.scan_type = WIFI_SCAN_TYPE_PASSIVE,
#endif
#if defined(CONFIG_SIDEWALK_WIFI_LOCATION_NRF70_SCAN_BAND_2_4GHZ)
		.bands = BIT(WIFI_FREQ_BAND_2_4_GHZ),
#elif defined(CONFIG_SIDEWALK_WIFI_LOCATION_NRF70_SCAN_BAND_2_4_AND_5_GHZ)
		.bands = BIT(WIFI_FREQ_BAND_2_4_GHZ) | BIT(WIFI_FREQ_BAND_5_GHZ),
#endif
	};

	ret = net_mgmt(NET_REQUEST_WIFI_SCAN, iface, &params, sizeof(params));
	if (ret) {
		nrf70_wifi_scan_abort();
		return ret;
	}

	return 0;
}

int nrf70_wifi_scan_get_result(size_t *stored, size_t *total, int *status)
{
	if (!cb_registered) {
		return -EINVAL;
	}
	if (!atomic_get(&scan_done)) {
		return 0;
	}

	net_mgmt_del_event_callback(&scan_cb);
	cb_registered = false;

	if (stored) {
		*stored = scan_ctx.stored;
	}
	if (total) {
		*total = scan_ctx.total;
	}
	if (status) {
		*status = scan_ctx.status;
	}

	return 1;
}

void nrf70_wifi_scan_abort(void)
{
	/* Zephyr's Wi-Fi mgmt API has no scan-cancel request, so a scan already
	 * running in the driver cannot be stopped; it will run to completion in
	 * the background. All we can do is stop listening for its results and
	 * release the collection context. */
	if (cb_registered) {
		net_mgmt_del_event_callback(&scan_cb);
		cb_registered = false;
	}
	scan_ctx.results = NULL;
	scan_ctx.max_results = 0;
	atomic_set(&scan_done, 0);
}
