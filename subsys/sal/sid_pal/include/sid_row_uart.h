/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_ROW_UART_H
#define SID_ROW_UART_H

#include <sid_serial_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback called from interrupt context when a complete frame is received.
 */
typedef void (*sid_row_uart_frame_ready_cb_t)(void);

/**
 * @brief Get the Registration over Wire transport for the Sidewalk serial registration.
 *
 * The transport shares the UART with the shell backend. Console text goes to
 * the shell and the 0xAA-framed messages go to the serial registration.
 *
 * @return Serial interface context.
 */
struct serial_ifc_context *sid_row_uart_get_ifc(void);

/**
 * @brief Set the callback for the received frames.
 *
 * @param cb Callback, NULL to disable.
 */
void sid_row_uart_set_frame_ready_cb(sid_row_uart_frame_ready_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif /* SID_ROW_UART_H */
