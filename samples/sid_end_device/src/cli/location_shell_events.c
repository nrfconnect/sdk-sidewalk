/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_error.h>
#include <cli/location_shell_events.h>
#include <sidewalk.h>
#include <sid_location.h>
#include <sid_detect_unwanted_location_tracker.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdio.h>

LOG_MODULE_REGISTER(location_shell_events, CONFIG_SIDEWALK_LOG_LEVEL);

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
	if (result->mode == SID_LOCATION_EFFORT_L3 || result->mode == SID_LOCATION_EFFORT_L4) {
		LOG_INF("loc payload:");
		LOG_HEXDUMP_INF(result->payload, result->size, "loc payload");
	}
}

static struct sid_location_config cfg = {
#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
	.sid_location_type_mask = SID_LOCATION_METHOD_ALL,
	.max_effort = SID_LOCATION_EFFORT_L4,
#else
	.sid_location_type_mask = SID_LOCATION_METHOD_BLE_GATEWAY,
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

void location_event_send_buffer(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);

	enum sid_location_effort_mode mode = SID_LOCATION_EFFORT_DEFAULT;
	mode = (enum sid_location_effort_mode)location_ctx_get_uint32(ctx);

	uint8_t test_buffer[50];
	memset(test_buffer, 0xFF, sizeof(test_buffer));

	struct sid_location_run_config config = {
		.type = SID_LOCATION_SEND_ONLY,
		.mode = mode,
		.buffer = test_buffer,
		.size = sizeof(test_buffer) / sizeof(test_buffer[0]),
	};

	sid_error_t res = SID_ERROR_NOT_FOUND;
	res = sid_location_run(sid->handle, &config, 0);
	LOG_INF("location_event_send_buffer mode: %d, returned %d", mode, res);
}

void location_event_alm_start(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	sid_error_t res = SID_ERROR_NOT_FOUND;
	// TODO: res = sid_pal_gnss_alm_demod_start();
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
