/*
 * Copyright 2022-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_SERIAL_BUS_IFC_H
#define SID_PAL_SERIAL_BUS_IFC_H

/** @file
 *
 * @defgroup sid_pal_serial_bus_ifc SID serial bus interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details Provides serial bus interface to be implemented by platform
 */

#include <sid_error.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Describes the bit order of messages exchanged on serial bus interface.
 */
enum sid_pal_serial_bus_bit_order {
    /** Most significant bit first. */
    SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
    /** Least significant bit first. */
    SID_PAL_SERIAL_BUS_BIT_ORDER_LSB_FIRST,
};

/**
 * Describes the client selection or deselection
 */
enum sid_pal_serial_bus_client_select {
    /** Deselect the serial bus client */
    SID_PAL_SERIAL_BUS_CLIENT_DESELECT,
    /** Select the serial bus client */
    SID_PAL_SERIAL_BUS_CLIENT_SELECT,
};

/**
 * Describes the configuration of the serial bus client.
 */
struct sid_pal_serial_bus_client {
    /** client id on the serial bus.*/
    uint32_t client_selector;
    /** baud rate.*/
    uint32_t speed_hz;
    /** bit order.*/
    enum sid_pal_serial_bus_bit_order bit_order;
    /** serial bus mode.*/
    uint8_t mode;
    /**
     * Callback to select client on serial bus
     *
     * If this callback if not set, then  #client_selector will be used.
     *
     * @param[in] client A pointer to #sid_pal_serial_bus_client, which is never NULL
     * @param[in] select Enum value that indicates if client should be selected or not
     * @param[in] context The context pointer given in #client_selector_context
     *
     * @retval true if client selection is successful, false otherwise
     * **/
    bool (*client_selector_cb)(const struct sid_pal_serial_bus_client *const client,
                               enum sid_pal_serial_bus_client_select select,
                               void *context);
    /** Context returned back in #client_selector_cb */
    void *client_selector_context;
};

struct sid_pal_serial_bus_iface;

/**
 * The set of callbacks the implementation supports.
 */
struct sid_pal_serial_bus_iface {
    /**
     * Callback to transfer messages in full duplex mode.
     *
     * @param[in] iface pointer to serial bus interface.
     * @param[in] client pointer to serial bus client.
     * @param[in] tx pointer to the message to be sent.
     * @param[out] rx pointer to the message to be received.
     * @param[in] xfer_size maximum length of the message sent or received.
     */
    sid_error_t (*xfer)(const struct sid_pal_serial_bus_iface *iface,
                        const struct sid_pal_serial_bus_client *client,
                        uint8_t *tx,
                        uint8_t *rx,
                        size_t xfer_size);
    /**
     * Callback to transfer messages in half duplex mode.
     *
     * @param[in] iface pointer to serial bus interface.
     * @param[in] client pointer to serial bus client.
     * @param[in] tx pointer to the message to be sent.
     * @param[out] rx pointer to the message to be received.
     * @param[in] tx_size maximum length of the message to be sent.
     * @param[in] rx_size maximum length of the message to be received.
     */
    sid_error_t (*xfer_hd)(const struct sid_pal_serial_bus_iface *iface,
                           const struct sid_pal_serial_bus_client *client,
                           uint8_t *tx,
                           uint8_t *rx,
                           size_t tx_size,
                           size_t rx_size);
    /**
     * Callback to delete the serial bus interface.
     *
     * @param[in] iface pointer to serial bus interface.
     */
    sid_error_t (*destroy)(const struct sid_pal_serial_bus_iface *iface);
};

struct sid_pal_serial_bus_factory {
    /**
     * Callback to create serial bus client interface
     *
     * @param[in] iface pointer to serial bus interface.
     * @param[in] config pointer to client config.
     */
    sid_error_t (*create)(const struct sid_pal_serial_bus_iface **iface, const void *config);
    /** pointer to client config*/
    const void *config;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif /* SID_PAL_SERIAL_BUS_IFC_H */
