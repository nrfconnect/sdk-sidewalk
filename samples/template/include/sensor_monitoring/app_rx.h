/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_RX_H
#define APP_RX_H

#include <stdint.h>

#define APP_RX_PAYLOAD_MAX_SIZE 255

struct app_rx_msg {
	uint8_t pld_size;
	uint8_t rx_payload[APP_RX_PAYLOAD_MAX_SIZE];
};

int app_rx_msg_received(struct app_rx_msg *rx_msg);

void app_rx_task(void *dummy1, void *dummy2, void *dummy3);

#endif /* APP_RX_H */
