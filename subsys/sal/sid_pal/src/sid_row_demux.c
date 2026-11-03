/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_row_demux.h>

#include <string.h>

/* Printable 7-bit ASCII and DEL, which some terminals send for backspace. */
#define ASCII_PRINTABLE_FIRST 0x20
#define ASCII_7BIT_LAST 0x7F

static bool is_console_text(uint8_t byte)
{
	return (byte >= ASCII_PRINTABLE_FIRST && byte <= ASCII_7BIT_LAST) || byte == '\r' ||
	       byte == '\n' || byte == '\b' || byte == '\t';
}

void sid_row_demux_reset(struct sid_row_demux *demux)
{
	if (!demux) {
		return;
	}
	demux->state = SID_ROW_DEMUX_WAIT_SYNC;
	demux->payload_len = 0;
	demux->expected_len = 0;
	demux->frame_ready = false;
	demux->discard = false;
}

enum sid_row_demux_result sid_row_demux_feed(struct sid_row_demux *demux, uint8_t byte)
{
	if (!demux) {
		return SID_ROW_DEMUX_DROPPED;
	}

	switch (demux->state) {
	case SID_ROW_DEMUX_WAIT_SYNC:
		if (byte == SID_ROW_FRAME_SYNC) {
			demux->state = SID_ROW_DEMUX_LEN;
			return SID_ROW_DEMUX_FRAME_BYTE;
		}
		return is_console_text(byte) ? SID_ROW_DEMUX_CONSOLE : SID_ROW_DEMUX_DROPPED;

	case SID_ROW_DEMUX_LEN:
		if (!byte || byte > sizeof(demux->payload)) {
			demux->state = SID_ROW_DEMUX_WAIT_SYNC;
			return SID_ROW_DEMUX_DROPPED;
		}
		demux->expected_len = byte;
		demux->discard = demux->frame_ready;
		if (!demux->discard) {
			demux->payload_len = 0;
		}
		demux->state = SID_ROW_DEMUX_PAYLOAD;
		return SID_ROW_DEMUX_FRAME_BYTE;

	case SID_ROW_DEMUX_PAYLOAD:
		if (demux->discard) {
			if (!--demux->expected_len) {
				demux->overrun_cnt++;
				demux->discard = false;
				demux->state = SID_ROW_DEMUX_WAIT_SYNC;
			}
			return SID_ROW_DEMUX_DROPPED;
		}
		demux->payload[demux->payload_len++] = byte;
		if (demux->payload_len < demux->expected_len) {
			return SID_ROW_DEMUX_FRAME_BYTE;
		}
		demux->frame_ready = true;
		demux->state = SID_ROW_DEMUX_WAIT_SYNC;
		return SID_ROW_DEMUX_FRAME_READY;

	default:
		demux->state = SID_ROW_DEMUX_WAIT_SYNC;
		return SID_ROW_DEMUX_DROPPED;
	}
}

size_t sid_row_demux_take_frame(struct sid_row_demux *demux, uint8_t *out, size_t out_size)
{
	if (!demux || !out || !demux->frame_ready) {
		return 0;
	}

	if (out_size < demux->payload_len) {
		/* Never hand out a partial frame. Drop it to unblock the next one. */
		demux->frame_ready = false;
		demux->overrun_cnt++;
		return 0;
	}

	const size_t len = demux->payload_len;

	memcpy(out, demux->payload, len);
	demux->frame_ready = false;

	return len;
}

size_t sid_row_frame_encode(const uint8_t *payload, size_t len, uint8_t *out, size_t out_size)
{
	if (!payload || !out || !len || len > SID_ROW_FRAME_PAYLOAD_MAX ||
	    out_size < len + SID_ROW_FRAME_HEADER_SIZE) {
		return 0;
	}

	out[0] = SID_ROW_FRAME_SYNC;
	out[1] = (uint8_t)len;
	memcpy(&out[SID_ROW_FRAME_HEADER_SIZE], payload, len);

	return len + SID_ROW_FRAME_HEADER_SIZE;
}
