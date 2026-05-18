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

#define LORA_DT_NODE DT_CHOSEN(zephyr_lora_transceiver)
#define SPI_NODE DT_PARENT(LORA_DT_NODE)

#define SPI_OPTIONS                                                                                \
	(uint16_t)(SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER | SPI_FULL_DUPLEX)

/*
 * Use a DT-derived spi_dt_spec rather than a hand-built spi_config. Zephyr's nrfx
 * SPIM driver uses POINTER equality on `spi_config*` in spi_context_configured()
 * to decide if it can skip a full reconfigure. Passing a different (but
 * content-identical) spi_config pointer than a previous caller triggers
 * nrfx_spim_uninit() + nrfx_spim_init(skip_psel_cfg=true), which on the
 * 320 MHz SPIM30 of nRF54L15 leaves the peripheral in a state where MISO is
 * read as constant 0x00. Sticking to a single, stable spi_config pointer (the
 * one synthesized by SPI_DT_SPEC_GET) keeps the driver on the fast path.
 */
static const struct spi_dt_spec bus_serial_spec = SPI_DT_SPEC_GET(LORA_DT_NODE, SPI_OPTIONS);

struct bus_serial_ctx_t {
	const struct sid_pal_serial_bus_iface *iface;
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

	int err = spi_transceive_dt(&bus_serial_spec, ((tx) ? &tx_set : NULL),
				    ((rx) ? &rx_set : NULL));

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

	if (!spi_is_ready_dt(&bus_serial_spec)) {
		LOG_ERR("SPI device not ready");
		return SID_ERROR_IO_ERROR;
	}

	*iface = &bus_ops;

	return SID_ERROR_NONE;
}
