/*
 * Copyright 2026 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_SERIAL_IFC_H
#define SID_SERIAL_IFC_H

/*
 * Application-provided serial (UART) transport seam.
 *
 * The application owns the physical serial/UART hardware and supplies these callbacks. Both the wired
 * network-link PHY (sid_network_link_wired) and the registration lifecycle (sid_serial_registration)
 * bind to this seam. Framing (e.g. 0xAA + length) is the application's responsibility inside these
 * callbacks; serial_read returns one complete de-framed message, serial_write frames one message.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sid_serial_read_callback_t)(uint8_t *data, size_t len, size_t *bytes_read);
typedef void (*sid_serial_write_callback_t)(const uint8_t *data, size_t len, size_t *bytes_written);
typedef void (*sid_serial_mfg_data_write_callback_t)(const uint8_t *data, size_t len, size_t *bytes_written);
typedef void (*sid_serial_ifc_init_t)();

struct serial_ifc_context {
    sid_serial_read_callback_t serial_read;
    sid_serial_write_callback_t serial_write;
    sid_serial_ifc_init_t serial_init;
    sid_serial_mfg_data_write_callback_t serial_mfg_data_write;
    void *serial_ctx;   // for additional data if required
};

#ifdef __cplusplus
}
#endif

#endif /* SID_SERIAL_IFC_H */
