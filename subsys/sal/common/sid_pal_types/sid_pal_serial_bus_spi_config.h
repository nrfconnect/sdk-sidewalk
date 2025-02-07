/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.  This file is a Modifiable
 * File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_SERIAL_BUS_SPI_CONFIG_H
#define SID_PAL_SERIAL_BUS_SPI_CONFIG_H

#include <sid_pal_serial_bus_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sid_pal_serial_bus_nordic_spi_config {
    uint32_t gpio_sck;
    uint32_t gpio_mosi;
    uint32_t gpio_miso;
    uint32_t gpio_default_nss;

    const void *ll_instance;
};

sid_error_t sid_pal_serial_bus_nordic_spi_create(const struct sid_pal_serial_bus_iface **iface,
                                                 const void *const context);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_PAL_SERIAL_BUS_SPI_CONFIG_H */
