/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <errno.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <tlv/tlv.h>

static uint32_t get_next_free_offset(tlv_ctx *ctx) __attribute__((nonnull));

static tlv_header bytes_to_header(uint8_t data[4])
{
	return (tlv_header){ .type = ((data[0] << 8) + data[1]),
			     .payload_size = { .padding = data[2], .data_size = data[3] } };
}

static void header_to_bytes(tlv_header header, uint8_t data[4])
{
	data[0] = header.type >> 8;
	data[1] = header.type & 0xFF;
	data[2] = header.payload_size.padding;
	data[3] = header.payload_size.data_size;
}

int tlv_lookup(tlv_ctx *ctx, tlv_type type, tlv_header *lookup_data)
{
	if (ctx == NULL || ctx->storage_impl.read == NULL) {
		return -EINVAL;
	}

	for (uint32_t offset = ctx->start_offset + ctx->tlv_storage_start_marker_size;
	     (offset + sizeof(tlv_header)) <= ctx->end_offset;) {
		uint8_t header_raw[4];
		int ret = ctx->storage_impl.read(ctx->storage_impl.ctx, offset, header_raw,
						 sizeof(header_raw));
		if (ret != 0) {
			return ret;
		}
		tlv_header header = bytes_to_header(header_raw);
		offset += sizeof(header);
		if (header.type == type) {
			if (lookup_data) {
				*lookup_data = header;
			}
			return 0;
		}

		offset += header.payload_size.data_size + header.payload_size.padding;
	}
	return -ENODATA;
}

int tlv_read(tlv_ctx *ctx, tlv_type type, uint8_t *data, uint16_t data_size)
{
	if (ctx == NULL || ctx->storage_impl.read == NULL) {
		return -EINVAL;
	}

	for (uint32_t offset = ctx->start_offset + ctx->tlv_storage_start_marker_size;
	     (offset + sizeof(tlv_header)) <= ctx->end_offset;) {
		uint8_t header_raw[4];
		int ret = ctx->storage_impl.read(ctx->storage_impl.ctx, offset, header_raw,
						 sizeof(header_raw));
		if (ret != 0) {
			return ret;
		}

		tlv_header header = bytes_to_header(header_raw);
		offset += sizeof(header_raw);

		if (header.type == type) {
			if (data_size <=
			    header.payload_size.data_size + header.payload_size.padding) {
				if (offset + data_size > ctx->end_offset) {
					return -ENODATA;
				}
				return ctx->storage_impl.read(ctx->storage_impl.ctx, offset, data,
							      data_size);
			}
			return -ENOMEM;
		}
		offset += header.payload_size.data_size + header.payload_size.padding;
	}
	return -ENODATA;
}

static uint32_t get_next_free_offset(tlv_ctx *ctx)
{
	if (ctx->storage_impl.read == NULL) {
		return ctx->end_offset;
	}
	for (uint32_t offset = ctx->start_offset + ctx->tlv_storage_start_marker_size;
	     offset <= ctx->end_offset;) {
		uint8_t header_raw[4];
		int ret = ctx->storage_impl.read(ctx->storage_impl.ctx, offset, header_raw,
						 sizeof(header_raw));
		if (ret != 0) {
			return ret;
		}
		tlv_header header = bytes_to_header(header_raw);
		if (header.type == UINT16_MAX || header.payload_size.data_size == 0) {
			return offset;
		}
		offset += sizeof(header_raw);
		offset += header.payload_size.data_size + header.payload_size.padding;
	}
	return ctx->end_offset;
}

int tlv_write(tlv_ctx *ctx, tlv_type type, const uint8_t *data, uint16_t data_size)
{
	if (ctx == NULL || ctx->storage_impl.read == NULL || ctx->storage_impl.write == NULL) {
		return -EINVAL;
	}

	uint32_t next_free_offset = get_next_free_offset(ctx);
	if (ctx->end_offset <
	    (next_free_offset + sizeof(tlv_header) + data_size + CALCULATE_PADDING(data_size))) {
		return -ENOMEM;
	}

	tlv_header header = { .type = type,
			      .payload_size = { .data_size = data_size,
						.padding = CALCULATE_PADDING(data_size) } };
	uint8_t header_raw[4] = { 0 };
	header_to_bytes(header, header_raw);
	int ret = ctx->storage_impl.write(ctx->storage_impl.ctx, next_free_offset, header_raw,
					  sizeof(header_raw));
	if (ret != 0) {
		return ret;
	}
	next_free_offset += sizeof(header);
	uint8_t write_buff[DATA_ALIGN] = { 0x0 };
	uint16_t data_written = 0;
	while (data_written < data_size) {
		// fill write buffer with padding
		memset(write_buff, PADDING_BYTE, sizeof(write_buff));
		// copy chunk of data to write buffer
		uint16_t bytes_to_write = MIN((data_size - data_written), DATA_ALIGN);
		memcpy(write_buff, data + data_written, bytes_to_write);
		// write the buffer to storage
		ret = ctx->storage_impl.write(ctx->storage_impl.ctx, next_free_offset, write_buff,
					      DATA_ALIGN);
		if (ret != 0) {
			return ret;
		}
		next_free_offset += DATA_ALIGN;
		data_written += DATA_ALIGN;
	}
	return 0;
}

int tlv_read_start_marker(tlv_ctx *ctx, uint8_t *data, uint8_t data_size)
{
	if (ctx == NULL || ctx->storage_impl.read == NULL) {
		return -EINVAL;
	}
	if (data_size != ctx->tlv_storage_start_marker_size) {
		return -ENOMEM;
	}
	return ctx->storage_impl.read(ctx->storage_impl.ctx, ctx->start_offset, data, data_size);
}

int tlv_write_start_marker(tlv_ctx *ctx, uint8_t *data, uint8_t data_size)
{
	if (ctx == NULL || ctx->storage_impl.write == NULL) {
		return -EINVAL;
	}
	if (data_size != ctx->tlv_storage_start_marker_size) {
		return -ENOMEM;
	}
	return ctx->storage_impl.write(ctx->storage_impl.ctx, ctx->start_offset, data, data_size);
}
