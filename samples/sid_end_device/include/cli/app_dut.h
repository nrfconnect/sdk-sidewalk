/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_DUT_H
#define APP_DUT_H

#include <sidewalk.h>

void dut_event_init(sidewalk_ctx_t *sid, void *ctx);
void dut_event_deinit(sidewalk_ctx_t *sid, void *ctx);
void dut_event_start(sidewalk_ctx_t *sid, void *ctx);
void dut_event_stop(sidewalk_ctx_t *sid, void *ctx);
void dut_event_get_mtu(sidewalk_ctx_t *sid, void *ctx);
void dut_event_get_time(sidewalk_ctx_t *sid, void *ctx);
void dut_event_get_status(sidewalk_ctx_t *sid, void *ctx);
void dut_event_get_option(sidewalk_ctx_t *sid, void *ctx);
void dut_event_set_option(sidewalk_ctx_t *sid, void *ctx);
void dut_event_set_dest_id(sidewalk_ctx_t *sid, void *ctx);
void dut_event_conn_req(sidewalk_ctx_t *sid, void *ctx);

#endif /* APP_DUT_H */
