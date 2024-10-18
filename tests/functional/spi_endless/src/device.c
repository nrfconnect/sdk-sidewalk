/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(device, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
extern uint8_t random_table[];

void device_start(const struct device *dev, const struct spi_config *cfg)
{
	static uint8_t tx[CONFIG_PAYLOAD_LEN] = { 0 };
	static uint8_t rx[CONFIG_PAYLOAD_LEN] = { 0 };
	static uint8_t rx_expect[CONFIG_PAYLOAD_LEN] = { 0 };
	memcpy(rx_expect, random_table, CONFIG_PAYLOAD_LEN);

	struct spi_buf tx_buff[] = {
		{
			.buf = tx,
			.len = CONFIG_PAYLOAD_LEN,
		},
	};

	struct spi_buf_set tx_set = { .buffers = tx_buff, .count = 1 };

	struct spi_buf rx_buff[] = {
		{
			.buf = rx,
			.len = CONFIG_PAYLOAD_LEN,
		},
	};
	struct spi_buf_set rx_set = { .buffers = rx_buff, .count = 1 };
	size_t packet_number = 0;

	while (true) {
		int ret = spi_transceive(dev, cfg, &tx_set, &rx_set);
		if (ret <= 0) {
			LOG_ERR("spi_transceive_dt returned %d", ret);
		}
		LOG_INF("received packet number %d", packet_number++);

		int cmp = memcmp(rx, rx_expect, CONFIG_PAYLOAD_LEN);
		if (cmp != 0) {
			LOG_ERR("Received unexpected payload");
			LOG_HEXDUMP_ERR(rx, CONFIG_PAYLOAD_LEN, "received");
			LOG_HEXDUMP_ERR(rx_expect, CONFIG_PAYLOAD_LEN, "expected");
			LOG_ERR("Fail");
		} else {
			// LOG_INF("OK");
		}

		// prepare tx for next response
		memcpy(tx, rx, CONFIG_PAYLOAD_LEN);
		// LOG_HEXDUMP_INF(rx, CONFIG_PAYLOAD_LEN, "received");
		// calculate what will be next rx payload
		for (size_t i = 0; i < CONFIG_PAYLOAD_LEN; i++) {
			uint8_t expected_val = rx[i] + 1;
			rx_expect[i] = expected_val;
		}
	}
}
