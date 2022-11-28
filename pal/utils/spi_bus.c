/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>

#include <spi_bus.h>
#include <sid_pal_gpio_ifc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sid_spi_bus, CONFIG_SPI_BUS_LOG_LEVEL);

#define SPI_OPTIONS (uint16_t)(SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER | SPI_FULL_DUPLEX | SPI_LOCK_ON | \
			       SPI_HOLD_ON_CS)

#define SPI_CS_DELAY_US (0U)

struct bus_serial_ctx_t {
	const halo_serial_bus_iface_t *iface;
	const struct device *device;
	struct spi_config cfg;
	struct spi_cs_control cs_cfg;
};

static halo_error_t bus_serial_spi_destroy(const halo_serial_bus_iface_t *iface);

static halo_error_t bus_serial_spi_xfer(const halo_serial_bus_iface_t *iface, const halo_serial_bus_client_t *client,
					uint8_t *tx, uint8_t *rx, size_t xfer_size);

static struct bus_serial_ctx_t bus_serial_ctx;

static const halo_serial_bus_iface_t bus_ops = {
	.xfer = bus_serial_spi_xfer,
	.destroy = bus_serial_spi_destroy,
};

static halo_error_t bus_serial_spi_xfer(const halo_serial_bus_iface_t *iface, const halo_serial_bus_client_t *client,
					uint8_t *tx, uint8_t *rx, size_t xfer_size)
{
	ARG_UNUSED(client);
	LOG_DBG("%s(%p, %p, %p, %p, %d)", __func__, iface, client, tx, rx, xfer_size);
	if (iface != bus_serial_ctx.iface || (!tx && !rx) || !xfer_size) {
		return HALO_ERROR_INVALID_ARGS;
	}

	struct spi_buf tx_buff[] = {
		{
			.buf = tx,
			.len = xfer_size,
		},
	};

	struct spi_buf_set tx_set = {
		.buffers =  tx_buff,
		.count = 1
	};

	struct spi_buf rx_buff[] = {
		{
			.buf = rx,
			.len = xfer_size,
		},
	};

	struct spi_buf_set rx_set = {
		.buffers =  rx_buff,
		.count = 1
	};

	sid_pal_gpio_write(client->client_selector, 0);
	int r =
		spi_transceive(bus_serial_ctx.device, &bus_serial_ctx.cfg, ((tx) ? &tx_set : NULL),
			       ((rx) ? &rx_set : NULL));

	sid_pal_gpio_write(client->client_selector, 1);

	if (r < 0) {
		LOG_ERR("spi_transceive failed with error %d", r);
		return HALO_ERROR_GENERIC;
	}

	return HALO_ERROR_NONE;
}

static halo_error_t bus_serial_spi_destroy(const halo_serial_bus_iface_t *iface)
{
	LOG_DBG("%s(%p)", __func__, iface);
	ARG_UNUSED(iface);
	return HALO_ERROR_NONE;
}

halo_error_t bus_serial_ncs_spi_create(const halo_serial_bus_iface_t **iface, const void *cfg)
{
	LOG_DBG("%s(%p, %p)", __func__, iface, cfg);

	ARG_UNUSED(cfg);
	if (!iface) {
		return HALO_ERROR_INCOMPATIBLE_PARAMS;
	}

	// if the device do not exist in DT, this will fail at linking process.
	const struct device *device = DEVICE_DT_GET(DT_NODELABEL(sid_spi));

	*iface = &bus_ops;
	bus_serial_ctx = (struct bus_serial_ctx_t){
		.device = device,
		.cfg = {
			.frequency = DT_PROP(DT_NODELABEL(sid_spi), clock_frequency),
			.operation = SPI_OPTIONS,
		},
		.iface = *iface
	};

	LOG_DBG("SPI device configuration:\r\n"	\
		"frequency \t= %dHz\r\n"	\
		"device flags \t= 0x%X\r\n"	\
		"cs pin \t= %s.%d; GPIO flags = %d; CS delay = %d\r\n",
		bus_serial_ctx.cfg.frequency,
		bus_serial_ctx.cfg.operation,
		bus_serial_ctx.cs_cfg.gpio.port->name,
		bus_serial_ctx.cs_cfg.gpio.pin,
		bus_serial_ctx.cs_cfg.gpio.dt_flags,
		bus_serial_ctx.cs_cfg.delay);

	return HALO_ERROR_NONE;
}
