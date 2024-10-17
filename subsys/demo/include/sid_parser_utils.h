/*
 * Copyright 2021-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PARSER_UTILS_H_
#define SID_PARSER_UTILS_H_

#include <sid_error.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SID_SET_BIT(val, pos) ((val) | (1 << (pos)))
#define SID_CLEAR_BIT(val, pos) ((val) & ~(1 << (pos)))
#define SID_IS_BIT(val, pos) ((val) & (1 << pos))

typedef enum {
    SET_BIT,
    CLR_BIT,
    CHK_BIT,
} bit_opt_t;

enum tlv_mask {
    TLV_SIZE_OFFSET = 6,
    TLV_TAG_OFFSET = 0,
    TLV_SIZE_MASK = 3,
    TLV_TAG_MASK = 0X3F,
    TLV_SIZE_VALUE_1BYTE = 0,
    TLV_SIZE_VALUE_2BYTES = 1,
    TLV_SIZE_VALUE_4BYTES = 2,
    TLV_SIZE_VALUE_UNDEFINED = 3,
};

struct tl_in_tlv {
    uint8_t len;
    uint8_t tag;
};

/**
 * Store TLV header structure into user defined buffer
 * @param tl - TLV record to store
 * @param buf - buffer start, will be advanced by number of bytes used to store value
 * @param end - pointer to the end of user buffer
 *
 * @return SID_ERROR_NONE if operation was successful or one of errors below
 * @retval SID_ERROR_OOM - in case if buffer size is too small to store data
 */
sid_error_t set_and_shift_tl_bytes_in_tlv(const struct tl_in_tlv *tl, uint8_t **buf, const uint8_t *end);

/**
 * Parse TLV header structure into user defined buffer
 * @param buf - buffer start, will be advanced by number of bytes read during parse
 * @param tl - TLV record to store parsed result
 * @param end - pointer to the end of user buffer
 *
 * @return SID_ERROR_NONE if operation was successful or one of errors below
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - in case if buffer does not have enough data
 */
sid_error_t get_and_shift_tl_from_tlv(const uint8_t **buf, struct tl_in_tlv *tl, const uint8_t *end);

/**
 * Retrieve bit field from uint8_t value by mask and offset
 *
 * @param buf - data buffer to read
 * @param ret - reference to return value
 * @param mask - bit field mask
 * @param offset - bit field offset
 *
 * @retval SID_ERROR_NONE
 */
sid_error_t get_bit_from_field(const uint8_t *buf, uint8_t *ret, uint8_t mask, uint8_t offset);

/**
 * Retrieve bit field from uint16_t value by mask and offset
 *
 * @param buf - reference to data
 * @param ret - return value
 * @param mask - bit field mask
 * @param offset - bit field offset
 *
 * @retval SID_ERROR_NONE
 */
sid_error_t get_bit_from_field16(const uint16_t *buf, uint16_t *ret, uint16_t mask, uint8_t offset);

/**
 * Store bit field by mask and offset from  uint8_t into raw buffer
 *
 * @param buf - buffer to store data
 * @param val - value with bit information
 * @param mask - bit field mask
 * @param offset - bit field offset
 *
 * @retval SID_ERROR_NONE
 */
sid_error_t set_bit_to_field(uint8_t *buf, uint8_t val, uint8_t mask, uint8_t offset);

/**
 * Store bit field by mask and offset from  uint16_t into raw buffer
 *
 * @param buf - buffer to store data
 * @param val - value with bit information
 * @param mask - bit field mask
 * @param offset - bit field offset
 *
 * @retval SID_ERROR_NONE
 */
sid_error_t set_bit_to_field16(uint16_t *buf, uint16_t val, uint16_t mask, uint8_t offset);

/**
 * Validates if buffer has more than byte available to store data
 *
 * @param start - buffer start
 * @param end - buffer end
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space available
 */
sid_error_t check_has_memory(const uint8_t *start, const uint8_t *end);

/**
 * Validates if buffer at least a byte available to store data
 *
 * @param start - buffer start
 * @param end - buffer end
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space available
 */
sid_error_t check_has_memory_at_offset(const uint8_t *start, const uint8_t *end);

/**
 * Validates if buffer has more than byte available to read from
 *
 * @param start - buffer start
 * @param end - buffer end
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - no more data available
 */
sid_error_t check_has_data(const uint8_t *start, const uint8_t *end);

/**
 * Validates if buffer has at least 1 byte available to read from
 *
 * @param start - buffer start
 * @param end - buffer end
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - no more data available
 */
sid_error_t check_has_data_at_offset(const uint8_t *start, const uint8_t *end);

/**
 * Retrieve uint8_t value from buffer and advances buffer pointer
 *
 * @param buf - reference to data buffer
 * @param ret - vale to read data into
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - no more data to process
 */
sid_error_t get_and_shift_uint8(const uint8_t **buf, uint8_t *ret, const uint8_t *end);

/**
 * Store uint8_t value into buffer and advances buffer pointer
 *
 * @param val - to store
 * @param buf - to store vale
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 */
sid_error_t set_and_shift_uint8(const uint8_t val, uint8_t **buf, const uint8_t *end);

/**
 * Retrieve uint16_t value from buffer and advances buffer pointer.
 *
 * @param buf - reference to data buffer
 * @param ret - vale to read data into
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - no more data to process
 *
 * @note endianness is not taken into account and should be handled separately
 */
sid_error_t get_and_shift_uint16(const uint8_t **buf, uint16_t *ret, const uint8_t *end);

/**
 * Store uint16_t value into buffer and advances buffer pointer
 * Data is stored as byte array, endianness is not taken into account,
 * should be handled separately.
 *
 * @param val - to store
 * @param buf - to store vale
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 *
 * @note endianness is not taken into account and should be handled separately
 */
sid_error_t set_and_shift_uint16(const uint16_t val, uint8_t **buf, const uint8_t *end);

/**
 * Retrieve 3 byte value from buffer into uint32_t and advances buffer pointer.
 * Function converts value from BigEndian format into native host format
 *
 * @param buf - reference to data buffer
 * @param ret - vale to read data into
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - no more data to process
 */
sid_error_t get_and_shift_uint24(const uint8_t **buf, uint32_t *ret, const uint8_t *end);

/**
 * Store 3 byte value (stored in uint32_t value) into buffer and advances buffer pointer
 * Value is converted to BigEndian representation
 *
 * @param val - to store
 * @param buf - to store vale
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 */
sid_error_t set_and_shift_uint24(const uint32_t val, uint8_t **buf, const uint8_t *end);

/**
 * Retrieve uint32_t value from buffer and advances buffer pointer
 *
 * @param buf - reference to data buffer
 * @param ret - vale to read data into
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - no more data to process
 *
 * @note endianness is not taken into account and should be handled separately
 */
sid_error_t get_and_shift_uint32(const uint8_t **buf, uint32_t *ret, const uint8_t *end);

/**
 * Store uint32_t value into buffer and advances buffer pointer
 * Data is stored as byte array, endianness is not taken into account,
 * should be handled separately.
 *
 * @param val - to store
 * @param buf - to store vale
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 *
 * @note endianness is not taken into account and should be handled separately
 */
sid_error_t set_and_shift_uint32(const uint32_t val, uint8_t **buf, const uint8_t *end);

/**
 * Retrieve specified number of bytes from buffer and advances buffer pointer
 *
 * @param buf - reference to data buffer
 * @param ret - vale to read data into
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - no more data to process
 */
sid_error_t get_and_shift_block(const uint8_t **buf, const size_t len, uint8_t *block, const uint8_t *end);

/**
 * Store specified number of bytes into buffer and advances buffer pointer
 *
 * @param val - to store
 * @param buf - to store vale
 * @param end - end of data buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 */
sid_error_t set_and_shift_block(const uint8_t *block, const size_t len, uint8_t **buf, const uint8_t *end);

/**
 * Parse prefix sized array of uint8_t value into user array
 *
 * @param rover - data to parse
 * @param end - end of data block
 * @param max_len - capacity of user array
 * @param array_size - parsed array size
 * @param array - user array
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_PARAM_OUT_OF_RANGE - no more data to process
 */
sid_error_t get_and_shift_array_prefix_size_uint8(const uint8_t **rover,
                                                  const uint8_t *end,
                                                  const size_t max_len,
                                                  uint8_t *array_size,
                                                  uint8_t *array);

/**
 * Store uint8_t value in the TLV format with user defined tag
 *
 * @param tag - value tag
 * @param val - value
 * @param buf - buffer to serialize data
 * @param end - end of serialize buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 */
sid_error_t set_and_shift_uint8_tlv(uint8_t tag, uint8_t val, uint8_t **buf, const uint8_t *end);

/**
 * Store uint16_t value in the TLV format with user defined tag
 *
 * uint16_t is stored by @see set_and_shift_uint16
 *
 * @param tag - value tag
 * @param val - value
 * @param buf - buffer to serialize data
 * @param end - end of serialize buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 *
 * @note endianness is not taken into account and should be handled separately
 */
sid_error_t set_and_shift_uint16_tlv(uint8_t tag, uint16_t val, uint8_t **buf, const uint8_t *end);

/**
 * Store uint32_t value in the TLV format with user defined tag
 *
 * uint32_t is stored by @see set_and_shift_uint32
 *
 * @param tag - value tag
 * @param val - value
 * @param buf - buffer to serialize data
 * @param end - end of serialize buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 *
 * @note endianness is not taken into account and should be handled separately
 */
sid_error_t set_and_shift_uint32_tlv(uint8_t tag, uint32_t val, uint8_t **buf, const uint8_t *end);

/**
 * Store user defined block in the TLV format with user defined tag
 *
 * @param tag - value tag
 * @param val - value
 * @param buf - buffer to serialize data
 * @param end - end of serialize buffer
 *
 * @retval SID_ERROR_NONE
 * @retval SID_ERROR_OOM - no more space left to store data
 */
sid_error_t set_and_shift_block_tlv(uint8_t tag, const uint8_t *block, size_t len, uint8_t **buf, const uint8_t *end);

/**
 * Describes the parse state
 * Ideally the ret_code should be check after all the write/read trasaction are
 * called, instead of after each call
 */
struct sid_parse_state {
    /**
     * Used to indicate if an error was encountered during parsing
     */
    sid_error_t ret_code;
    /**
     * Memory buffer which to read/write to
     */
    uint8_t *buffer;
    /**
     * Len of the buffer
     */
    size_t buffer_len;
    /**
     * size_t of data in bytes that was written to read from
     */
    size_t offset;
};

#define SID_PARSE_STATE_INIT(buffer_, buffer_len_)                                             \
    {                                                                                          \
        .ret_code = SID_ERROR_NONE, .buffer = buffer_, .buffer_len = buffer_len_, .offset = 0, \
    }

/**
 * Initialie sid_parse_state structure
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] buffer A pointer to the buffer that needs to processed
 * @param[in] buffer_len Length of the buffer
 */
void sid_parse_state_init(struct sid_parse_state *const state, uint8_t *const buffer, size_t buffer_len);

/**
 * Store uint8_t value into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] val uint8_t that needs to be written to buffer
 */
void sid_write_entry_uint8(struct sid_parse_state *const state, uint8_t val);

/**
 * Store uint16_t value into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] val uint16_t that needs to be written to buffer
 */
void sid_write_entry_uint16(struct sid_parse_state *const state, uint16_t val);

/**
 * Store uint24_t value into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] val uint24_t that needs to be written to buffer
 */
void sid_write_entry_uint24(struct sid_parse_state *const state, uint32_t val);

/**
 * Store uint32_t value into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] val uint32_t that needs to be written to buffer
 */
void sid_write_entry_uint32(struct sid_parse_state *const state, uint32_t val);

/**
 * Store nbytes into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] val buffer that needs to be stored into buffer within state
 * @param[in] len Leng of the buffer that needs to be written
 */
void sid_write_entry_nbytes(struct sid_parse_state *const state, const uint8_t *const val, size_t len);

/**
 * Store TLV header structure into user defined buffer
 * Data is stored in TLV header structure
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tl - TLV record to store
 */
void sid_write_entry_tlv(struct sid_parse_state *const state, const struct tl_in_tlv *tl);

/**
 * Store TLV header structure into user defined buffer
 * Data is stored in TLV header structure
 * Store uint8_t value into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tl[in] TLV record to store
 * @param[in] val uint8_t that needs to be written to buffer
 */
void sid_write_entry_tlv_uint8(struct sid_parse_state *const state, const struct tl_in_tlv *tl, uint8_t val);

/**
 * Store TLV header structure into user defined buffer
 * Data is stored in TLV header structure
 * Store uint16_t value into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tl[in] TLV record to store
 * @param[in] val uint16_t that needs to be written to buffer
 */
void sid_write_entry_tlv_uint16(struct sid_parse_state *const state, const struct tl_in_tlv *tl, uint16_t val);

/**
 * Store TLV header structure into user defined buffer
 * Data is stored in TLV header structure
 * Store uint24_t value into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tl[in] TLV record to store
 * @param[in] val uint24_t that needs to be written to buffer
 */
void sid_write_entry_tlv_uint24(struct sid_parse_state *const state, const struct tl_in_tlv *tl, uint32_t val);

/**
 * Store TLV header structure into user defined buffer
 * Data is stored in TLV header structure
 * Store uint32_t value into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tl[in] TLV record to store
 * @param[in] val uint32_t that needs to be written to buffer
 */
void sid_write_entry_tlv_uint32(struct sid_parse_state *const state, const struct tl_in_tlv *tl, uint32_t val);

/**
 * Store TLV header structure into user defined buffer
 * Data is stored in TLV header structure
 * Store nbytes into buffer indicated in state structure
 * Data is stored as byte array, endianness is not taken into account,
 * and should be handled separately.
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tl[in] TLV record to store
 * @param[in] val buffer that needs to be stored into buffer within state
 * @param[in] len Leng of the buffer that needs to be written
 */
void sid_write_entry_tlv_nbytes(struct sid_parse_state *const state,
                                const struct tl_in_tlv *tl,
                                const uint8_t *const val,
                                size_t len);

/**
 * Retrieve uint8_t value from buffer stored in state
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] val A pointer to uint8_t that data is read into
 */
void sid_read_entry_uint8(struct sid_parse_state *const state, uint8_t *const val);

/**
 * Retrieve uint16_t value from buffer stored in state
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] val A pointer to uint16_t that data is read into
 */
void sid_read_entry_uint16(struct sid_parse_state *const state, uint16_t *const val);

/**
 * Retrieve uint24_t value from buffer stored in state
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] val A pointer to uint24_t that data is read into
 */
void sid_read_entry_uint24(struct sid_parse_state *const state, uint32_t *const val);

/**
 * Retrieve uint32_t value from buffer stored in state
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] val A pointer to uint32_t that data is read into
 */
void sid_read_entry_uint32(struct sid_parse_state *const state, uint32_t *const val);

/**
 * Retrieve nbytes value from buffer stored in state
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] val A pointer to buffer that data should be read into
 * @param[in] len Size of the buffer that data is read into
 */
void sid_read_entry_nbytes(struct sid_parse_state *const state, uint8_t *const val, size_t len);

/**
 * Parse prefix sized array of uint8_t value into user array
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param array - user array
 * @param max_len - capacity of user array
 * @param array_size - parsed array size
 */
void sid_read_array_prefix_size_uint8(struct sid_parse_state *const state,
                                      uint8_t *array,
                                      const size_t max_len,
                                      uint8_t *array_size);

/**
 * Store TLV header structure into user defined buffer
 *
 * @param tl - TLV record to store
 * @param[in,out] state A pointer to the sid_parse_state*
 */
void sid_write_tl_to_tlv(struct sid_parse_state *const state, const struct tl_in_tlv *tl);

/**
 * Parse TLV header structure into user defined buffer
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tl - TLV record to store parsed result
 */
void sid_read_tl_from_tlv(struct sid_parse_state *const state, struct tl_in_tlv *tl);

/**
 * Store uint8_t value in the TLV format with user defined tag
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tag - value tag
 * @param val - value
 */
void sid_write_uint8_tlv(struct sid_parse_state *const state, uint8_t tag, uint8_t val);

/**
 * Store uint16_t value in the TLV format with user defined tag
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tag - value tag
 * @param val - value
 */
void sid_write_uint16_tlv(struct sid_parse_state *const state, uint8_t tag, uint16_t val);

/**
 * Store uint32_t value in the TLV format with user defined tag
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tag - value tag
 * @param val - value
 */
void sid_write_uint32_tlv(struct sid_parse_state *const state, uint8_t tag, uint32_t val);

/**
 * Store user defined block in the TLV format with user defined tag
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param tag - value tag
 * @param val - value
 * @param len - length of the value
 */
void sid_write_nbytes_tlv(struct sid_parse_state *const state, uint8_t tag, const uint8_t *val, size_t len);

/**
 * Get the lenght of bytes left in the buffer to read
 *
 * @param[in] state A pointer to the sid_parse_state
 */
size_t sid_get_length_from_parse_state(const struct sid_parse_state *const state);

/**
 * Get the read buffer in current state
 *
 * @param[in] state A pointer to the sid_parse_state
 */
const uint8_t *sid_get_buf_from_parse_state(const struct sid_parse_state *const state);

/**
 * Drop date in the state
 *
 * @param[in,out] state A pointer to the sid_parse_state
 */
void sid_drop_nbyte_from_parse_state(struct sid_parse_state *const state, size_t len);

/** @brief Convert unsigned integer to ascii c-style string in specified base
 *
 *  @param unsigned integer to convert to ascii string
 *  @param pointer to buffer to store result
 *  @param base to convert to (2 to 16)
 *  @return string containing number converted to ascii base
 */
uint8_t *utils_uint_to_str(uint32_t value, uint8_t *result, uint8_t base);

/** @brief Convert unsigned integer to ascii c-style string in specified base
 *
 *  Skips over characters that are not numbers which helps
 *  to automatically convert NMEA float values to binary integers.
 *  @param string to convert to int
 *  @param base to convert from (2 to 16)
 *  @return unsigned integer value of numeric string
 */
uint32_t utils_str_to_uint(uint8_t *str, uint8_t base);

/** @brief Convert integer to ascii c-style string in specified base
 *
 *  Skips over characters that are not numbers which helps
 *  to automatically convert NMEA float values to binary integers.
 *
 *  @param string to convert to int
 *  @param base to convert from (2 to 16)
 *  @return integer value of numeric string
 */
int32_t utils_str_to_int(uint8_t *str, uint8_t base);

/** @brief Print buffer in hexadecimal
 *
 *  @param pointer to buffer
 *  @param size of the buffer
 *  @return none
 */
void utils_print_hex(uint8_t *buf, uint8_t len);

/** @brief parse integer in the passed string buffer
 *
 *  @param pointer to buffer
 *  @return integer present in the buffer
 */
int32_t utils_parse_input_num(uint8_t *buf);

/** @brief convert buffer in hex string format to
 *  string format
 *
 *  @param pointer to hex string buffer
 *  @param pointer to buffer
 *  @return number of bytes filled in output buffer
 */
int16_t utils_parse_hex_in_str(uint8_t *out_buf, uint8_t *buf);

/** @brief convert buffer in hex string format to
 *  decimal string format
 *
 *  @param pointer to hex string buffer
 *  @param output buffer pointer to decimal string
 *  @return number of bytes filled in output buffer
 */
int8_t utils_hex_str_to_bin(uint8_t *str, uint8_t *dst);

/** @brief convert buffer in decimal string format to
 *  hex string format
 *
 *  @param pointer to decimal string buffer
 *  @param output buffer pointer to hex string
 *  @return number of bytes filled in output buffer
 */
void utils_bin_to_hex_str(uint8_t *dst, uint8_t *src, uint16_t len);

/** @brief switch endianess of the buffer
 *
 *  @param pointer to the buffer
 *  @param size of the buffer
 *  @return none
 */
void utils_flip_bytes(uint8_t *ptr, uint8_t sz);

/** @brief Performs bitwise operations on passed uint32_t
 *
 *  Sets the nth bit (n>=0)
 *  Clears the nth bit (n>=0)
 *  Returns if the nth bit (n>=0) is 1 or not
 *
 *  @param pointer to the uint32_t
 *  @param bit position to check or alter
 *  @param desired operation on the bit position
 *  @return always true for set and clear, for check
 *  operation the result whether the bit is set or not
 */
bool utils_set_clr_chk_bit(uint32_t *val, int n, bit_opt_t opr);

/**
 * @brief Convert formatted ASCII char mac string to byte buffer using format string
 *
 * @param str Formatted ascii char null-terminated string: "11:22:33:44:AA:bb" e.g.
 * @param out Output byte buffer
 * @return 0-0xF if hex ascii param is valid or 0xFF if param is invalid
 */
uint8_t utils_parse_mac_str(const uint8_t *str, uint8_t *out);

/** @brief Read 2 bytes from a buffer
 *
 *  @param pointer to the buffer
 *  @param position in the buffer to read from
 *  @return unsigned short value
 */
uint16_t utils_read_word(uint8_t *buf, uint32_t pos);

/** @brief Read 4 bytes to a buffer
 *
 *  @param pointer to the buffer
 *  @param position in the buffer to write
 *  @return unsigned int
 */
uint32_t utils_read_dword(uint8_t *buf, uint32_t pos);

/** @brief Write 2 bytes to a buffer
 *
 *  @param pointer to the buffer
 *  @param value to write
 *  @return none
 */
void utils_write_word(uint8_t *buf, uint16_t val);

/** @brief Write 4 bytes to a buffer
 *
 *  @param pointer to the buffer
 *  @param value to write
 *  @return none
 */
void utils_write_dword(uint8_t *buf, uint32_t val);

#ifdef __cplusplus
}   // extern "C" {
#endif

#endif /* SID_PARSER_UTILS_H_ */
