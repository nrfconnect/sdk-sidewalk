/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>

#define CMD_RETURN_OK 0
#define CMD_RETURN_HELP 1
#define CMD_RETURN_ARGUMENT_INVALID -EINVAL
#define CMD_RETURN_NOT_EXECUTED -ENOEXEC

void CLI_register_message_send();
void CLI_register_message_not_send();
void CLI_register_message_received(uint16_t resp_id);
void CLI_init(struct sid_handle *handler);
void CLI_register_sid_status(const struct sid_status *status);

void sidewalk_button_pressed(uint32_t button_bit);
