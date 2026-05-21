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

#define NULL_STRUCT_INITIALIZER                                                                    \
	{                                                                                          \
		0                                                                                  \
	}
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

/* LR11xx GetVersion smoke test using sid_pal serial bus interface. */
#if DT_NODE_EXISTS(DT_NODELABEL(lora_semtech_lr11xxmb1xxs))

#define LR11XX_NODE DT_NODELABEL(lora_semtech_lr11xxmb1xxs)

#define LR11XX_GET_VERSION_OC 0x0101u
#define LR11XX_VERSION_LEN 4u
#define LR11XX_TYPE_LR1110 0x01u
#define LR11XX_BUSY_TIMEOUT_MS 500u

static const struct gpio_dt_spec lr11xx_busy = GPIO_DT_SPEC_GET(LR11XX_NODE, busy_gpios);

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
	const struct sid_pal_serial_bus_iface *interface = NULL;
	struct sid_pal_serial_bus_client client;
	uint8_t cmd[] = {
		(uint8_t)((LR11XX_GET_VERSION_OC >> 8) & 0xFF),
		(uint8_t)(LR11XX_GET_VERSION_OC & 0xFF),
	};
	uint8_t tx_dummy[LR11XX_VERSION_LEN + 1u] = { 0 };
	uint8_t rx[LR11XX_VERSION_LEN + 1u] = { 0 };

	client.client_selector = sid_gpio_utils_register_gpio(
		(struct gpio_dt_spec)SPI_CS_GPIOS_DT_SPEC_GET(LR11XX_NODE));
	sid_pal_gpio_set_direction(client.client_selector, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_ok(gpio_pin_configure_dt(&lr11xx_busy, GPIO_INPUT), "BUSY gpio configure failed");

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
	zassert_not_null(interface);
	zassert_not_null(interface->xfer);

	/* LR11xx read command is two SPI transactions. */
	zassert_ok(lr11xx_wait_busy(LR11XX_BUSY_TIMEOUT_MS), "BUSY stuck high before command");
	zassert_equal(SID_ERROR_NONE, interface->xfer(interface, &client, cmd, NULL, sizeof(cmd)));
	zassert_ok(lr11xx_wait_busy(LR11XX_BUSY_TIMEOUT_MS), "BUSY stuck high after command");
	zassert_equal(SID_ERROR_NONE,
		      interface->xfer(interface, &client, tx_dummy, rx, sizeof(rx)));

	printk("LR11xx version: hw=0x%02x type=0x%02x fw=0x%04x stat=0x%02x\n", rx[1], rx[2],
	       ((uint16_t)rx[3] << 8) | rx[4], rx[0]);

	zassert_not_equal(0xFFu, rx[1], "hw=0xFF: radio not responding");
	zassert_equal(LR11XX_TYPE_LR1110, rx[2], "unexpected chip type");
}

#endif /* DT_NODE_EXISTS(lora_semtech_lr11xxmb1xxs) */

ZTEST_SUITE(spi_bus, NULL, NULL, NULL, NULL, NULL);
