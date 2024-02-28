/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/sys/util.h>
#include <sid_bulk_data_transfer_api.h>
#include <sid_hal_memory_ifc.h>
#include <file_transfer.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <sidTypes2str.h>
#include <sidTypes2Json.h>

#include <sidewalk.h>

LOG_MODULE_REGISTER(file_transfer, CONFIG_SIDEWALK_LOG_LEVEL);

#define PARALEL_TRANSFER_MAX 3

K_HEAP_DEFINE(File_transfer_heap, KB(10));

struct buffer_repo_element {
	uint32_t file_id;
	void *ptr;
};

static struct buffer_repo_element buffer_repo[PARALEL_TRANSFER_MAX];

static void on_transfer_request(const struct sid_bulk_data_transfer_request *const transfer_request,
				struct sid_bulk_data_transfer_response *const transfer_response,
				void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(
		JSON_NAME("on_transfer_request", JSON_OBJ(JSON_VAL_sid_bulk_data_transfer_request(
							 "transfer_request", transfer_request))))));
	LOG_HEXDUMP_INF(transfer_request->file_descriptor, transfer_request->file_descriptor_size,
			"file_descriptor");
	size_t repo_index = UINT_MAX;
	for (size_t i = 0; i < PARALEL_TRANSFER_MAX; i++) {
		if (buffer_repo[i].ptr == NULL) {
			repo_index = i;
			break;
		}
	}
	if (repo_index > PARALEL_TRANSFER_MAX) {
		LOG_ERR("Failed to find slot for transfer");
		transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_REJECT;
		transfer_response->reject_reason = SID_BULK_DATA_TRANSFER_REJECT_REASON_GENERIC;
		return;
	}
	void *ptr = k_heap_alloc(&File_transfer_heap, transfer_request->minimum_scratch_buffer_size,
				 K_NO_WAIT);
	if (ptr == NULL) {
		LOG_ERR("Failed to alloc memory");
		transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_REJECT;
		transfer_response->reject_reason = SID_BULK_DATA_TRANSFER_REJECT_REASON_NO_SPACE;
		return;
	}

	// accept all requests if only we have avaliable memory for scratch buffer
	buffer_repo[repo_index].ptr = ptr;
	buffer_repo[repo_index].file_id = transfer_request->file_id;
	transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_ACCEPT;
	transfer_response->reject_reason = SID_BULK_DATA_TRANSFER_REJECT_REASON_NONE;
	transfer_response->scratch_buffer = ptr;
	transfer_response->scratch_buffer_size = transfer_request->minimum_scratch_buffer_size;
}

static void on_data_received(const struct sid_bulk_data_transfer_desc *const desc,
			     const struct sid_bulk_data_transfer_buffer *const buffer,
			     void *context)
{
	printk(JSON_NEW_LINE(
		JSON_OBJ(JSON_NAME("on_data_received",
				   JSON_OBJ(JSON_VAL_sid_bulk_data_transfer_desc("desc", desc))))));

	struct data_received_args *args =
		(struct data_received_args *)sid_hal_malloc(sizeof(struct data_received_args));
	args->desc = (struct sid_bulk_data_transfer_desc *)desc;
	args->buffer = (struct sid_bulk_data_transfer_buffer *)buffer;
	args->context = context;
	int err = sidewalk_event_send(SID_EVENT_FILE_TRANSFER, args);
	if (err) {
		sid_hal_free(args);
	}
}

static void on_finalize_request(uint32_t file_id, void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_finalize_request", JSON_OBJ(JSON_NAME("file_id", JSON_INT(file_id)))))));

	// Illustrative API indicating verification of file
	//     validate received file

	// always report success
	sid_error_t ret = sid_bulk_data_transfer_finalize(
		(struct sid_handle *)context, file_id, SID_BULK_DATA_TRANSFER_FINAL_STATUS_SUCCESS);
	if (ret != SID_ERROR_NONE) {
		LOG_ERR("sid_bulk_data_transfer_finalize returned %s", SID_ERROR_T_STR(ret));
	}
}

static void on_cancel_request(uint32_t file_id, void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_cancel_request", JSON_OBJ(JSON_NAME("file_id", JSON_INT(file_id)))))));
}

static void on_error(uint32_t file_id, void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(
		JSON_NAME("on_error", JSON_OBJ(JSON_NAME("file_id", JSON_INT(file_id)))))));
}

static void on_release_scratch_buffer(uint32_t file_id, void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_release_scratch_buffer", JSON_OBJ(JSON_NAME("file_id", JSON_INT(file_id)))))));

	for (size_t i = 0; i < PARALEL_TRANSFER_MAX; i++) {
		if (buffer_repo[i].file_id == file_id) {
			k_heap_free(&File_transfer_heap, buffer_repo[i].ptr);

			buffer_repo[i].ptr = NULL;
			buffer_repo[i].file_id = 0;
			return;
		}
	}
	LOG_ERR("failed to find file_id to be freed");
}

static struct sid_bulk_data_transfer_event_callbacks ft_callbacks = {
	.context = NULL,
	.on_transfer_request = on_transfer_request,
	.on_data_received = on_data_received,
	.on_finalize_request = on_finalize_request,
	.on_cancel_request = on_cancel_request,
	.on_error = on_error,
	.on_release_scratch_buffer = on_release_scratch_buffer
};

void app_file_transfer_demo_init(struct sid_handle *handle)
{
	ft_callbacks.context = (void *)handle;

	sid_error_t err = sid_bulk_data_transfer_init(
		&(struct sid_bulk_data_transfer_config){ .callbacks = &ft_callbacks }, handle);
	if (err != SID_ERROR_NONE) {
		LOG_ERR("sid_bulk_data_transfer_init returned %s", SID_ERROR_T_STR(err));
	}
}

void app_file_transfer_demo_deinit(struct sid_handle *handle)
{
	sid_error_t err = sid_bulk_data_transfer_deinit(handle);
	if (err != SID_ERROR_NONE) {
		LOG_ERR("sid_bulk_data_transfer_deinit returned %s", SID_ERROR_T_STR(err));
	}
}
