/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/devicetree.h>

#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_gpio_ifc.h>
#include <app_subGHz_config.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sid_spi_bus, CONFIG_SPI_BUS_LOG_LEVEL);

#define SPI_NODE DT_PARENT(DT_CHOSEN(zephyr_lora_transceiver))

#define SPI_OPTIONS                                                                                \
	(uint16_t)(SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER | SPI_FULL_DUPLEX)

struct bus_serial_ctx_t {
	const struct sid_pal_serial_bus_iface *iface;
	const struct device *device;
	struct spi_config cfg;
};

static sid_error_t bus_serial_spi_xfer(const struct sid_pal_serial_bus_iface *iface,
				       const struct sid_pal_serial_bus_client *client, uint8_t *tx,
				       uint8_t *rx, size_t xfer_size);
static sid_error_t bus_serial_spi_destroy(const struct sid_pal_serial_bus_iface *iface);

static const struct sid_pal_serial_bus_iface bus_ops = {
	.xfer = bus_serial_spi_xfer,
	.destroy = bus_serial_spi_destroy,
};

static struct bus_serial_ctx_t bus_serial_ctx = {
	.iface = &bus_ops,
	.device = DEVICE_DT_GET(SPI_NODE),
	.cfg = (struct spi_config){ .operation = SPI_OPTIONS },
};

static sid_error_t bus_serial_spi_xfer(const struct sid_pal_serial_bus_iface *iface,
				       const struct sid_pal_serial_bus_client *client, uint8_t *tx,
				       uint8_t *rx, size_t xfer_size)
{
	LOG_DBG("%s(%p, %p, %p, %p, %d)", __func__, iface, client, (void *)tx, (void *)rx,
		xfer_size);

	sid_error_t ret = SID_ERROR_NONE;

	if (iface != bus_serial_ctx.iface || (!tx && !rx) || !xfer_size || !client) {
		return SID_ERROR_INVALID_ARGS;
	}

	struct spi_buf tx_buff[] = {
		{
			.buf = tx,
			.len = xfer_size,
		},
	};

	struct spi_buf_set tx_set = { .buffers = tx_buff, .count = 1 };

	struct spi_buf rx_buff[] = {
		{
			.buf = rx,
			.len = xfer_size,
		},
	};

	struct spi_buf_set rx_set = { .buffers = rx_buff, .count = 1 };

	int err = spi_transceive(bus_serial_ctx.device, &bus_serial_ctx.cfg,
				 ((tx) ? &tx_set : NULL), ((rx) ? &rx_set : NULL));

	if (err < 0) {
		LOG_ERR("spi xfer err %d", err);
		ret = SID_ERROR_GENERIC;
	}

	return ret;
}

static sid_error_t bus_serial_spi_destroy(const struct sid_pal_serial_bus_iface *iface)
{
	LOG_DBG("%s(%p)", __func__, iface);
	if (!iface) {
		return SID_ERROR_INVALID_ARGS;
	}
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_serial_bus_nordic_spi_create(const struct sid_pal_serial_bus_iface **iface,
						 const void *cfg)
{
	ARG_UNUSED(cfg);

	if (!iface) {
		return SID_ERROR_INVALID_ARGS;
	}

	if (!device_is_ready(bus_serial_ctx.device)) {
		LOG_ERR("SPI device not ready");
		return SID_ERROR_IO_ERROR;
	}

	*iface = &bus_ops;
	bus_serial_ctx.cfg.frequency = get_radio_cfg()->bus_selector.speed_hz;
	bus_serial_ctx.cfg.cs.delay = 0;
	bus_serial_ctx.cfg.cs.gpio = (struct gpio_dt_spec)GPIO_DT_SPEC_GET(SPI_NODE, cs_gpios);

	return SID_ERROR_NONE;
}
