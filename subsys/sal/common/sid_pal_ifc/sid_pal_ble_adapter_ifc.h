/*
 * Copyright 2020-2025 Amazon.com, Inc. or its affiliates. All rights reserved.
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
