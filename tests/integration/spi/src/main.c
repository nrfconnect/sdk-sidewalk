/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <stdint.h>
#include <string.h>

#include <stdlib.h>

#include <sid_gpio_utils.h>
#include <sid_pal_gpio_ifc.h>
#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_serial_bus_spi_config.h>

#define NULL_STRUCT_INITIALIZER { 0 }
#define INVALID_DT_GPIO NULL_STRUCT_INITIALIZER

ZTEST(spi_bus, test_init_spi)
{
	const struct sid_pal_serial_bus_iface *interface;

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
}

ZTEST(spi_bus, test_invalid_init_spi)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_serial_bus_nordic_spi_create(NULL, NULL));
}

ZTEST(spi_bus, test_xfer_no_buffer)
{
	const struct sid_pal_serial_bus_iface *interface = NULL;

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
	zassert_not_null(interface);
	zassert_not_null(interface->xfer);
	sid_error_t e = interface->xfer(interface, NULL, NULL, NULL, 1);

	zassert_equal(SID_ERROR_INVALID_ARGS, e);
}

ZTEST(spi_bus, test_xfer_no_interface)
{
	const struct sid_pal_serial_bus_iface *interface = NULL;

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
	zassert_not_null(interface);
	zassert_not_null(interface->xfer);
	uint8_t tx[] = { 0x1d, 0x08, 0xac, 0, 0 };
	uint8_t rx[5] = { 0 };
	sid_error_t e = interface->xfer(NULL, NULL, tx, rx, sizeof(rx));

	zassert_equal(SID_ERROR_INVALID_ARGS, e);
	uint8_t rx_expect[5] = { 0 };

	zassert_mem_equal(rx_expect, rx, sizeof(rx));
}

ZTEST(spi_bus, test_xfer_wrong_interface)
{
	const struct sid_pal_serial_bus_iface *interface = NULL;
	const struct sid_pal_serial_bus_iface interface2;

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
	zassert_not_null(interface);
	zassert_not_null(interface->xfer);
	uint8_t tx[] = { 0x1d, 0x08, 0xac, 0, 0 };
	uint8_t rx[5] = { 0 };
	sid_error_t e = interface->xfer(&interface2, NULL, tx, rx, sizeof(rx));

	zassert_equal(SID_ERROR_INVALID_ARGS, e);
	uint8_t rx_expect[5] = { 0 };

	zassert_mem_equal(rx_expect, rx, sizeof(rx));
}

ZTEST(spi_bus, test_send_spi)
{
	const struct sid_pal_serial_bus_iface *interface = NULL;
	struct sid_pal_serial_bus_client client;
	client.client_selector =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_NODELABEL(sid_semtech), cs_gpios, INVALID_DT_GPIO));
	sid_pal_gpio_set_direction(client.client_selector, SID_PAL_GPIO_DIRECTION_OUTPUT);

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
	zassert_not_null(interface);
	zassert_not_null(interface->xfer);
	uint8_t tx[] = { 0x1d, 0x08, 0xac, 0, 0 };
	uint8_t rx[5] = { 0 };
	sid_error_t e = interface->xfer(interface, &client, tx, rx, sizeof(rx));

	printk("err %d\n", e);
	zassert_equal(SID_ERROR_NONE, e);
}

ZTEST(spi_bus, test_only_tx_spi)
{
	const struct sid_pal_serial_bus_iface *interface = NULL;
	struct sid_pal_serial_bus_client client;
	client.client_selector =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_NODELABEL(sid_semtech), cs_gpios, INVALID_DT_GPIO));
	sid_pal_gpio_set_direction(client.client_selector, SID_PAL_GPIO_DIRECTION_OUTPUT);

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
	zassert_not_null(interface);
	zassert_not_null(interface->xfer);
	uint8_t tx[] = { 0x1d, 0x08, 0xac };
	sid_error_t e = interface->xfer(interface, &client, tx, NULL, sizeof(tx));

	zassert_equal(SID_ERROR_NONE, e);
}

/*
 * LR11xx GetVersion WHOAMI test.
 *
 * LR11xx SPI read protocol (two separate CS transactions):
 *   Phase 1 — send command:    CS↓ TX[opcode_hi, opcode_lo] CS↑
 *   Wait BUSY low
 *   Phase 2 — read response:   CS↓ TX[0x00 × (N+1)] RX[stat, data…] CS↑
 *
 * Opcode 0x0101 = SYSTEM_GET_VERSION.
 * Response (4 bytes after status byte): hw, type, fw_hi, fw_lo.
 * Expected:
 *   type = 0x01  (LR11XX_SYSTEM_VERSION_TYPE_LR1110)
 *   hw   != 0xFF (0xFF means MISO stuck high — chip not responding / CS not asserted)
 *   fw   != 0x0000 and != 0xFFFF
 */
#if DT_NODE_EXISTS(DT_NODELABEL(lora_semtech_lr11xxmb1xxs))

#define LR11XX_NODE DT_NODELABEL(lora_semtech_lr11xxmb1xxs)

#define LR11XX_GET_VERSION_OC 0x0101u
#define LR11XX_VERSION_LEN 4u
#define LR11XX_TYPE_LR1110 0x01u
#define LR11XX_BUSY_TIMEOUT_MS 500u

static const struct gpio_dt_spec lr11xx_busy = GPIO_DT_SPEC_GET(LR11XX_NODE, busy_gpios);
static const struct gpio_dt_spec lr11xx_reset = GPIO_DT_SPEC_GET(LR11XX_NODE, reset_gpios);

static int lr11xx_wait_busy(uint32_t timeout_ms)
{
	uint32_t deadline = k_uptime_get_32() + timeout_ms;

	while (k_uptime_get_32() < deadline) {
		if (gpio_pin_get_dt(&lr11xx_busy) == 0) {
			return 0;
		}
		k_msleep(1);
	}
	return -ETIMEDOUT;
}

ZTEST(spi_bus, test_lr11xx_get_version)
{
	/* BUSY is an input. */
	zassert_ok(gpio_pin_configure_dt(&lr11xx_busy, GPIO_INPUT), "BUSY gpio configure failed");

	/* Pulse RESET: assert (active = LOW) for 5ms, then release. */
	zassert_ok(gpio_pin_configure_dt(&lr11xx_reset, GPIO_OUTPUT_ACTIVE), "RESET assert failed");
	k_msleep(5);
	zassert_ok(gpio_pin_set_dt(&lr11xx_reset, 0), "RESET release failed");

	/*
	 * Immediately after RESET release, BUSY should go HIGH while the chip boots (~185ms).
	 * Sample BUSY at 1ms, 10ms, 50ms to diagnose pin connectivity.
	 */
	k_msleep(1);
	int busy_1ms = gpio_pin_get_dt(&lr11xx_busy);
	k_msleep(9);
	int busy_10ms = gpio_pin_get_dt(&lr11xx_busy);
	k_msleep(40);
	int busy_50ms = gpio_pin_get_dt(&lr11xx_busy);

	printk("BUSY after reset: 1ms=%d 10ms=%d 50ms=%d\n", busy_1ms, busy_10ms, busy_50ms);

	/*
	 * LR1110 needs ~185ms to boot after RESET release before BUSY goes low.
	 * Wait for BUSY low then add a small settle margin.
	 */
	zassert_ok(lr11xx_wait_busy(LR11XX_BUSY_TIMEOUT_MS),
		   "LR11xx BUSY stuck high — chip not powered or reset not released");
	k_msleep(5);

	/*
	 * Use raw Zephyr SPI (spi_transceive_dt) to eliminate PAL abstraction.
	 * SPI_DT_SPEC_GET pulls freq, mode and cs-gpios directly from devicetree.
	 */
	static const struct spi_dt_spec spi = SPI_DT_SPEC_GET(
		LR11XX_NODE,
		SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER | SPI_FULL_DUPLEX, 0);

	zassert_true(spi_is_ready_dt(&spi), "SPI device not ready");

	/* Phase 1: send GetVersion command [0x01, 0x01]. */
	uint8_t cmd[2] = {
		(LR11XX_GET_VERSION_OC >> 8) & 0xFF,
		(LR11XX_GET_VERSION_OC) & 0xFF,
	};
	uint8_t stat_phase1[2] = { 0 };

	struct spi_buf tx1[] = { { .buf = cmd, .len = sizeof(cmd) } };
	struct spi_buf rx1[] = { { .buf = stat_phase1, .len = sizeof(stat_phase1) } };
	struct spi_buf_set tx1_set = { .buffers = tx1, .count = 1 };
	struct spi_buf_set rx1_set = { .buffers = rx1, .count = 1 };

	zassert_ok(spi_transceive_dt(&spi, &tx1_set, &rx1_set), "Phase-1 xfer failed");
	printk("Phase1 stat: 0x%02x 0x%02x\n", stat_phase1[0], stat_phase1[1]);

	/* Wait BUSY after command. */
	zassert_ok(lr11xx_wait_busy(LR11XX_BUSY_TIMEOUT_MS),
		   "LR11xx BUSY stuck high after GetVersion command");

	/* Phase 2: clock out 1 status byte + 4 version bytes. */
	uint8_t tx_dummy[LR11XX_VERSION_LEN + 1u] = { 0 };
	uint8_t rx[LR11XX_VERSION_LEN + 1u] = { 0 };

	struct spi_buf tx2[] = { { .buf = tx_dummy, .len = sizeof(tx_dummy) } };
	struct spi_buf rx2[] = { { .buf = rx, .len = sizeof(rx) } };
	struct spi_buf_set tx2_set = { .buffers = tx2, .count = 1 };
	struct spi_buf_set rx2_set = { .buffers = rx2, .count = 1 };

	zassert_ok(spi_transceive_dt(&spi, &tx2_set, &rx2_set), "Phase-2 xfer failed");

	/*
	 * rx[0] = stat1 (status from chip during dummy clock)
	 * rx[1] = hw version
	 * rx[2] = chip type
	 * rx[3] = fw version high byte
	 * rx[4] = fw version low byte
	 */
	uint8_t hw = rx[1];
	uint8_t type = rx[2];
	uint16_t fw = ((uint16_t)rx[3] << 8) | rx[4];

	printk("LR11xx version: hw=0x%02x type=0x%02x fw=0x%04x stat=0x%02x\n", hw, type, fw,
	       rx[0]);

	zassert_not_equal(0xFFu, hw,
			  "hw=0xFF: MISO stuck high — CS not toggling or chip unpowered");
	zassert_equal(LR11XX_TYPE_LR1110, type,
		      "unexpected chip type 0x%02x (expected 0x01=LR1110)", type);
	zassert_not_equal(0u, fw, "fw=0x0000: unexpected zero firmware version");
	zassert_not_equal(0xFFFFu, fw, "fw=0xFFFF: MISO stuck high");
}

#endif /* DT_NODE_EXISTS(lora_semtech_lr11xxmb1xxs) */

ZTEST_SUITE(spi_bus, NULL, NULL, NULL, NULL, NULL);
