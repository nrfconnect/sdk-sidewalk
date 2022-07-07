/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef SPI_BUS_H
#define SPI_BUS_H

#include <dev/bus/serial/serial.h>

halo_error_t bus_serial_ncs_spi_create(const halo_serial_bus_iface_t **iface, const void *cfg);

#endif // SPI_BUS_H
