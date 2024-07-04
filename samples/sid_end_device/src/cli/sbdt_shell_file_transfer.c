/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/crc.h>

#include <sidewalk.h>
#include <cli/sbdt_shell.h>
#include <cli/sbdt_shell_events.h>
#include <sbdt/scratch_buffer.h>
#include <sid_bulk_data_transfer_api.h>
#include <sid_hal_memory_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_assert_ifc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sid_sbdt_file_transfer, CONFIG_SIDEWALK_LOG_LEVEL);

#define FILE_TRANSFER_CRC_GROUP 0xB
#define FILE_TRANSFER_CRC_KEY 1

struct sbdt_file_info transfer_info[CONFIG_SBDT_MAX_PARALEL_TRANSFERS] = {};

struct sbdt_file_info *get_file_info_by_id(uint32_t file_id)
{
	for (size_t i = 0; i < CONFIG_SBDT_MAX_PARALEL_TRANSFERS; i++) {
		if (transfer_info[i].file_id == file_id && transfer_info[i].is_consumed) {
			return &transfer_info[i];
		}
	}
	return NULL;
}

static struct sbdt_file_info *allocate_new_info_instance(uint32_t file_id)
{
	for (size_t i = 0; i < CONFIG_SBDT_MAX_PARALEL_TRANSFERS; i++) {
		if (transfer_info[i].is_consumed == false) {
			transfer_info[i] = (struct sbdt_file_info){ .is_consumed = true,
								    .file_id = file_id,
								    .crc = 0 };
			return &transfer_info[i];
		}
	}
	return NULL;
}

static void release_info_instance(struct sbdt_file_info *info)
{
	if (info != NULL) {
		memset(info, 0, sizeof(*info));
	}
}

void on_sbdt_transfer_request(const struct sid_bulk_data_transfer_request *const transfer_request,
			      struct sid_bulk_data_transfer_response *const transfer_response,
			      void *context)
{
	struct sbdt_context *sbdt_context = (struct sbdt_context *)context;
	LOG_INF("EVENT SBDT TRANSFER REQUEST: FILE_ID: %x, FILE_SIZE: %d, FILE_OFFSET: %d, BLOCK_SIZE: %d, "
		"MIN_SCRATCH_SPACE: %u, "
		"FILE_DESCRIPTOR_SIZE: %d",
		transfer_request->file_id, transfer_request->file_size,
		transfer_request->file_offset, transfer_request->fragment_size,
		transfer_request->minimum_scratch_buffer_size,
		transfer_request->file_descriptor_size);

	if (transfer_request->file_descriptor_size) {
		SID_PAL_ASSERT(transfer_request->file_descriptor);
		LOG_HEXDUMP_INF(transfer_request->file_descriptor,
				transfer_request->file_descriptor_size,
				"EVENT SBDT TRANSFER REQUEST: FILE_DESCRIPTOR: ");
	}
	struct sbdt_file_info *info = get_file_info_by_id(transfer_request->file_id);
	if (info) {
		LOG_WRN("Trying to allocate new file transfer info under the same file ID. This will wipe the previous content.");
		release_info_instance(info);
	}
	info = allocate_new_info_instance(transfer_request->file_id);
	if (!info) {
		transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_REJECT;
		transfer_response->scratch_buffer = NULL;
		transfer_response->scratch_buffer_size = 0;
		LOG_ERR("Can not store new transfer info");
		return;
	}

	info->file_size = transfer_request->file_size;
	info->block_size = transfer_request->fragment_size;
	info->minimum_scratch_buffer_size = transfer_request->minimum_scratch_buffer_size;
	info->file_descriptor_size = transfer_request->file_descriptor_size;
	SID_PAL_ASSERT(info->file_descriptor_size < sizeof(info->file_descriptor));
	if (info->file_descriptor_size) {
		memcpy(info->file_descriptor, transfer_request->file_descriptor,
		       transfer_request->file_descriptor_size);
	}
	transfer_response->status = sbdt_context->transfer_request_action;
	transfer_response->reject_reason = sbdt_context->transfer_request_reject_reason;
	transfer_response->scratch_buffer = scratch_buffer_create(
		transfer_request->file_id, transfer_request->minimum_scratch_buffer_size);

	if (transfer_response->scratch_buffer == NULL) {
		release_info_instance(info);
		transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_REJECT;
		transfer_response->scratch_buffer = NULL;
		transfer_response->scratch_buffer_size = 0;
		LOG_ERR("Can not store new transfer");
		return;
	}
	transfer_response->scratch_buffer_size = transfer_request->minimum_scratch_buffer_size;

	sbdt_context->started_transfer = true;
}

static void on_sbdt_data_received_delayed(struct k_timer *timer)
{
	struct sbdt_buffer_release_ctx *ctx =
		CONTAINER_OF(timer, struct sbdt_buffer_release_ctx, delay);
	sidewalk_event_send(sbdt_event_release_buffer, ctx, sid_hal_free);
}

static void on_sbdt_data_received_stopped(struct k_timer *timer)
{
	struct sbdt_buffer_release_ctx *ctx =
		CONTAINER_OF(timer, struct sbdt_buffer_release_ctx, delay);
	sid_hal_free(ctx);
}

void on_sbdt_data_received(const struct sid_bulk_data_transfer_desc *const desc,
			   const struct sid_bulk_data_transfer_buffer *const buffer, void *context)
{
	struct sbdt_context *sbdt_context = (struct sbdt_context *)context;
	LOG_INF("EVENT SBDT DATA RECEIVED: FILE_ID: %x, FILE_OFFSET: 0x%x, LINK: %x", desc->file_id,
		desc->file_offset, desc->link_type);
	struct sbdt_file_info *info = get_file_info_by_id(desc->file_id);
	if (!info) {
		return;
	}
	if (desc->file_offset == 0) {
		sid_pal_storage_kv_group_delete(FILE_TRANSFER_CRC_GROUP);
	}
	uint32_t crc = 0;
	sid_error_t result = sid_pal_storage_kv_record_get(
		FILE_TRANSFER_CRC_GROUP, FILE_TRANSFER_CRC_KEY, &crc, sizeof(uint32_t));
	if (result == SID_ERROR_NOT_FOUND) {
		LOG_INF("CRC NOT FOUND IN FLASH!");
	} else if (result != SID_ERROR_NONE) {
		LOG_INF("CRC COULD NOT BE LOADED");
	}
	info->crc = crc;

	LOG_INF("SBDT PREV CRC: 0x%x", info->crc);
	info->file_offset = desc->file_offset;
	info->crc = crc32_ieee_update(info->crc, buffer->data, buffer->size);
	if (info->file_size == info->file_offset + buffer->size) {
		LOG_INF("EVENT SBDT FILE RECEIVED: FILE_ID: %x, FILE_SIZE: %u, FILE_CRC: 0x%x",
			info->file_id, info->file_size, info->crc);
	} else {
		LOG_INF("EVENT SBDT UPDATED CRC: 0x%x", info->crc);
	}
	result = sid_pal_storage_kv_record_set(FILE_TRANSFER_CRC_GROUP, FILE_TRANSFER_CRC_KEY,
					       &info->crc, sizeof(info->crc));
	if (result != SID_ERROR_NONE) {
		LOG_ERR("COULD NOT STORE CRC: %x", info->crc);
	}
	uint8_t *tmp = buffer->data;
	for (size_t i = 0; i < buffer->size; i += 217) {
		if (i + 217 > buffer->size) {
			LOG_INF("LB:%u F:%x L:%x", i, *(tmp + i), *(tmp + (buffer->size - 1)));
		} else {
			LOG_INF("B:%u F:%x L:%x", i, *(tmp + i), *(tmp + (i + 217 - 1)));
		}
	}
	struct sbdt_buffer_release_ctx *ctx =
		sid_hal_malloc((sizeof(struct sbdt_buffer_release_ctx)));
	if (ctx == NULL) {
		return;
	}
	ctx->file_id = desc->file_id;
	ctx->transfer_buffer = (struct sid_bulk_data_transfer_buffer){ .data = buffer->data,
								       .size = buffer->size };
	k_timer_init(&ctx->delay, on_sbdt_data_received_delayed, on_sbdt_data_received_stopped);
	k_timer_start(&ctx->delay, K_MSEC(sbdt_context->release_buffer_delay_ms), K_NO_WAIT);
}

static void on_sbdt_finalize_request_delayed(struct k_timer *timer)
{
	struct sbdt_finalize_resp_ctx *ctx =
		CONTAINER_OF(timer, struct sbdt_finalize_resp_ctx, delay);
	sidewalk_event_send(sbdt_event_finalize_request_response, ctx, sid_hal_free);
}

static void on_sbdt_finalize_request_stopped(struct k_timer *timer)
{
	struct sbdt_finalize_resp_ctx *ctx =
		CONTAINER_OF(timer, struct sbdt_finalize_resp_ctx, delay);
	sid_hal_free(ctx);
}

void on_sbdt_finalize_request(uint32_t file_id, void *context)
{
	struct sbdt_context *sbdt_context = (struct sbdt_context *)context;
	struct sbdt_finalize_resp_ctx *ctx = sid_hal_malloc(sizeof(struct sbdt_finalize_resp_ctx));
	*ctx = (struct sbdt_finalize_resp_ctx){ .file_id = file_id,
						.finalize_response_action =
							sbdt_context->finalize_response_action };
	k_timer_init(&ctx->delay, on_sbdt_finalize_request_delayed,
		     on_sbdt_finalize_request_stopped);
	k_timer_start(&ctx->delay, K_SECONDS(sbdt_context->finalize_response_delay_s), K_NO_WAIT);
}

void on_sbdt_cancel_request(uint32_t file_id, void *context)
{
	LOG_INF("EVENT SBDT CANCEL EVENT: FILE_ID: %x", file_id);
}

void on_sbdt_error(uint32_t file_id, void *context)
{
	LOG_INF("EVENT SBDT ERROR EVENT: FILE_ID: %x", file_id);
}

void on_release_scratch_buffer(uint32_t file_id, void *context)
{
	struct sbdt_context *sbdt_context = (struct sbdt_context *)context;
	LOG_INF("EVENT SBDT RELEASE SCRATCH EVENT: FILE_ID: %x", file_id);

	struct sbdt_file_info *info = get_file_info_by_id(file_id);
	release_info_instance(info);
	scratch_buffer_remove(file_id);
	sbdt_context->started_transfer = false;
}
