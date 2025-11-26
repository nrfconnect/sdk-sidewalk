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
#include <stdio.h>

LOG_MODULE_REGISTER(location_shell_events, CONFIG_SIDEWALK_LOG_LEVEL);

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

void location_event_alm_start(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	sid_error_t res = SID_ERROR_NOT_FOUND;
	// TODO: res = sid_pal_gnss_alm_demod_start();
	LOG_INF("location_event_alm_start returned %d", res);
}
