/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>

#define SPI_MODE (SPI_WORD_SET(8) | SPI_LINES_SINGLE | SPI_TRANSFER_MSB | SPI_FULL_DUPLEX)

#define SPIM_OP (SPI_OP_MODE_MASTER | SPI_MODE)
#define SPIS_OP (SPI_OP_MODE_SLAVE | SPI_MODE)

#if CONFIG_ROLE_HOST == 1
static struct spi_dt_spec spim = SPI_DT_SPEC_GET(DT_NODELABEL(dut_spi_dt), SPIM_OP, 0);
#else
static const struct device *spis_dev = DEVICE_DT_GET(DT_NODELABEL(dut_spis));
static const struct spi_config spis_config = { .operation = SPIS_OP };
#endif

void host_start(struct spi_dt_spec *spi_dt);
void device_start(const struct device *dev, const struct spi_config *cfg);

int main(void)
{
#if CONFIG_ROLE_HOST == 1
	host_start(&spim);
#else
	device_start(spis_dev, &spis_config);
#endif
}
