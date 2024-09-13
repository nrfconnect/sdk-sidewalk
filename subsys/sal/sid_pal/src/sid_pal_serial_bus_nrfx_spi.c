/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app_subGHz_config.h>
#include <sid_pal_serial_bus_ifc.h>
#include <nrfx_spi.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sid_nrfx_spi_bus, CONFIG_SPI_BUS_LOG_LEVEL);

#define SPI_INSTANCE_ID CONFIG_SIDEWALK_NRFX_SPI_ID
#define SPI_SCK_PIN NRF_DT_GPIOS_TO_PSEL(DT_NODELABEL(nrfx_spi_sck), gpios)
#define SPI_MOSI_PIN NRF_DT_GPIOS_TO_PSEL(DT_NODELABEL(nrfx_spi_mosi), gpios)
#define SPI_MISO_PIN NRF_DT_GPIOS_TO_PSEL(DT_NODELABEL(nrfx_spi_miso), gpios)
#define SPI_CS_PIN NRF_DT_GPIOS_TO_PSEL(DT_NODELABEL(nrfx_spi_cs), gpios)

static const nrfx_spi_t spi = NRFX_SPI_INSTANCE(SPI_INSTANCE_ID);

static sid_error_t bus_serial_spi_xfer(const struct sid_pal_serial_bus_iface *iface,
				       const struct sid_pal_serial_bus_client *client, uint8_t *tx,
				       uint8_t *rx, size_t xfer_size);
static sid_error_t bus_serial_spi_destroy(const struct sid_pal_serial_bus_iface *iface);

static const struct sid_pal_serial_bus_iface bus_ops = {
	.xfer = bus_serial_spi_xfer,
	.destroy = bus_serial_spi_destroy,
};

static sid_error_t bus_serial_spi_xfer(const struct sid_pal_serial_bus_iface *iface,
				       const struct sid_pal_serial_bus_client *client, uint8_t *tx,
				       uint8_t *rx, size_t xfer_size)
{
	ARG_UNUSED(iface);
	ARG_UNUSED(client);

	sid_error_t ret = SID_ERROR_NONE;
	nrfx_err_t result;
	nrfx_spi_xfer_desc_t trx_set =
		NRFX_SPI_XFER_TRX(tx, tx ? xfer_size : 0, rx, rx ? xfer_size : 0);
	nrfx_spi_config_t config =
		NRFX_SPI_DEFAULT_CONFIG(SPI_SCK_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_CS_PIN);

	config.frequency = get_radio_cfg()->bus_selector.speed_hz;

	LOG_DBG("%s(%p, %p, %d)", __func__, (void *)tx, (void *)rx, xfer_size);

	result = nrfx_spi_init(&spi, &config, NULL, NULL);
	if (result != NRFX_SUCCESS) {
		LOG_ERR("NRFX driver init fail: %08x", result);
		return SID_ERROR_IO_ERROR;
	}

	result = nrfx_spi_xfer(&spi, &trx_set, 0);
	if (result != NRFX_SUCCESS) {
		LOG_ERR("NRFX driver transfer fail: %08x", result);
		ret = SID_ERROR_GENERIC;
	}

	nrfx_spi_uninit(&spi);

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

	*iface = &bus_ops;

	return SID_ERROR_NONE;
}
