/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_api.h>
#include <sidewalk_thread.h>

void CLI_register_message_send();
void CLI_register_message_not_send();
void CLI_register_message_received(uint16_t resp_id);
void CLI_init(app_context_t *ctx);
void CLI_register_sid_status(const struct sid_status *status);

// add handler to send message
void sidewalk_send_message(struct sid_msg msg);
