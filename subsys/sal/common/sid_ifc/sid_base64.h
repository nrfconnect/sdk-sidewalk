/*
 * Copyright 2020-2025 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_BASE64_H
#define SID_BASE64_H

/** @file
 *
 * @defgroup sid_base64 sid base64 chunked encode/decode
 *
 * Loosely based on the Zlib API for handling input/output buffers.
 * This is to support encoding/decoding large or unknown-length strings
 * without large or dynamic memory allocations.
 *
 * To start, the user must allocate a base64_ctx_t (e.g. on the stack) and
 * call base64_init().
 *
 * Then the user must provide an input buffer, by setting ctx.next_in and
 * ctx.avail_in; and an output buffer, by setting ctx.next_out and
 * ctx.avail_out.
 *
 * base64_encode()/base64_decode() will read from the input buffer and write to
 * the output buffer, until one of the buffers is empty. It will then return
 * SID_ERROR_NONE (except in the SID_ERROR_BUFFER_OVERFLOW case described below).
 *
 * If ctx.avail_in == 0 and there are more input chunks available, the user must
 * set ctx.next_in/ctx.avail_in to the next input chunk before calling
 * base64_encode()/base64_decode() again.
 *
 * If ctx.avail_out == 0, the user must set ctx.next_out/ctx.avail_out to the
 * next output chunk before calling base64_encode()/base64_decode() again.
 *
 * When encoding: If this is the final input chunk, base64_encode() must be
 * called with finish=true. This instructs the encoder to add the '=' padding
 * bytes if needed. Once called with finish=true, all subsequent calls must
 * also have finish=true. base64_encode() will return SID_ERROR_NONE when all
 * output bytes (including padding) have been written. Otherwise, the function
 * will return SID_ERROR_BUFFER_OVERFLOW if the input or output buffer is
 * exhausted.
 *
 * ctx.total_in and ctx.total_out must not be modified by the user, but may be
 * read. They contain the total number of bytes read/written so far.
 */

#include <sid_error.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sid_base64_ctx {
    const uint8_t *next_in;
    size_t avail_in;
    size_t total_in;

    uint8_t *next_out;
    size_t avail_out;
    size_t total_out;

    // Private fields for implementation:

    uint32_t temp;      // FIFO of decoded bits
    uint8_t temp_len;   // size in bits
    int state;
};

/*
 * Initialize (or reinitialize) the encode/decode context.
 * Must be called before base64_encode()/base64_decode().
 */
void sid_base64_init(struct sid_base64_ctx *ctx);

/*
 * Returns SID_ERROR_BUFFER_OVERFLOW if the input or output buffer is exhausted.
 *
 * Returns SID_ERROR_NONE if finish=true and all bytes have been output.
 */
sid_error_t sid_base64_encode(struct sid_base64_ctx *ctx, bool finish);

/*
 * Returns SID_ERROR_INVALID_ARGS if the input is not valid base64
 * (contains unrecognized characters, or contains characters after the '='
 * padding).
 *
 * Returns SID_ERROR_NONE if the input or output buffer is exhausted.
 */
sid_error_t sid_base64_decode(struct sid_base64_ctx *ctx);

/*
 * Length of base64-encoded string in bytes,
 * given length of input string in bytes.
 */
#define SID_BASE64_ENCODED_LENGTH(bytes) ((((bytes) + 2) / 3) * 4)

/*
 * Maximum length of base64-decoded output string in bytes,
 * given length of input string in bytes.
 * (The actual length may be less if the input ends with padding bytes.)
 */
#define SID_BASE64_DECODED_MAX_LENGTH(bytes) ((((bytes) + 3) / 4) * 3)

#ifdef __cplusplus
}
#endif

/** @} */

#endif
