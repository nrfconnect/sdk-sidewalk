/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef TLV_H
#define TLV_H

#include <stdint.h>
#include <stdbool.h>

#define DATA_ALIGN 4
#define CALCULATE_PADDING(val) ((DATA_ALIGN - (val % DATA_ALIGN)) % DATA_ALIGN)
#define PADDING_BYTE 0xFF

/**
 * @brief Write data to storage.
 * 
 * @param ctx Storage context.
 * @param offset Location where to place the date.
 * @param data Data to save.
 * @param data_size Number of bytes to save.
 * 
 */
typedef int (*tlv_storage_write_t)(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);

/**
 * @brief Read data from storage.
 * 
 * @param ctx Storage context.
 * @param offset Location where to find the date.
 * @param data Buffor where data will be placed.
 * @param data_size Number of bytes to read.
 * 
 */
typedef int (*tlv_storage_read_t)(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_cap);

typedef struct tlv_ctx {
	struct tlv_storage {
		void *ctx;
		tlv_storage_write_t write;
		tlv_storage_read_t read;
	} storage_impl;
	/*starting offset of the tlv memory region, including storage marker*/
	uint32_t start_offset;
	/*first offset outside valid memory range, that can not be written*/
	uint32_t end_offset;
	/* size of starting marker, after the marker the first tlv entry is stored.*/
	uint32_t tlv_storage_start_marker_size;
} tlv_ctx;

typedef uint16_t tlv_type;
typedef struct {
	uint8_t padding;
	uint8_t data_size;
} tlv_size;

typedef struct {
	tlv_type type;
	tlv_size payload_size;
} tlv_header;

/**
 * @brief read the header of the TLV storage
 *        The header usually contains some magic value that signal start of data
 * 
 * @param ctx tlv context
 * @param data [OUT] content of header
 * @param data_size size of the header to read, need to match the size passed in ctx
 * @return int 0 on success, negative in case of error
 */
int tlv_read_start_marker(tlv_ctx *ctx, uint8_t *data, uint8_t data_size);

/**
 * @brief Write the header of the TLV storage
 *        The header usually contains some magic value that signal start of data
 * 
 * @param ctx tlv context
 * @param data [IN] content of header
 * @param data_size size of the header to read, need to match the size passed in ctx
 * @return int 0 on success, negative in case of error
 */
int tlv_write_start_marker(tlv_ctx *ctx, uint8_t *data, uint8_t data_size);

/**
 * @brief Find TLV header
 * 
 * @param ctx tlv context
 * @param type type to find
 * @param header [OUT] header of the type if found
 * @return int 0 on success, negative in case of error
 *   -EINVAL when ctx is invalid
 *   -ENODATA when did not found type
 *   other errors are passed from storage handlers. 
 */
int tlv_lookup(tlv_ctx *ctx, tlv_type type, tlv_header *header);

/**
 * @brief Read data from TLV
 * 
 * @param ctx tlv context
 * @param type type to read
 * @param data buffer for the read data
 * @param data_size number of bytes to read. Need to be smaller or equal to the payload size written in TLV heaader for the type.
 * @return int 0 on success, negative in case of error
 *   -EINVAL when ctx is invalid
 *   -ENODATA when did not found type or data is invalid
 *   -ENOMEM when data_size is bigger than data_size encoded in the TLV header.
 *   other errors are passed from storage handlers. 
 */
int tlv_read(tlv_ctx *ctx, tlv_type type, uint8_t *data, uint16_t data_size);

/**
 * @brief Write data to TLV
 * 
 * @param ctx tlv context
 * @param type type to write
 * @param data payload to write
 * @param data_size size of payload to write
 * @return int 0 on success, negative in case of error
 *   -EINVAL when ctx is invalid
 *   -ENOMEM when can not fit data in storage
 *   other errors are passed from storage handlers. 
 */
int tlv_write(tlv_ctx *ctx, tlv_type type, const uint8_t *data, uint16_t data_size);

#endif
