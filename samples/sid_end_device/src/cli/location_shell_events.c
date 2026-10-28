/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_error.h>
#include <cli/location_shell_events.h>
#include <sidewalk.h>
#include <sid_location.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(location_shell_events, CONFIG_SIDEWALK_LOG_LEVEL);

/* Access points are reported packed as [1B rssi][6B bssid]. */
#define LOCATION_WIFI_AP_STRIDE 7
/* Upper bound on access points a single Wi-Fi result can carry. */
#define LOCATION_MAX_WIFI_APS (SID_LOCATION_MAX_PAYLOAD_SIZE / LOCATION_WIFI_AP_STRIDE)
/* Size of the synthetic payload used by the "send buffer" DUT command,
 * a whole number of access points so it is valid on the Wi-Fi path too.
 */
#define LOCATION_TEST_PAYLOAD_SIZE (7 * LOCATION_WIFI_AP_STRIDE)

static const char *location_mode_name(enum sid_location_effort_mode mode)
{
	switch (mode) {
	case SID_LOCATION_EFFORT_L1:
		return "L1 BLE gateway";
	case SID_LOCATION_EFFORT_L2:
		return "L2 FSK gateway";
	case SID_LOCATION_EFFORT_L3:
		return "L3 WiFi";
	case SID_LOCATION_EFFORT_L4:
		return "L4 GNSS";
	case SID_LOCATION_EFFORT_L5:
		return "L5 LoRa MLAT";
	case SID_LOCATION_EFFORT_L6:
		return "L6 GNSS GPS assist";
	default:
		return "unknown";
	}
}

static const char *location_link_name(enum sid_link_type link)
{
	switch (link) {
	case SID_LINK_TYPE_1:
		return "LINK_1 (BLE)";
	case SID_LINK_TYPE_2:
		return "LINK_2 (FSK)";
	case SID_LINK_TYPE_3:
		return "LINK_3 (LoRa)";
	default:
		return "unknown";
	}
}

static const char *location_status_name(enum sid_location_status status)
{
	switch (status) {
	case SID_LOCATION_LVL1_READY:
		return "LVL1_READY";
	case SID_LOCATION_LVL1_UNAVAILABLE:
		return "LVL1_UNAVAILABLE";
	case SID_LOCATION_SEND_ONLY_DONE:
		return "SEND_ONLY_DONE";
	case SID_LOCATION_SCAN_ONLY_DONE:
		return "SCAN_ONLY_DONE";
	case SID_LOCATION_SCAN_AND_SEND_DONE:
		return "SCAN_AND_SEND_DONE";
	case SID_LOCATION_RESOLVE_MSG_SENT:
		return "RESOLVE_MSG_SENT";
	case SID_LOCATION_RESOLVE_MSG_ACK_RECEIVED:
		return "RESOLVE_MSG_ACK_RECEIVED";
	case SID_LOCATION_RESOLVE_MSG_TIMEOUT:
		return "RESOLVE_MSG_TIMEOUT";
	default:
		return "unknown";
	}
}

void location_callback(const struct sid_location_result *const result, void *context)
{
	LOG_INF("loc send result: %d", result->err);
	LOG_INF("loc effort mode: %s (%d)", location_mode_name(result->mode), result->mode);
	LOG_INF("loc link type: %s (%d)", location_link_name(result->link), result->link);
	LOG_INF("loc status: %s (%d)", location_status_name(result->status), result->status);

	switch (result->result_type) {
	case SID_LOCATION_RESULT_WIFI: {
		struct sid_location_wifi_ap aps[LOCATION_MAX_WIFI_APS];
		size_t num_aps = 0;

		sid_error_t ret = sid_location_parse_wifi(&result->parsed.wifi, aps,
							  ARRAY_SIZE(aps), &num_aps);
		if (ret != SID_ERROR_NONE) {
			LOG_ERR("loc wifi parse failed %d", ret);
			break;
		}
		LOG_INF("loc wifi APs: %u", (unsigned int)num_aps);
		for (size_t i = 0; i < num_aps; i++) {
			LOG_INF("  rssi %d bssid %02x:%02x:%02x:%02x:%02x:%02x", aps[i].rssi,
				aps[i].bssid[0], aps[i].bssid[1], aps[i].bssid[2],
				aps[i].bssid[3], aps[i].bssid[4], aps[i].bssid[5]);
		}
		break;
	}
	case SID_LOCATION_RESULT_GNSS: {
		struct sid_location_gnss_info gnss = { 0 };

		sid_error_t ret = sid_location_parse_gnss(&result->parsed.gnss, &gnss);
		if (ret != SID_ERROR_NONE) {
			LOG_ERR("loc gnss parse failed %d", ret);
			break;
		}
		if (gnss.nav && gnss.nav_size) {
			LOG_HEXDUMP_INF(gnss.nav, gnss.nav_size, "loc gnss nav");
		}
		if (gnss.raw && gnss.raw_size) {
			LOG_HEXDUMP_INF(gnss.raw, gnss.raw_size, "loc gnss raw");
		}
		break;
	}
	case SID_LOCATION_RESULT_NONE:
	default:
		/* Gateway-trigger levels resolve in the cloud, no device payload. */
		break;
	}
}

static struct sid_location_config cfg = {
#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
	.sid_location_type_mask = SID_LOCATION_METHOD_ALL,
	.max_effort = SID_LOCATION_EFFORT_L4,
#elif defined(CONFIG_SIDEWALK_NRF7X_LOCATION)
	/* Let managed effort progress past BLE (L1) into the nRF70 Wi-Fi scan
	 * (L3) instead of stopping at the L1 default below. */
	.sid_location_type_mask = SID_LOCATION_METHOD_GATEWAY | SID_LOCATION_METHOD_WIFI,
	.max_effort = SID_LOCATION_EFFORT_L3,
#else
	.sid_location_type_mask = SID_LOCATION_METHOD_GATEWAY,
	.max_effort = SID_LOCATION_EFFORT_L1,
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */
	.manage_effort = true,
	/* GPS assist (L6) is not implemented in this sample: gps_assist_pal_config is left NULL
	 * as no GPS-assist PAL backend is available (SID_SDK_CONFIG_ENABLE_GPS_ASSIST is 0).
	 */
	.callbacks =
		{
			.on_update = location_callback,
		},
};

static uint32_t location_ctx_get_uint32(void *ctx)
{
	if (!ctx) {
		LOG_ERR("Invalid context!");
		return 0;
	}
	uint32_t ctx_val = *((uint32_t *)ctx);
	return ctx_val;
}

/* Location Event Handlers */
void location_event_init(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	sid_error_t res = SID_ERROR_NOT_FOUND;
	res = sid_location_init(sid->handle, &cfg);
	LOG_INF("location_event_init returned %d", res);
}

void location_event_deinit(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	sid_error_t res = SID_ERROR_NOT_FOUND;
	res = sid_location_deinit(sid->handle);
	LOG_INF("location_event_deinit returned %d", res);
}

void location_event_scan(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);

	enum sid_location_effort_mode mode = SID_LOCATION_EFFORT_DEFAULT;
	mode = (enum sid_location_effort_mode)location_ctx_get_uint32(ctx);

	struct sid_location_run_config config = { .type = SID_LOCATION_SCAN_ONLY, .mode = mode };

	sid_error_t res = SID_ERROR_NOT_FOUND;
	res = sid_location_run(sid->handle, &config, 0);
	LOG_INF("location_event_scan mode: %d, returned %d", mode, res);
}

void location_event_send(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);

	enum sid_location_effort_mode mode = SID_LOCATION_EFFORT_DEFAULT;
	mode = (enum sid_location_effort_mode)location_ctx_get_uint32(ctx);

	struct sid_location_run_config config = { .type = SID_LOCATION_SCAN_AND_SEND,
						  .mode = mode };

	sid_error_t res = SID_ERROR_NOT_FOUND;
	res = sid_location_run(sid->handle, &config, 0);
	LOG_INF("location_event_send mode: %d, returned %d", mode, res);
}

void location_event_send_levels(sidewalk_ctx_t *sid, void *ctx)
{
	const struct location_send_levels_params *params = ctx;

	if (!params) {
		LOG_ERR("Invalid context!");
		return;
	}

	struct sid_location_run_config config = { .type = SID_LOCATION_SCAN_AND_SEND,
						  .num_levels = params->num_levels };

	for (uint8_t i = 0; i < params->num_levels; i++) {
		config.levels[i] = (enum sid_location_effort_mode)params->levels[i];
	}

	sid_error_t res = sid_location_run(sid->handle, &config, 0);
	LOG_INF("location_event_send_levels levels: %u, returned %d", params->num_levels, res);
}

void location_event_send_buffer(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);

	enum sid_location_effort_mode mode = SID_LOCATION_EFFORT_DEFAULT;
	mode = (enum sid_location_effort_mode)location_ctx_get_uint32(ctx);

	uint8_t test_buffer[LOCATION_TEST_PAYLOAD_SIZE];
	memset(test_buffer, 0xFF, sizeof(test_buffer));

	struct sid_location_run_config config = {
		.type = SID_LOCATION_SEND_ONLY,
		.mode = mode,
	};

	/* The 2.0 API takes a typed payload instead of a raw buffer, so the test
	 * pattern is handed over on the path matching the requested effort level.
	 */
	switch (mode) {
	case SID_LOCATION_EFFORT_L3:
		config.payload_type = SID_LOCATION_RESULT_WIFI;
		config.payload.wifi.num_aps = LOCATION_TEST_PAYLOAD_SIZE / LOCATION_WIFI_AP_STRIDE;
		config.payload.wifi.aps = test_buffer;
		config.payload.wifi.aps_size = sizeof(test_buffer);
		break;
	case SID_LOCATION_EFFORT_L4:
		config.payload_type = SID_LOCATION_RESULT_GNSS;
		config.payload.gnss.nav = test_buffer;
		config.payload.gnss.nav_size = sizeof(test_buffer);
		break;
	default:
		LOG_ERR("send buffer supports only L3 and L4, got %d", mode);
		return;
	}

	sid_error_t res = SID_ERROR_NOT_FOUND;
	res = sid_location_run(sid->handle, &config, 0);
	LOG_INF("location_event_send_buffer mode: %d, returned %d", mode, res);
}

void location_event_send_pseudorange(sidewalk_ctx_t *sid, void *ctx)
{
	const struct location_send_pseudorange_params *params = ctx;

	if (!params) {
		LOG_ERR("Invalid context!");
		return;
	}

	sid_error_t res = sid_location_send_pseudoranges_raw(sid->handle, params->data,
							     params->size);
	LOG_INF("location_event_send_pseudorange size: %zu, returned %d", params->size, res);
}

void location_event_manage_effort(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);

	if (ctx) {
		cfg.manage_effort = location_ctx_get_uint32(ctx) != 0;
	}
	LOG_INF("manage_effort: %s%s", cfg.manage_effort ? "true" : "false",
		ctx ? " (applies on next location init)" : "");
}
