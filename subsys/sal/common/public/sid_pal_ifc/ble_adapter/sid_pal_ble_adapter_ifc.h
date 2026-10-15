/*
 * Copyright 2020-2026 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_BLE_ADAPTER_IFC_H
#define SID_PAL_BLE_ADAPTER_IFC_H

/** @file
 *
 * @defgroup sid_pal_ble_adapter_ifc SID BLE adapter interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details  Provides ble adapter interface to be implemented by the platform.
 */

#include <sid_ble_config_ifc.h>
#include <sid_error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sid_pal_ble_adapter_interface *sid_pal_ble_adapter_interface_t;

/**
 * This callback should be defined outside a module which defines implementation.
 * Data callback is triggered when RX data is received over the link.
 *
 * @param [out] id     - provides the information on which service data
 *                       is received.
 * @param [out] data   - received RX data.
 * @param [out] length - size of received buffer in bytes.
 */
typedef void (*sid_pal_ble_data_callback_t)(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length);

/**
 * This callback should be defined outside a module which defines implementation.
 * Notification callback is triggered when a notification is received over a
 * particular service.
 *
 * @param [out] id    - provides the information on which service the
 *                      notification is received.
 * @param [out] state - provides state of the notification(Enabled/Disabled).
 */
typedef void (*sid_pal_ble_notify_callback_t)(sid_ble_cfg_service_identifier_t id, bool state);

/**
 * This callback should be defined outside a module which defines implementation.
 * Connection callback is triggered to indicate the state of the link.
 *
 * @param [out] state - indicates the state of the link (Connected/Disconnected).
 * @param [out] addr  - provides the peer BLE mac address.
 */
typedef void (*sid_pal_ble_connection_callback_t)(bool state, uint8_t *addr);

/**
 * This callback should be defined outside a module which defines implementation.
 * Indication callback is triggered to indicate the transmission status.
 *
 * @param [out] status - indicates the transmission state.
 *                       true - success, false - failure.
 */
typedef void (*sid_pal_ble_indication_callback_t)(bool status);

/**
 * This callback should be defined outside a module which defines implementation.
 * Mtu callback provides the negotiated mtu with peer over the link.
 *
 * @param [out] size - negotiated mtu size.
 */
typedef void (*sid_pal_ble_mtu_callback_t)(uint16_t size);

/**
 * This callback should be defined outside a module which defines implementation.
 * Callback is triggered to notify that advertisement is started.
 *
 * @param [out] size - negotiated mtu size.
 */
typedef void (*sid_pal_ble_adv_start_callback_t)(void);

/**
 * This callback should be defined outside a module which defines implementation.
 * Bond callback is triggered when BLE bonding completes or fails.
 *
 * @param [out] bonded - true if bonding succeeded, false if bonding failed or bond was lost.
 * @param [out] addr   - peer BLE address of the bonded device (NULL on failure).
 */
typedef void (*sid_pal_ble_bond_callback_t)(bool bonded, uint8_t *addr);

/**
 * Bonding control commands for the bond_control interface.
 */
enum sid_pal_ble_bond_cmd {
    /** Enable bonding mode — accept pairing requests from peers */
    SID_PAL_BLE_BOND_CMD_ENABLE = 0,
    /** Disable bonding mode — reject pairing requests */
    SID_PAL_BLE_BOND_CMD_DISABLE,
    /** Delete all stored bonds from persistent storage */
    SID_PAL_BLE_BOND_CMD_DELETE_ALL,
    /** Delete bond for a specific peer. data = uint8_t* bonding handle */
    SID_PAL_BLE_BOND_CMD_DELETE_PEER,
    /** Query if any bond exists. data = bool* (output: true if bonded) */
    SID_PAL_BLE_BOND_CMD_IS_BONDED,
    /**
     * Query if the peer on the *current* connection is bonded. Unlike
     * IS_BONDED (which reports whether any bond exists in the persistent
     * store), this reflects the live connection: true only while connected to
     * a peer whose address resolved to a stored bond (i.e. the bonded owner).
     * Returns false when disconnected or connected to a non-bonded peer.
     * data = bool* (output: true if the current connection is bonded)
     */
    SID_PAL_BLE_BOND_CMD_IS_CONN_BONDED,
};

/**
 * @struct sid_pal_ble_adapter_callbacks_t
 * @brief Defines types of callbacks.
 *
 * These callbacks are set from the Network Interface using set_callback.
 * When specific runtime events occur, the callbacks are triggered for BLE
 * network interface to take appropriate actions.
 */
typedef struct {
    sid_pal_ble_data_callback_t data_callback;
    sid_pal_ble_notify_callback_t notify_callback;
    sid_pal_ble_connection_callback_t conn_callback;
    sid_pal_ble_indication_callback_t ind_callback;
    sid_pal_ble_mtu_callback_t mtu_callback;
    sid_pal_ble_adv_start_callback_t adv_start_callback;
    sid_pal_ble_bond_callback_t bond_callback;
} sid_pal_ble_adapter_callbacks_t;

struct sid_pal_ble_adapter_interface {
    /**
     * Initialize the BLE stack with the BLE configuration.
     *
     * @param [in] cfg - contains BLE configuration.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*init)(const sid_ble_config_t *cfg);

    /**
     * Update the BLE advertising configuration.
     *
     * @param [in] cfg - contains BLE user configuration.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*user_config)(sid_ble_user_config_t *cfg);

    /**
     * Start the service using the configuration.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*start_service)(void);

    /**
     * Set the advertisement data provided by Network Interface to BLE stack.
     *
     * @param [in] data   - advertisement payload.
     * @param [in] length - size of advertisement payload.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*set_adv_data)(uint8_t *data, uint8_t length);

    /**
     * Start advertisement.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*start_adv)(void);

    /**
     * Stop advertisement.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*stop_adv)(void);

    /**
     * Get RSSI of recieved data
     *
     * @return SID_ERROR_NONE - In case method is successful
     *          In case of error, the error type depends on implementation
     */
    sid_error_t (*get_rssi)(int8_t *rssi);

    /**
     * Get tx power value
     * Interface gets an upper limit on the Tx power previously set by user.
     * In case user has not set Tx power yet, default value of Tx power
     * for current platform is returned.
     *
     * @param [out] tx_power - actual upper limit for the Tx power.
     *
     * @return SID_ERROR_NONE - In case method is successful
     *          In case of error, the error type depends on implementation
     */
    sid_error_t (*get_tx_pwr)(int16_t *tx_power);

    /**
     * Sends the data provided by the Network Interface over the air.
     *
     * @param [in] id     - service id on which data to be sent.
     * @param [in] data   - data packet to be sent.
     * @param [in] length - size of data packet.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*send)(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length);

    /**
     * Used to set the defined callbacks from the Network Interface.
     *
     * @param [in] cb - callbacks set by Network Interface.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*set_callback)(const sid_pal_ble_adapter_callbacks_t *cb);

    /**
     * Sets Tx power value.
     * Interface sets an upper limit on the Tx power. The provided argument
     * is rounded to the nearest (less or equal) supported value by the platform
     * and then set as the maximum allowable Tx power. That is why arguments
     * after sequence set_tx_pwr(x) and get_tx_pwr(x) can be different.
     * FYI, hardware can have its own automatic gain control.
     *
     * @param [in] tx_power - desired upper limit for the Tx power.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*set_tx_pwr)(int16_t tx_power);

    /**
     * If defined, forwards the result of sid_pal_ble_data_callback_t data_callback.
     *
     * @param [in] result - result returned from data callback.
     */
    void (*received_data_result)(sid_error_t result);

    /**
     * Disconnect the link.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*disconnect)(void);

    /**
     * Deinitialize underneath BLE stack.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*deinit)(void);

    /**
     * Notify AMA status to ble adapter
     *
     * @param [in] is_active   - indicate wheather AMA is active.
     *
     * @return void.
     */
    void (*notify_ama_state)(bool is_active);

    /**
     * Get BLE public mac address
     *
     * @param [out] addr   - current ble public mac address.
     *
     * @return void.
     */
    sid_error_t (*get_mac_addr)(uint8_t *addr);

    /**
     * BLE bonding control interface.
     *
     * Accepts a bonding command and optional data to control the security
     * manager's bonding behavior. See sid_pal_ble_bond_cmd for commands.
     *
     * @param [in] cmd  - bonding control command.
     * @param [in] data - command-specific data (may be NULL depending on cmd).
     * @param [in] size - size of data in bytes (0 when data is NULL).
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         SID_ERROR_NOSUPPORT - if bonding is not supported on this platform.
     *         In case of error, the error type depend upon implementation.
     */
    sid_error_t (*bond_control)(enum sid_pal_ble_bond_cmd cmd, void *data, size_t size);

    /**
     * Get the BLE address currently used by the local advertiser.
     *
     * Returns the address that the BLE stack is presently advertising with.
     * When MAC rotation is enabled this reflects the rotated (resolvable
     * private) address rather than the static public address returned by
     * @ref get_mac_addr, and can be used to verify address rotation.
     *
     * @param [out] addr   - buffer to receive the current advertiser address.
     *                       Must be large enough to hold a BLE device
     *                       address (6 bytes).
     *
     * @return SID_ERROR_NONE          - on success.
     *         SID_ERROR_INVALID_ARGS  - if @p addr is NULL.
     *         SID_ERROR_NOSUPPORT     - if the platform does not implement
     *                                   advertiser address retrieval.
     *         Other sid_error_t values may be returned depending on the
     *         underlying BLE stack implementation.
     */
    sid_error_t (*get_advertiser_addr)(uint8_t *addr);

    /**
     * Request that advertising (re)enter the FAST phase now. Optional (may be
     * NULL); when NULL start_adv keeps its default behavior of always beginning
     * fast. The SAL calls this only on an external "make me discoverable" event
     * (fresh start, reconnect, connection request, factory reset, GCS time
     * request, D2D config) -- never on a TX-ID rotation restart.
     *
     * The PAL owns the fast->slow transition and preserves its own phase across
     * a stop/start, so the SAL does not drive the slow phase: on rotation it
     * simply does not call this, and the PAL's own fast-window timer carries the
     * device from fast back to slow. This is why the hint is one-way (force
     * fast) rather than a fast/slow selector -- the SAL has no view of the PAL's
     * fast-window deadline and must not reset a device already progressing to
     * slow.
     *
     * Advertising continues indefinitely while disconnected in both phases;
     * neither phase arms a timeout that stops advertising on its own. The hint
     * is advisory (a wrong phase is a power/latency matter, not a functional
     * one), hence the void return: a PAL that cannot apply it keeps its phase.
     */
    void (*request_fast_adv)(void);
};

/**
 * Provides the sid_pal_ble_adapter_interface interface handle to Network
 * Interface for calling the interface function.
 *
 * @param [in] handle - interface handle.
 *
 * @return SID_ERROR_NONE - in case method finished with success.
 *         In case of error, the error type depend upon implementation.
 */
sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SID_PAL_BLE_ADAPTER_IFC_H */
