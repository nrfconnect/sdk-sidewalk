/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/devicetree.h>

#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_gpio_ifc.h>
#include <app_subGHz_config.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sid_spi_bus, CONFIG_SPI_BUS_LOG_LEVEL);

#define LORA_DT_NODE DT_CHOSEN(zephyr_lora_transceiver)

#define SPI_OPTIONS                                                                                \
	(uint16_t)(SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER | SPI_FULL_DUPLEX)

static const struct spi_dt_spec bus_serial_spec = SPI_DT_SPEC_GET(LORA_DT_NODE, SPI_OPTIONS);

#if defined(CONFIG_SIDEWALK_SPI_BUS_IDLE_SUSPEND_DELAY_MS) &&                                      \
	(CONFIG_SIDEWALK_SPI_BUS_IDLE_SUSPEND_DELAY_MS > 0)
/*
 * The nRF SPIM runtime PM is ISR safe, so pm_device_runtime_put_async() ignores its delay.
 * Hold one reference while the bus is busy and drop it from a delayed work instead.
 *
 * k_work_cancel_delayable() cannot tell a cancelled release from one that already ran,
 * so bus_pm_held records whether the reference is currently taken.
 */
static atomic_t bus_pm_held;

static void bus_idle_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	if (atomic_cas(&bus_pm_held, 1, 0)) {
		int err = pm_device_runtime_put(bus_serial_spec.bus);

		if (err < 0) {
			LOG_ERR("spi pm put err %d", err);
		}
	}
}

static K_WORK_DELAYABLE_DEFINE(bus_idle_work, bus_idle_work_handler);

static int bus_pm_hold(void)
{
	struct k_work_sync sync;
	int err = 0;

	(void)k_work_cancel_delayable_sync(&bus_idle_work, &sync);

	if (atomic_cas(&bus_pm_held, 0, 1)) {
		err = pm_device_runtime_get(bus_serial_spec.bus);
		if (err < 0) {
			atomic_clear(&bus_pm_held);
		}
	}

	return err;
}

static void bus_pm_release_later(void)
{
	(void)k_work_reschedule(&bus_idle_work, K_MSEC(CONFIG_SIDEWALK_SPI_BUS_IDLE_SUSPEND_DELAY_MS));
}
#else
static int bus_pm_hold(void)
{
	return 0;
}

static void bus_pm_release_later(void)
{
}
#endif /* CONFIG_SIDEWALK_SPI_BUS_IDLE_SUSPEND_DELAY_MS */

static sid_error_t zephyr_spi_bus_xfer(const struct sid_pal_serial_bus_iface *iface,
				       const struct sid_pal_serial_bus_client *client, uint8_t *tx,
				       uint8_t *rx, size_t xfer_size);
static sid_error_t zephyr_spi_bus_destroy(const struct sid_pal_serial_bus_iface *iface);

static const struct sid_pal_serial_bus_iface zephyr_spi_bus_iface = {
	.xfer = zephyr_spi_bus_xfer,
	.destroy = zephyr_spi_bus_destroy,
};

static sid_error_t zephyr_spi_bus_xfer(const struct sid_pal_serial_bus_iface *iface,
				       const struct sid_pal_serial_bus_client *client, uint8_t *tx,
				       uint8_t *rx, size_t xfer_size)
{
	LOG_DBG("%s(%p, %p, %p, %p, %d)", __func__, iface, client, (void *)tx, (void *)rx,
		xfer_size);

	sid_error_t ret = SID_ERROR_NONE;

	if (iface != &zephyr_spi_bus_iface || (!tx && !rx) || !xfer_size || !client) {
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

	int err = bus_pm_hold();

	if (err < 0) {
		LOG_ERR("spi pm get err %d", err);
		return SID_ERROR_GENERIC;
	}

	err = spi_transceive_dt(&bus_serial_spec, ((tx) ? &tx_set : NULL), ((rx) ? &rx_set : NULL));

	bus_pm_release_later();

	if (err < 0) {
		LOG_ERR("spi xfer err %d", err);
		ret = SID_ERROR_GENERIC;
	}

	return ret;
}

static sid_error_t zephyr_spi_bus_destroy(const struct sid_pal_serial_bus_iface *iface)
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

	*iface = &zephyr_spi_bus_iface;

	return SID_ERROR_NONE;
}
