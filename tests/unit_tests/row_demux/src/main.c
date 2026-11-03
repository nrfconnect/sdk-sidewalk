/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_row_demux.h>

#include <zephyr/ztest.h>

#include <string.h>

static struct sid_row_demux demux;

static void before(void *fixture)
{
	ARG_UNUSED(fixture);
	memset(&demux, 0, sizeof(demux));
	sid_row_demux_reset(&demux);
}

ZTEST_SUITE(row_demux, NULL, NULL, before, NULL, NULL);

static enum sid_row_demux_result feed_all(const uint8_t *data, size_t len)
{
	enum sid_row_demux_result res = SID_ROW_DEMUX_DROPPED;

	for (size_t i = 0; i < len; i++) {
		res = sid_row_demux_feed(&demux, data[i]);
	}

	return res;
}

ZTEST(row_demux, test_console_text)
{
	const char *line = "cert init\r\n";

	for (size_t i = 0; i < strlen(line); i++) {
		zassert_equal(sid_row_demux_feed(&demux, line[i]), SID_ROW_DEMUX_CONSOLE);
	}
	zassert_equal(sid_row_demux_feed(&demux, 0x7F), SID_ROW_DEMUX_CONSOLE, "DEL is console");
	zassert_equal(sid_row_demux_feed(&demux, '\b'), SID_ROW_DEMUX_CONSOLE);
}

ZTEST(row_demux, test_non_ascii_dropped)
{
	const uint8_t junk[] = { 0x00, 0x80, 0xFF, 0x1B, 0xF0 };

	for (size_t i = 0; i < sizeof(junk); i++) {
		zassert_equal(sid_row_demux_feed(&demux, junk[i]), SID_ROW_DEMUX_DROPPED);
	}
	zassert_equal(demux.state, SID_ROW_DEMUX_WAIT_SYNC);
}

ZTEST(row_demux, test_single_frame)
{
	const uint8_t frame[] = { SID_ROW_FRAME_SYNC, 3, 0x01, SID_ROW_FRAME_SYNC, 0x03 };
	uint8_t out[SID_ROW_FRAME_PAYLOAD_MAX];

	zassert_equal(sid_row_demux_feed(&demux, frame[0]), SID_ROW_DEMUX_FRAME_BYTE);
	zassert_equal(sid_row_demux_feed(&demux, frame[1]), SID_ROW_DEMUX_FRAME_BYTE);
	zassert_equal(sid_row_demux_feed(&demux, frame[2]), SID_ROW_DEMUX_FRAME_BYTE);
	zassert_equal(sid_row_demux_feed(&demux, frame[3]), SID_ROW_DEMUX_FRAME_BYTE,
		      "sync byte inside the payload is payload");
	zassert_equal(sid_row_demux_feed(&demux, frame[4]), SID_ROW_DEMUX_FRAME_READY);

	zassert_equal(sid_row_demux_take_frame(&demux, out, sizeof(out)), 3);
	zassert_mem_equal(out, &frame[2], 3);
	zassert_equal(sid_row_demux_take_frame(&demux, out, sizeof(out)), 0,
		      "frame taken only once");
}

ZTEST(row_demux, test_no_partial_frame)
{
	const uint8_t partial[] = { SID_ROW_FRAME_SYNC, 4, 0x11, 0x22 };
	uint8_t out[SID_ROW_FRAME_PAYLOAD_MAX];

	zassert_equal(feed_all(partial, sizeof(partial)), SID_ROW_DEMUX_FRAME_BYTE);
	zassert_equal(sid_row_demux_take_frame(&demux, out, sizeof(out)), 0);
}

ZTEST(row_demux, test_max_payload)
{
	uint8_t out[SID_ROW_FRAME_PAYLOAD_MAX];

	zassert_equal(sid_row_demux_feed(&demux, SID_ROW_FRAME_SYNC), SID_ROW_DEMUX_FRAME_BYTE);
	zassert_equal(sid_row_demux_feed(&demux, SID_ROW_FRAME_PAYLOAD_MAX),
		      SID_ROW_DEMUX_FRAME_BYTE);
	for (int i = 0; i < SID_ROW_FRAME_PAYLOAD_MAX - 1; i++) {
		zassert_equal(sid_row_demux_feed(&demux, (uint8_t)i), SID_ROW_DEMUX_FRAME_BYTE);
	}
	zassert_equal(sid_row_demux_feed(&demux, 0xEE), SID_ROW_DEMUX_FRAME_READY);
	zassert_equal(sid_row_demux_take_frame(&demux, out, sizeof(out)),
		      SID_ROW_FRAME_PAYLOAD_MAX);
	zassert_equal(out[SID_ROW_FRAME_PAYLOAD_MAX - 1], 0xEE);
}

ZTEST(row_demux, test_zero_length_resync)
{
	const uint8_t data[] = { SID_ROW_FRAME_SYNC, 0, 'h' };

	zassert_equal(sid_row_demux_feed(&demux, data[0]), SID_ROW_DEMUX_FRAME_BYTE);
	zassert_equal(sid_row_demux_feed(&demux, data[1]), SID_ROW_DEMUX_DROPPED);
	zassert_equal(sid_row_demux_feed(&demux, data[2]), SID_ROW_DEMUX_CONSOLE,
		      "back to console after an invalid length");
}

ZTEST(row_demux, test_text_after_frame)
{
	const uint8_t frame[] = { SID_ROW_FRAME_SYNC, 1, 0x42 };

	zassert_equal(feed_all(frame, sizeof(frame)), SID_ROW_DEMUX_FRAME_READY);
	zassert_equal(sid_row_demux_feed(&demux, 'h'), SID_ROW_DEMUX_CONSOLE);
}

ZTEST(row_demux, test_overrun_discards_second_frame)
{
	const uint8_t first[] = { SID_ROW_FRAME_SYNC, 2, 0x01, 0x02 };
	const uint8_t second[] = { SID_ROW_FRAME_SYNC, 2, 'a', 'b' };
	uint8_t out[SID_ROW_FRAME_PAYLOAD_MAX];

	zassert_equal(feed_all(first, sizeof(first)), SID_ROW_DEMUX_FRAME_READY);
	zassert_equal(feed_all(second, sizeof(second)), SID_ROW_DEMUX_DROPPED,
		      "frame bytes are not console text even when discarded");
	zassert_equal(demux.overrun_cnt, 1);
	zassert_equal(sid_row_demux_feed(&demux, 'h'), SID_ROW_DEMUX_CONSOLE);

	zassert_equal(sid_row_demux_take_frame(&demux, out, sizeof(out)), 2);
	zassert_mem_equal(out, &first[2], 2, "first frame kept intact");
}

ZTEST(row_demux, test_small_buffer_drops_frame)
{
	const uint8_t frame[] = { SID_ROW_FRAME_SYNC, 3, 1, 2, 3 };
	uint8_t out[2];

	zassert_equal(feed_all(frame, sizeof(frame)), SID_ROW_DEMUX_FRAME_READY);
	zassert_equal(sid_row_demux_take_frame(&demux, out, sizeof(out)), 0);
	zassert_false(demux.frame_ready, "dropped frame does not block the next one");

	zassert_equal(feed_all(frame, sizeof(frame)), SID_ROW_DEMUX_FRAME_READY);
}

ZTEST(row_demux, test_frame_encode)
{
	const uint8_t payload[] = { 0x10, 0x20, 0x30 };
	uint8_t out[SID_ROW_FRAME_PAYLOAD_MAX + SID_ROW_FRAME_HEADER_SIZE];
	const uint8_t expected[] = { SID_ROW_FRAME_SYNC, 3, 0x10, 0x20, 0x30 };

	zassert_equal(sid_row_frame_encode(payload, sizeof(payload), out, sizeof(out)),
		      sizeof(expected));
	zassert_mem_equal(out, expected, sizeof(expected));
}

ZTEST(row_demux, test_frame_encode_invalid)
{
	uint8_t payload[SID_ROW_FRAME_PAYLOAD_MAX + 1] = { 0 };
	uint8_t out[sizeof(payload) + SID_ROW_FRAME_HEADER_SIZE];

	zassert_equal(sid_row_frame_encode(payload, 0, out, sizeof(out)), 0, "empty payload");
	zassert_equal(sid_row_frame_encode(payload, sizeof(payload), out, sizeof(out)), 0,
		      "payload over 255 bytes");
	zassert_equal(sid_row_frame_encode(payload, 4, out, 5), 0, "output too small");
	zassert_equal(sid_row_frame_encode(NULL, 4, out, sizeof(out)), 0);
}

ZTEST(row_demux, test_encode_decode_round_trip)
{
	uint8_t payload[184];
	uint8_t frame[sizeof(payload) + SID_ROW_FRAME_HEADER_SIZE];
	uint8_t out[SID_ROW_FRAME_PAYLOAD_MAX];

	for (size_t i = 0; i < sizeof(payload); i++) {
		payload[i] = (uint8_t)(i * 7);
	}

	const size_t frame_len = sid_row_frame_encode(payload, sizeof(payload), frame, sizeof(frame));

	zassert_equal(frame_len, sizeof(frame));
	zassert_equal(feed_all(frame, frame_len), SID_ROW_DEMUX_FRAME_READY);
	zassert_equal(sid_row_demux_take_frame(&demux, out, sizeof(out)), sizeof(payload));
	zassert_mem_equal(out, payload, sizeof(payload));
}
