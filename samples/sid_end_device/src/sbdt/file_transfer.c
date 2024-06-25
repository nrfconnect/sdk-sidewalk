/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sbdt/file_transfer.h>
#include <sbdt/scratch_buffer.h>
#include <stdint.h>
#include <zephyr/sys/util.h>
#include <sid_bulk_data_transfer_api.h>
#include <sid_hal_memory_ifc.h>
#include <sid_pal_crypto_ifc.h>

#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/logging/log.h>
#include <json_printer/sidTypes2str.h>

#include <sidewalk.h>
#include <sid_hal_memory_ifc.h>
#include <json_printer/sidTypes2Json.h>
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
#include <sidewalk_dfu/nordic_dfu_img.h>
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */

#include <sid_bulk_data_transfer_api.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(file_transfer, CONFIG_SIDEWALK_LOG_LEVEL);

static void on_transfer_request(const struct sid_bulk_data_transfer_request *const transfer_request,
				struct sid_bulk_data_transfer_response *const transfer_response,
				void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(
		JSON_NAME("on_transfer_request", JSON_OBJ(JSON_VAL_sid_bulk_data_transfer_request(
							 "transfer_request", transfer_request))))));
	LOG_HEXDUMP_INF(transfer_request->file_descriptor, transfer_request->file_descriptor_size,
			"file_descriptor");

#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	int dfu_err = nordic_dfu_img_init();
	if (dfu_err) {
		LOG_ERR("dfu img init fail %d", dfu_err);
		transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_REJECT;
		transfer_response->reject_reason = SID_BULK_DATA_TRANSFER_REJECT_REASON_GENERIC;
		transfer_response->scratch_buffer_size = 0;
		return;
	}
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */

	transfer_response->scratch_buffer = scratch_buffer_create(
		transfer_request->file_id, transfer_request->minimum_scratch_buffer_size);

	if (!transfer_response->scratch_buffer) {
		transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_REJECT;
		transfer_response->reject_reason = SID_BULK_DATA_TRANSFER_REJECT_REASON_NO_SPACE;
		transfer_response->scratch_buffer_size = 0;
		return;
	}

	transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_ACCEPT;
	transfer_response->reject_reason = SID_BULK_DATA_TRANSFER_REJECT_REASON_NONE;
	transfer_response->scratch_buffer_size = transfer_request->minimum_scratch_buffer_size;
}

static void on_data_received(const struct sid_bulk_data_transfer_desc *const desc,
			     const struct sid_bulk_data_transfer_buffer *const buffer,
			     void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(
		JSON_LIST_2(JSON_NAME("on_data_received",
				      JSON_OBJ(JSON_VAL_sid_bulk_data_transfer_desc("desc", desc))),
			    JSON_NAME("data_size", JSON_INT(buffer->size))))));

	sidewalk_transfer_t *transfer =
		(sidewalk_transfer_t *)sid_hal_malloc(sizeof(sidewalk_transfer_t));
	if (!transfer) {
		LOG_ERR("Fail transfer alloc");
		return;
	}
	transfer->file_id = desc->file_id;
	transfer->file_offset = desc->file_offset;
	transfer->data = buffer->data;
	transfer->data_size = buffer->size;

	int err = sidewalk_event_send(SID_EVENT_FILE_TRANSFER, transfer);
	if (err) {
		LOG_ERR("Event transfer err %d", err);
		LOG_INF("Cancelig file transfer");
#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
		err = nordic_dfu_img_cancel();
		if (err) {
			LOG_ERR("Fail to complete dfu %d", err);
		}
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
		sid_error_t ret =
			sid_bulk_data_transfer_cancel((struct sid_handle *)context,
						      transfer->file_id,
						      SID_BULK_DATA_TRANSFER_REJECT_REASON_GENERIC);
		if (ret != SID_ERROR_NONE) {
			LOG_ERR("Fail to cancel sbdt %d", ret);
		}
		sid_hal_free(transfer);
	}
}

static void on_finalize_request(uint32_t file_id, void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_finalize_request", JSON_OBJ(JSON_NAME("file_id", JSON_INT(file_id)))))));

	// report transfer success
	sid_error_t ret = sid_bulk_data_transfer_finalize(
		(struct sid_handle *)context, file_id, SID_BULK_DATA_TRANSFER_FINAL_STATUS_SUCCESS);
	if (ret != SID_ERROR_NONE) {
		LOG_ERR("sid_bulk_data_transfer_finalize returned %s", SID_ERROR_T_STR(ret));
	}

#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	// request upgrade and reboot
	int err = 0;
	err = nordic_dfu_img_finalize();
	if (err) {
		LOG_ERR("dfu image finalize fail %d", err);
	}

	err = sidewalk_event_send(SID_EVENT_REBOOT, NULL);
	if (err) {
		LOG_ERR("reboot event send ret %d", err);
	}
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
}

static void on_cancel_request(uint32_t file_id, void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_cancel_request", JSON_OBJ(JSON_NAME("file_id", JSON_INT(file_id)))))));

#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	int err = nordic_dfu_img_cancel();
	if (err) {
		LOG_ERR("Fail to complete dfu %d", err);
	}
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
}

static void on_error(uint32_t file_id, void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(
		JSON_NAME("on_error", JSON_OBJ(JSON_NAME("file_id", JSON_INT(file_id)))))));

#ifdef CONFIG_SIDEWALK_FILE_TRANSFER_DFU
	int err = nordic_dfu_img_cancel();
	if (err) {
		LOG_ERR("Fail to complete dfu %d", err);
	}
#endif /* CONFIG_SIDEWALK_FILE_TRANSFER_DFU */
}

static void on_release_scratch_buffer(uint32_t file_id, void *context)
{
	printk(JSON_NEW_LINE(JSON_OBJ(JSON_NAME(
		"on_release_scratch_buffer", JSON_OBJ(JSON_NAME("file_id", JSON_INT(file_id)))))));

	scratch_buffer_remove(file_id);
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
	scratch_buffer_init();

	ft_callbacks.context = (void *)handle;
	sid_error_t ret = sid_bulk_data_transfer_init(
		&(struct sid_bulk_data_transfer_config){ .callbacks = &ft_callbacks }, handle);
	if (ret != SID_ERROR_NONE) {
		LOG_ERR("sid_bulk_data_transfer_init returned %s", SID_ERROR_T_STR(ret));
	}
}

void app_file_transfer_demo_deinit(struct sid_handle *handle)
{
	sid_error_t err = sid_bulk_data_transfer_deinit(handle);
	if (err != SID_ERROR_NONE) {
		LOG_ERR("sid_bulk_data_transfer_deinit returned %s", SID_ERROR_T_STR(err));
	}
}

/* --- Sidewalk FSM Event handlers --- */

void sid_sidewalk_event_file_transfer_handler(void *ctx, sm_t *sm)
{
	sidewalk_transfer_t *args = (sidewalk_transfer_t *)ctx;
	if (!args) {
		LOG_ERR("File transfer event data is NULL");
		return;
	}
	LOG_INF("Received file Id %d; buffer size %d; file offset %d", args->file_id,
		args->data_size, args->file_offset);
	uint8_t hash_out[32];
	sid_pal_hash_params_t params = { .algo = SID_PAL_HASH_SHA256,
					 .data = args->data,
					 .data_size = args->data_size,
					 .digest = hash_out,
					 .digest_size = sizeof(hash_out) };

	sid_error_t e = sid_pal_crypto_hash(&params);
	if (e != SID_ERROR_NONE) {
		LOG_ERR("Failed to hash received file transfer with error %s", SID_ERROR_T_STR(e));
	} else {
#define HEX_PRINTER(a, ...) "%02X"
#define HEX_PRINTER_ARG(a, ...) hash_out[a]
		char hex_str[sizeof(hash_out) * 2 + 1] = { 0 };
		snprintf(hex_str, sizeof(hex_str), LISTIFY(32, HEX_PRINTER, ()),
			 LISTIFY(32, HEX_PRINTER_ARG, (, )));
		LOG_INF("SHA256: %s", hex_str);
	}

	sid_error_t ret =
		sid_bulk_data_transfer_release_buffer(sm->sid->handle, args->file_id, args->data);
	if (ret != SID_ERROR_NONE) {
		LOG_ERR("sid_bulk_data_transfer_release_buffer returned %s", SID_ERROR_T_STR(ret));
	}
	sid_hal_free(args);
}
