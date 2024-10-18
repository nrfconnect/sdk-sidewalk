/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(host, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>

extern uint8_t random_table[];

void host_start(struct spi_dt_spec *spi_dt)
{
	static uint8_t tx[CONFIG_PAYLOAD_LEN] = { 0 };
	static uint8_t rx[CONFIG_PAYLOAD_LEN] = { 0 };
	static uint8_t rx_expect[CONFIG_PAYLOAD_LEN] = { 0 };

	if (CONFIG_PAYLOAD_LEN > 1024) {
		LOG_ERR("Can not send %d bytes of data, max limit is 1024", CONFIG_PAYLOAD_LEN);
		return;
	}
	memcpy(tx, random_table, CONFIG_PAYLOAD_LEN);

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
		bool error = false;
		// LOG_INF("Sending packet %d", packet_number);
		// LOG_HEXDUMP_INF(tx,CONFIG_PAYLOAD_LEN, "sending" );

		memcpy(rx_expect, tx, CONFIG_PAYLOAD_LEN);
		for (size_t i = 0; i < CONFIG_PAYLOAD_LEN; i++) {
			tx[i]++;
		}
		memset(rx, 0, CONFIG_PAYLOAD_LEN);

		int ret = spi_transceive_dt(spi_dt, &tx_set, &rx_set);
		if (ret != 0) {
			LOG_ERR("spi_transceive_dt returned error %d", ret);
			error = true;
		}
		int cmp = memcmp(rx, rx_expect, CONFIG_PAYLOAD_LEN);
		if (cmp != 0) {
			error = true;
			LOG_ERR("on transfer number %d Readed data does not match expected",
				packet_number);
			LOG_HEXDUMP_ERR(rx, CONFIG_PAYLOAD_LEN, "readed");
			LOG_HEXDUMP_ERR(rx_expect, CONFIG_PAYLOAD_LEN, "expected");
		}
		if (error) {
			LOG_ERR("Fail");
		} else {
			// LOG_INF("OK");
		}
		packet_number++;
		if (CONFIG_DELAY_BETWEEN_PACKETS_MS > 0) {
			k_sleep(K_MSEC(CONFIG_DELAY_BETWEEN_PACKETS_MS));
		}
	}
}
