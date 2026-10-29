/*
 * Copyright 2025-2026 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_SERIAL_REGISTRATION_IFC_H
#define SID_SERIAL_REGISTRATION_IFC_H

/*
 * Factory registration over a serial/wired transport.
 *
 * Stands up a lean SDK stack (state / address / memory pool / app-cmd allocator + handler / event queue
 * / message manager) plus the wired network-link PHY (sid_network_link_wired), then triggers and drives
 * factory registration to completion. It does NOT use sid_init()/sid_start(); registration app_cmds flow
 * through the message manager to the wired PHY (the only registered/connected link).
 *
 * Lifecycle (non-blocking):
 *   sid_serial_registration_init(cfg)  - build the stack + wired PHY, register the completion callback.
 *   sid_serial_registration_start()    - trigger the registration exchange; sends the first frame and
 *                                        returns immediately (does NOT block until registered).
 *   sid_serial_registration_process()  - drive one non-blocking pass (RX -> decode -> dispatch ->
 *                                        response TX -> deferred notify -> event queue). Call it from the
 *                                        app main loop; completion is signaled via cfg.on_registration_done
 *                                        (or poll sid_state_is_device_factory_registered()).
 */

#include <sid_serial_ifc.h> /* struct serial_ifc_context */

#include <sid_error.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sid_serial_registration_config {
    /* Application-provided serial/UART transport. The application must bring the transport up
     * (ifc->serial_init) before calling sid_serial_registration_init(); it is not re-initialized here. */
    const struct serial_ifc_context *ifc;
    /*
     * Invoked when the device becomes factory-registered. PRECONDITION: runs deep inside the RX /
     * handshake dispatch stack while the message manager still owns a live app_cmd (freed on unwind).
     * It MUST only set a flag. It MUST NOT call sid_serial_registration_process(), free resources, or
     * otherwise re-enter the manager or event queue.
     */
    void (*on_registration_done)(void *ctx);
    void *ctx;
};

/* Build the registration stack + wired PHY and register the completion callback. Does not trigger. */
sid_error_t sid_serial_registration_init(const struct sid_serial_registration_config *cfg);

/* Trigger the factory-registration exchange; sends the first frame and returns immediately. */
sid_error_t sid_serial_registration_start(void);

/* Drive one non-blocking pass of the registration exchange. Call from the app main loop. */
void sid_serial_registration_process(void);

/*
 * Tear down the registration stack + wired PHY brought up by sid_serial_registration_init(): disconnect
 * and deregister the wired link, deregister the state callback, and deinit the registration lib, message
 * manager, app-cmd allocator, state, and memory pool (reverse of init). After this, _start()/_process()
 * are safe no-ops until _init() is called again. Returns SID_ERROR_UNINITIALIZED if not initialized.
 * Call only from the app loop (never from on_registration_done).
 */
sid_error_t sid_serial_registration_deinit(void);

#ifdef __cplusplus
}
#endif

#endif   // SID_SERIAL_REGISTRATION_IFC_H
