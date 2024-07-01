/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SIDEWALK_MOCK_H
#define SIDEWALK_MOCK_H

#include <zephyr/fff.h>
#include <sidewalk.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(sidewalk_start, sidewalk_ctx_t *);
FAKE_VALUE_FUNC(void *, sid_hal_malloc, size_t);
FAKE_VOID_FUNC(sid_hal_free, void *);
FAKE_VALUE_FUNC(int, sidewalk_event_send, event_handler_t, void *, ctx_free);

FAKE_VOID_FUNC(sidewalk_event_process, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_autostart, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_factory_reset, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_new_status, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_send_msg, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_connect, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_link_switch, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_file_transfer, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_exit, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_reboot, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(sidewalk_event_platform_init, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_init, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_deinit, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_start, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_stop, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_get_mtu, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_get_time, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_get_status, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_get_option, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_set_option, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_set_dest_id, sidewalk_ctx_t *, void *);
FAKE_VOID_FUNC(dut_event_conn_req, sidewalk_ctx_t *, void *);

#define SIDEWALK_FAKES_LIST(FAKE)                                                                  \
	FAKE(sidewalk_start)                                                                       \
	FAKE(sid_hal_malloc)                                                                       \
	FAKE(sid_hal_free)                                                                         \
	FAKE(sidewalk_event_send)                                                                  \
	FAKE(sidewalk_event_process)                                                               \
	FAKE(sidewalk_event_autostart)                                                             \
	FAKE(sidewalk_event_factory_reset)                                                         \
	FAKE(sidewalk_event_new_status)                                                            \
	FAKE(sidewalk_event_send_msg)                                                              \
	FAKE(sidewalk_event_connect)                                                               \
	FAKE(sidewalk_event_link_switch)                                                           \
	FAKE(sidewalk_event_file_transfer)                                                         \
	FAKE(sidewalk_event_exit)                                                                  \
	FAKE(sidewalk_event_reboot)                                                                \
	FAKE(sidewalk_event_platform_init)                                                         \
	FAKE(dut_event_init)                                                                       \
	FAKE(dut_event_deinit)                                                                     \
	FAKE(dut_event_start)                                                                      \
	FAKE(dut_event_stop)                                                                       \
	FAKE(dut_event_get_mtu)                                                                    \
	FAKE(dut_event_get_time)                                                                   \
	FAKE(dut_event_get_status)                                                                 \
	FAKE(dut_event_get_option)                                                                 \
	FAKE(dut_event_set_option)                                                                 \
	FAKE(dut_event_set_dest_id)                                                                \
	FAKE(dut_event_conn_req)

#endif /* SIDEWALK_MOCK_H */
