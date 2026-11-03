/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_ROW_DEMUX_H
#define SID_ROW_DEMUX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Sync byte that opens a Registration over Wire frame. */
#define SID_ROW_FRAME_SYNC 0xAA
/** Number of framing bytes: the sync byte and the 1-byte length. */
#define SID_ROW_FRAME_HEADER_SIZE 2
/** Maximum frame payload size, limited by the 1-byte length field. */
#define SID_ROW_FRAME_PAYLOAD_MAX UINT8_MAX

enum sid_row_demux_state {
	SID_ROW_DEMUX_WAIT_SYNC,
	SID_ROW_DEMUX_LEN,
	SID_ROW_DEMUX_PAYLOAD,
};

/** Classification of a received byte. */
enum sid_row_demux_result {
	/** Byte dropped: not console text, or part of a discarded frame. */
	SID_ROW_DEMUX_DROPPED,
	/** Byte is console text for the shell. */
	SID_ROW_DEMUX_CONSOLE,
	/** Byte consumed as part of a frame that is not complete yet. */
	SID_ROW_DEMUX_FRAME_BYTE,
	/** Byte completed a frame, which is ready to be taken. */
	SID_ROW_DEMUX_FRAME_READY,
};

struct sid_row_demux {
	uint8_t payload[SID_ROW_FRAME_PAYLOAD_MAX];
	uint16_t payload_len;
	uint16_t expected_len;
	uint32_t overrun_cnt;
	enum sid_row_demux_state state;
	bool frame_ready;
	bool discard;
};

/**
 * @brief Reset the demultiplexer to wait for the sync byte with no frame pending.
 *
 * @param demux Demultiplexer instance.
 */
void sid_row_demux_reset(struct sid_row_demux *demux);

/**
 * @brief Feed one received byte to the demultiplexer.
 *
 * Outside a frame, 7-bit printable ASCII and line editing keys are console text,
 * every other byte except the sync byte is dropped. Inside a frame, every byte
 * belongs to the frame. A frame that arrives while the previous one has not been
 * taken is parsed and discarded.
 *
 * @param demux Demultiplexer instance.
 * @param byte Received byte.
 *
 * @return Classification of the byte.
 */
enum sid_row_demux_result sid_row_demux_feed(struct sid_row_demux *demux, uint8_t byte);

/**
 * @brief Take the completed frame payload.
 *
 * @param demux Demultiplexer instance.
 * @param out Output buffer.
 * @param out_size Size of the output buffer.
 *
 * A pending frame that does not fit in the output buffer is dropped.
 *
 * @return Number of payload bytes copied, 0 if no complete frame is pending
 *         or the pending frame was dropped.
 */
size_t sid_row_demux_take_frame(struct sid_row_demux *demux, uint8_t *out, size_t out_size);

/**
 * @brief Encode a frame: sync byte, 1-byte length and payload.
 *
 * @param payload Frame payload.
 * @param len Payload length, from 1 to SID_ROW_FRAME_PAYLOAD_MAX.
 * @param out Output buffer.
 * @param out_size Size of the output buffer.
 *
 * @return Encoded frame size, 0 on invalid arguments.
 */
size_t sid_row_frame_encode(const uint8_t *payload, size_t len, uint8_t *out, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif /* SID_ROW_DEMUX_H */
