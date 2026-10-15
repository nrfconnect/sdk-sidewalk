/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_error.h>
#include <cli/location_shell_events.h>
#include <sidewalk.h>
#include <sid_location.h>
#include <sid_sdk_config.h>
#if SID_SDK_CONFIG_ENABLE_GNSS
#include <sid_pal_gnss_ifc.h>
#endif
#include <sid_detect_unwanted_location_tracker.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdio.h>

LOG_MODULE_REGISTER(location_shell_events, CONFIG_SIDEWALK_LOG_LEVEL);

/* Access points are reported packed as [1B rssi][6B bssid]. */
#define LOCATION_WIFI_AP_STRIDE 7
/* Upper bound on access points a single Wi-Fi result can carry. */
#define LOCATION_MAX_WIFI_APS (SID_LOCATION_MAX_PAYLOAD_SIZE / LOCATION_WIFI_AP_STRIDE)
/* Size of the synthetic payload used by the "send buffer" DUT command,
 * a whole number of access points so it is valid on the Wi-Fi path too.
 */
#define LOCATION_TEST_PAYLOAD_SIZE (7 * LOCATION_WIFI_AP_STRIDE)

/* Data structures for DULT commands - must match location_shell.c */
struct dult_config_params {
	bool reset;
	char product_data[9]; /* 8 bytes + null terminator */
	char manufacturer_data[65]; /* 64 bytes + null terminator */
	char model_data[65]; /* 64 bytes + null terminator */
	char identifier[33]; /* 32 bytes + null terminator */
	uint16_t firmware_major;
	uint8_t firmware_minor;
	uint8_t firmware_revision;
	uint8_t category_id;
	uint8_t network_id;
	uint8_t battery_type;
	uint8_t advertising_mode;
	uint8_t initial_proximity;
};

struct dult_set_params {
	bool reset;
	uint8_t proximity_state;
	uint8_t ignore_non_owner_events;
	uint8_t non_owner_find_status;
	uint8_t identifier_read_mode;
	uint8_t battery_level;
};

void location_callback(const struct sid_location_result *const result, void *context)
{
	LOG_INF("loc send result: %d", result->err);
	LOG_INF("loc effort mode: %d", result->mode);
	LOG_INF("loc link type: %d", result->link);
	LOG_INF("loc status: %d", result->status);

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

/* DULT Configuration and State */
static struct dult_config {
	uint8_t product[64]; // SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_PRODUCT_DATA_MAX_SIZE + 1
	uint8_t man[64]; // SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_MANUFACTURER_NAME_MAX_SIZE + 1
	uint8_t model[64]; // SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_MODEL_NAME_MAX_SIZE + 1
	uint8_t ident[32]; // SID_DETECT_UNWANTED_LOCATION_TRACKER_IDENTIFIER_PAYLOAD_MAX_SIZE
	uint32_t firmware_version;
	uint8_t network_id;
	uint8_t category;
	uint8_t battery;
	uint8_t battery_level;
	uint8_t both_modes;
	uint8_t ident_size;
	uint8_t initial_proximity;
} dult_conf;

static bool ignore_non_owner_events = false;
static bool non_owner_find_status = true;

static void reset_dult_config(void)
{
	memset(&dult_conf, 0, sizeof(dult_conf));
	strcpy((char *)dult_conf.model, "sidewalk example");
	strcpy((char *)dult_conf.man, "sidewalk");
	strcpy((char *)dult_conf.product, "dut");
	dult_conf.firmware_version = 0x010000; // Major:1, Minor:0, Revision:0
	dult_conf.network_id = 3; // Sidewalk
	dult_conf.category = 1;
	dult_conf.battery = 0;
	dult_conf.battery_level = 0;
	dult_conf.both_modes = 1; // NEAR+SEPARATED mode
	dult_conf.initial_proximity = 1; // NEAR
	dult_conf.ident_size = 0;
}

static void print_dult_config(void)
{
	LOG_INF("DULT CONFIG:");
	LOG_INF("  PRODUCT_DATA: %s", dult_conf.product);
	LOG_INF("  MANUFACTURER: %s", dult_conf.man);
	LOG_INF("  MODEL: %s", dult_conf.model);
	LOG_INF("  FIRMWARE_VERSION: %d", dult_conf.firmware_version);
	LOG_INF("  NETWORK_ID: %d", dult_conf.network_id);
	LOG_INF("  CATEGORY_ID: %d", dult_conf.category);
	LOG_INF("  BATTERY_TYPE: %d", dult_conf.battery);
	LOG_INF("  ADVERTISEMENT: %d (%s)", dult_conf.both_modes,
		dult_conf.both_modes ? "NEAR+SEPARATED" : "SEPARATED");
	LOG_INF("  INITIAL PROXIMITY: %d (%s)", dult_conf.initial_proximity,
		dult_conf.initial_proximity ? "NEAR" : "SEPARATED");

	if (dult_conf.ident_size > 0) {
		LOG_INF("  IDENTIFIER: ");
		for (int i = 0; i < dult_conf.ident_size; i++) {
			LOG_INF("    %02x", dult_conf.ident[i]);
		}
	}
}

static void print_dult_set(void)
{
	LOG_INF("DULT SET:");
	LOG_INF("  IGNORE NON OWNER EVENTS: %d", ignore_non_owner_events);
	LOG_INF("  NON OWNER EVENT STATUS: %d", non_owner_find_status);
	LOG_INF("  BATTERY_LEVEL: %d", dult_conf.battery_level);
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
	if (mode == SID_LOCATION_EFFORT_L4) {
		config.payload_type = SID_LOCATION_RESULT_GNSS;
		config.payload.gnss.nav = test_buffer;
		config.payload.gnss.nav_size = sizeof(test_buffer);
	} else {
		config.payload_type = SID_LOCATION_RESULT_WIFI;
		config.payload.wifi.num_aps = LOCATION_TEST_PAYLOAD_SIZE / LOCATION_WIFI_AP_STRIDE;
		config.payload.wifi.aps = test_buffer;
		config.payload.wifi.aps_size = sizeof(test_buffer);
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

void location_event_alm_start(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

#if SID_SDK_CONFIG_ENABLE_GNSS
	sid_error_t res = sid_pal_gnss_alm_demod_start();
#else
	sid_error_t res = SID_ERROR_NOSUPPORT;
#endif
	LOG_INF("location_event_alm_start returned %d", res);
}

/* DULT Event Handlers */
void dult_event_init(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	sid_error_t res = SID_ERROR_NOT_FOUND;
	// Initialize DULT configuration and callbacks
	// TODO: res = sid_detect_unwanted_location_tracker_init(&config, sid->handle);
	LOG_INF("dult_event_init returned %d", res);
}

void dult_event_deinit(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	sid_error_t res = SID_ERROR_NOT_FOUND;
	// res = sid_detect_unwanted_location_tracker_deinit(sid->handle);
	LOG_INF("dult_event_deinit returned %d", res);
}

void dult_event_config(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);

	if (ctx == NULL) {
		// No arguments - just print current config
		print_dult_config();
		LOG_INF("dult_event_config - configuration display only");
		return;
	}

	struct dult_config_params *params = (struct dult_config_params *)ctx;

	if (params->reset) {
		reset_dult_config();
		LOG_INF("DULT config reset to default");
		return;
	}

	// Update configuration with provided parameters
	if (strlen(params->product_data) > 0) {
		strncpy((char *)dult_conf.product, params->product_data,
			sizeof(dult_conf.product) - 1);
	}
	if (strlen(params->manufacturer_data) > 0) {
		strncpy((char *)dult_conf.man, params->manufacturer_data,
			sizeof(dult_conf.man) - 1);
	}
	if (strlen(params->model_data) > 0) {
		strncpy((char *)dult_conf.model, params->model_data, sizeof(dult_conf.model) - 1);
	}
	if (strlen(params->identifier) > 0) {
		strncpy((char *)dult_conf.ident, params->identifier, sizeof(dult_conf.ident) - 1);
		dult_conf.ident_size = strlen(params->identifier);
	}

	// Update firmware version
	if (params->firmware_major > 0 || params->firmware_minor > 0 ||
	    params->firmware_revision > 0) {
		dult_conf.firmware_version = (params->firmware_major << 16) |
					     (params->firmware_minor << 8) |
					     params->firmware_revision;
	}

	// Update other parameters
	if (params->category_id > 0) {
		dult_conf.category = params->category_id;
	}
	if (params->network_id > 0) {
		dult_conf.network_id = params->network_id;
	}
	if (params->battery_type <= 2) {
		dult_conf.battery = params->battery_type;
	}
	if (params->advertising_mode <= 1) {
		dult_conf.both_modes = params->advertising_mode;
	}
	if (params->initial_proximity <= 1) {
		dult_conf.initial_proximity = params->initial_proximity;
	}

	LOG_INF("DULT config updated with parameters");
	print_dult_config();
}

void dult_event_set(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);

	if (ctx == NULL) {
		// No arguments - just print current set values
		print_dult_set();
		LOG_INF("dult_event_set - behavior display only");
		return;
	}

	struct dult_set_params *params = (struct dult_set_params *)ctx;

	if (params->reset) {
		ignore_non_owner_events = false;
		non_owner_find_status = true;
		dult_conf.battery_level = 0;
		LOG_INF("DULT set reset to default");
		return;
	}

	// Update behavior with provided parameters
	if (params->proximity_state <= 1) {
		// TODO: Update proximity state in DULT module
		LOG_INF("Proximity state set to: %d", params->proximity_state);
	}

	if (params->ignore_non_owner_events <= 1) {
		ignore_non_owner_events = (bool)params->ignore_non_owner_events;
	}

	if (params->non_owner_find_status <= 1) {
		non_owner_find_status = (bool)params->non_owner_find_status;
	}

	if (params->identifier_read_mode <= 1) {
		// TODO: Set identifier read mode in DULT module
		LOG_INF("Identifier read mode set to: %d", params->identifier_read_mode);
	}

	if (params->battery_level <= 3) {
		dult_conf.battery_level = params->battery_level;
	}

	LOG_INF("DULT behavior updated with parameters");
	print_dult_set();
}

/* Initialize DULT configuration on module load */
static void __attribute__((constructor)) dult_init_config(void)
{
	reset_dult_config();
}
