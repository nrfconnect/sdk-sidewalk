/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_BLE_CONFIG_IFC_H
#define SID_BLE_CONFIG_IFC_H

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup SIDEWALK_API Sidewalk API
 * @brief API for communicating with the Sidewalk network
 * @{
 * @ingroup  SIDEWALK_API
 */

#include <stdint.h>
#include <stdbool.h>

#define BLE_UUID_MAX_LEN 16
#define BLE_ADDR_MAX_LEN 6

typedef enum sid_ble_cfg_uuid_type {
    UUID_TYPE_16 = 1, /**< 16 bit UUID */
    UUID_TYPE_32 = 2, /**< 32 bit UUID */
    UUID_TYPE_128 = 3 /**< 128 bit UUID */
} sid_ble_cfg_uuid_type_t;

typedef enum sid_ble_cfg_service_identifier {
    AMA_SERVICE,
    VENDOR_SERVICE,
    LOGGING_SERVICE
} sid_ble_cfg_service_identifier_t;

typedef struct sid_ble_cfg_uuid_info {
    sid_ble_cfg_uuid_type_t type;
    uint8_t uu[BLE_UUID_MAX_LEN];
} sid_ble_cfg_uuid_info_t;

typedef struct sid_ble_cfg_prop {
    bool is_read;
    bool is_write;
    bool is_notify;
    bool is_write_no_resp;
} sid_ble_cfg_prop_t;

typedef struct sid_ble_cfg_permission {
    bool is_read;
    bool is_write;
    bool is_none;
} sid_ble_cfg_permission_t;

typedef struct sid_ble_cfg_service {
    sid_ble_cfg_service_identifier_t type;
    sid_ble_cfg_uuid_info_t id;
} sid_ble_cfg_service_t;

typedef struct sid_ble_cfg_characteristics {
    sid_ble_cfg_uuid_info_t id;
    sid_ble_cfg_prop_t properties;
    sid_ble_cfg_permission_t perm;
} sid_ble_cfg_characteristics_t;

typedef struct sid_ble_cfg_descriptor {
    sid_ble_cfg_uuid_info_t id;
    sid_ble_cfg_permission_t perm;
} sid_ble_cfg_descriptor_t;

typedef struct sid_ble_cfg_gatt_profile {
    sid_ble_cfg_service_t service;
    uint8_t char_count;
    const sid_ble_cfg_characteristics_t *characteristic;
    uint8_t desc_count;
    const sid_ble_cfg_descriptor_t *desc;
} sid_ble_cfg_gatt_profile_t;

typedef struct sid_ble_cfg_adv_param {
    sid_ble_cfg_service_identifier_t type;
    bool fast_enabled;
    bool slow_enabled;
    uint32_t fast_interval;         /* In units of 0.625 ms */
    uint32_t fast_timeout;          /* In units of 10 ms    */
    uint32_t slow_interval;         /* In units of 0.625 ms */
    uint32_t slow_timeout;          /* In units of 10 ms    */
} sid_ble_cfg_adv_param_t;

typedef struct sid_ble_cfg_conn_param {
    uint16_t min_conn_interval;     /* Minimum Connection Interval in 1.25 ms units */
    uint16_t max_conn_interval;     /* Maximum Connection Interval in 1.25 ms units */
    uint16_t slave_latency;         /* Slave Latency in number of connection events */
    uint16_t conn_sup_timeout;      /* Connection Supervision Timeout in 10 ms units*/
} sid_ble_cfg_conn_param_t;

enum sid_ble_user_config_type {
    SID_BLE_USER_CFG_ADV,
    SID_BLE_USER_CFG_CONN,
    SID_BLE_USER_CFG_ALL,
};

typedef struct sid_ble_user_config {
    sid_ble_cfg_adv_param_t adv_param;
    sid_ble_cfg_conn_param_t conn_param;
    bool is_set;                                 /* True: Set Param, False: Get Param */
    enum sid_ble_user_config_type cfg_type;
} sid_ble_user_config_t;

enum sid_ble_cfg_mac_address_type {
    /** Address obtained from IEEE registration authority, no privacy */
    SID_BLE_CFG_MAC_ADDRESS_TYPE_PUBLIC,
    /** Random generated address non resolvable by remote changes on every
     * advertisement or connection */
    SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE,
    /** Random generated address changes on each device boot up or stay fixed
     * for lifetime of device */
    SID_BLE_CFG_MAC_ADDRESS_TYPE_STATIC_RANDOM,
    /** Random generated address with IRK(Identity Resolving Key) resolvable by
     * remote. Used when bonding is enabled */
    SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_RESOLVABLE,
};

typedef struct sid_ble_config {
    const char *name;
    uint16_t mtu;
    bool is_adv_available;
    enum sid_ble_cfg_mac_address_type mac_addr_type;
    sid_ble_cfg_adv_param_t adv_param;
    bool is_conn_available;
    sid_ble_cfg_conn_param_t conn_param;
    uint8_t num_profile;
    const sid_ble_cfg_gatt_profile_t *profile;
    int8_t max_tx_power_in_dbm;
    /** Enable transmission Sidewalk stack metrics to Sidewalk cloud services using explicit commands */
    bool enable_link_metrics;
    /** Number of retries that Sidewalk stack metrics message can be retried by Sidewalk stack
     * configuring to 0 will disable the retries of the message
     */
    uint8_t metrics_msg_retries;
} sid_ble_config_t;

/** @} */

/// @endcond

#endif
