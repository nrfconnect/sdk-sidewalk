/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sidewalk.h>
#include <sid_bulk_data_transfer_api.h>

void sbdt_event_init(sidewalk_ctx_t *sid, void *ctx);
void sbdt_event_deinit(sidewalk_ctx_t *sid, void *ctx);
void sbdt_event_cancel(sidewalk_ctx_t *sid, void *ctx);

void sbdt_event_print_stats(sidewalk_ctx_t *sid, void *ctx);
void sbdt_event_print_params(sidewalk_ctx_t *sid, void *ctx);
void sbdt_event_finalize_request_response(sidewalk_ctx_t *sid, void *ctx);
void sbdt_event_release_buffer(sidewalk_ctx_t *sid, void *ctx);

struct sbdt_cancel_ctx {
	int file_id;
	enum sid_bulk_data_transfer_reject_reason reason;
};

struct sbdt_finalize_resp_ctx {
	int file_id;
	uint8_t finalize_response_action;
	struct k_timer delay;
};
struct sbdt_buffer_release_ctx {
	int file_id;
	struct sid_bulk_data_transfer_buffer transfer_buffer;
	struct k_timer delay;
};
