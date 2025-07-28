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

ZTEST_SUITE(spi_bus, NULL, NULL, NULL, NULL, NULL);
