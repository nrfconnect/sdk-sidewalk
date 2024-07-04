/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sidewalk.h>
#include <cli/sbdt_shell_events.h>
#include <cli/sbdt_shell.h>
#include <sid_bulk_data_transfer_api.h>
#include <sbdt/scratch_buffer.h>
#include <json_printer/sidTypes2str.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sid_sbdt_events, CONFIG_SIDEWALK_LOG_LEVEL);

void sbdt_event_init(sidewalk_ctx_t *sid, void *ctx)
{
	struct sbdt_context *context = (struct sbdt_context *)ctx;
	sid_error_t ret = sid_bulk_data_transfer_init(
		&(struct sid_bulk_data_transfer_config){ .callbacks = &context->ft_callbacks },
		sid->handle);
	scratch_buffer_init();
	LOG_INF("sid_bulk_data_transfer_init returned %d (%s)", ret, SID_ERROR_T_STR(ret));
}

void sbdt_event_deinit(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(ctx);
	sid_error_t ret = sid_bulk_data_transfer_deinit(sid->handle);
	scratch_bufer_deinit();
	LOG_INF("sid_bulk_data_transfer_deinit returned %d (%s)", ret, SID_ERROR_T_STR(ret));
}

void sbdt_event_cancel(sidewalk_ctx_t *sid, void *ctx)
{
	struct sbdt_cancel_ctx *context = (struct sbdt_cancel_ctx *)ctx;
	sid_error_t ret =
		sid_bulk_data_transfer_cancel(sid->handle, context->file_id, context->reason);
	LOG_INF("sidewalk_event_sbdt_cancel returned %d (%s)", ret, SID_ERROR_T_STR(ret));
}

void sbdt_event_print_stats(sidewalk_ctx_t *sid, void *ctx)
{
	uint32_t file_id = *(uint32_t *)ctx;
	struct sid_bulk_data_transfer_stats stats = {};
	sid_error_t ret = sid_bulk_data_transfer_get_transfer_stats(sid->handle, file_id, &stats);
	if (ret != SID_ERROR_NONE) {
		LOG_INF("sid_bulk_data_transfer_get_transfer_stats returned %d (%s)", ret,
			SID_ERROR_T_STR(ret));
	}
	LOG_INF("FILE_OFFSET: %d, FILE_PROGRESS: %.0d%%", stats.file_offset,
		stats.file_progress_percent);
}

void sbdt_event_print_params(sidewalk_ctx_t *sid, void *ctx)
{
	uint32_t file_id = *(uint32_t *)ctx;
	struct sid_bulk_data_transfer_params params = {};
	sid_error_t ret = sid_bulk_data_transfer_get_transfer_params(sid->handle, file_id, &params);
	if (ret != SID_ERROR_NONE) {
		// CMD_PRINT_RESULT(ret);
		LOG_ERR("sid_bulk_data_transfer_get_transfer_params returned %d, (%s)", ret,
			SID_ERROR_T_STR(ret));
		return;
	}
	struct sbdt_file_info *info = get_file_info_by_id(file_id);
	if (info == NULL) {
		LOG_ERR("Failed to retreive file info");
		return;
	}
	if (params.fragment_size != info->block_size) {
		LOG_ERR("BLOCK_SIZE: R:%u != P:%u", info->block_size, params.fragment_size);
	}
	if (params.file_size != info->file_size) {
		LOG_ERR("FILE_SIZE: R:%u != P:%u", info->file_size, params.file_size);
	}
	if (params.minimum_scratch_buffer_size != info->minimum_scratch_buffer_size) {
		LOG_ERR("MIN SCRATCH BUFFER SIZE: R:%u != P:%u", info->minimum_scratch_buffer_size,
			params.minimum_scratch_buffer_size);
	}

	if (params.scratch_buffer != info->scratch) {
		LOG_ERR("SCRATCH_BUFFER: R:%p != P:%p", info->scratch, params.scratch_buffer);
	}

	if (params.file_descriptor_size != info->file_descriptor_size) {
		LOG_ERR("FILE_DESCRIPTOR_SIZE: R:%u != P:%u", info->file_descriptor_size,
			params.file_descriptor_size);
	}

	if (params.file_descriptor_size &&
	    memcmp(params.file_descriptor, info->file_descriptor, params.file_descriptor_size)) {
		LOG_ERR("FILE_DESCRIPTOR DOES NOT MATCH");
	}

	LOG_INF("FILE_ID: %x, FILE_SIZE: %d, BLOCK_SIZE: %d, MIN_SCRATCH_SPACE: %u, "
		"FILE_DESCRIPTOR_SIZE: %d, SCRATCH_BUFFER_SIZE: %u",
		file_id, params.file_size, params.fragment_size, params.minimum_scratch_buffer_size,
		params.file_descriptor_size, params.scratch_buffer_size);

	if (params.file_descriptor_size) {
		LOG_HEXDUMP_INF(params.file_descriptor, params.file_descriptor_size,
				"FILE_DESCRIPTOR: ");
	}
}

void sbdt_event_finalize_request_response(sidewalk_ctx_t *sid, void *ctx)
{
	struct sbdt_finalize_resp_ctx *resp_event = (struct sbdt_finalize_resp_ctx *)ctx;
	sid_error_t result = sid_bulk_data_transfer_finalize(sid->handle, resp_event->file_id,
							     resp_event->finalize_response_action);
	LOG_INF("CMD: FINALIZE RESP:  ERR: %d", result);
}

void sbdt_event_release_buffer(sidewalk_ctx_t *sid, void *ctx)
{
	struct sbdt_buffer_release_ctx *release_event = ctx;
	sid_error_t result = sid_bulk_data_transfer_release_buffer(
		sid->handle, release_event->file_id, &release_event->transfer_buffer);
	LOG_INF("CMD: RELEASE BUFFER:  ERR: %d", result);
}
