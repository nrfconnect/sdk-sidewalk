/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <zephyr/kernel.h>
#include <stdint.h>
#include <string.h>

#include <stdlib.h>

#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_serial_bus_spi_config.h>

// Real pin number for sx1262 NSS for nrf52840
#define NSS_PIN_NUMBER 40

struct sid_pal_serial_bus_client client =
	(struct sid_pal_serial_bus_client){ .client_selector = NSS_PIN_NUMBER };

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

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
	zassert_not_null(interface);
	zassert_not_null(interface->xfer);
	uint8_t tx[] = { 0x1d, 0x08, 0xac, 0, 0 };
	uint8_t rx[5] = { 0 };
	uint8_t invalid_rx[5] = { 0 };
	sid_error_t e = interface->xfer(interface, &client, tx, rx, sizeof(rx));

	zassert_equal(SID_ERROR_NONE, e);
	for (int i = 0; i < ARRAY_SIZE(rx); i++) {
		printf("%x ", rx[i]);
	}
	zassert_true(memcmp(rx, invalid_rx, ARRAY_SIZE(rx)) != 0);
}

ZTEST(spi_bus, test_only_tx_spi)
{
	const struct sid_pal_serial_bus_iface *interface = NULL;

	zassert_equal(SID_ERROR_NONE, sid_pal_serial_bus_nordic_spi_create(&interface, NULL));
	zassert_not_null(interface);
	zassert_not_null(interface->xfer);
	uint8_t tx[] = { 0x1d, 0x08, 0xac };
	sid_error_t e = interface->xfer(interface, &client, tx, NULL, sizeof(tx));

	zassert_equal(SID_ERROR_NONE, e);
}

ZTEST(spi_bus, test_sanity)
{
	zassert_equal(true, true);
}

ZTEST_SUITE(spi_bus, NULL, NULL, NULL, NULL, NULL);
