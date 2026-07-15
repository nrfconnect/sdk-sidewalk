/*
 * Copyright 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_DEVICE_CAPABILITIES_H
#define SID_DEVICE_CAPABILITIES_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sid_device_capabilities_config_tag {
    DEVICE_CAPABILITIES_CONFIG_VERSION = 0x01,
    DEVICE_CAPABILITIES_CONFIG_DEVICE_ROLE = 0x02,
    DEVICE_CAPABILITIES_CONFIG_LINKS_SUPPORTED = 0x03,
    DEVICE_CAPABILITIES_CONFIG_GATEWAY_MODES = 0x04,
    DEVICE_CAPABILITIES_CONFIG_OFFLINE_CONNECTIVITY_SUPPORT = 0x05,
    DEVICE_CAPABILITIES_CONFIG_NETWORK_TOPOLOGIES_SUPPORTED = 0x06,
};

enum sid_device_capabilities_config_gateway_modes {
    DEVICE_MODE_PRIMARY_ONLY = 0x00,
    DEVICE_MODE_SECONDARY_ONLY = 0x01,
    DEVICE_MODE_PRIMARY_SECONDARY = 0x02,
};

enum sid_device_capabilities_config_links_supported {
    LINK_BLE = 0x00,
    LINK_FSK = 0x01,
    LINK_LoRa = 0x02,
    LINK_OFDM = 0x03,
};

enum sid_device_capabilities_config_link_supported_mask {
    SID_LINK_MASK_BLE_ONLY = (1 << LINK_BLE),
    SID_LINK_MASK_FSK_ONLY = (1 << LINK_FSK),
    SID_LINK_MASK_LORA_ONLY = (1 << LINK_LoRa),
    SID_LINK_MAKSK_BLE_FSK = ((1 << LINK_BLE) | (1 << LINK_FSK)),
    SID_LINK_MASK_BLE_LORA = ((1 << LINK_BLE) | (1 << LINK_LoRa)),
    SID_LINK_MASK_FSK_LORA = ((1 << LINK_FSK) | (1 << LINK_LoRa)),
    SID_LINK_MASK_BLE_FSK_LORA = ((1 << LINK_BLE) | (1 << LINK_FSK) | (1 << LINK_LoRa)),
};

enum sid_device_capabilities_config_device_role {
    DEVICE_ROLE_GW_ONLY = 0x00,
    DEVICE_ROLE_EP = 0x01,
    DEVICE_ROLE_GW_EP = 0x02,
};

enum sid_device_capabilities_config_offline_connectivity_support {
    SOC_ROLE_NOT_SUPPORTED = 0x00,
    SOC_ROLE_ASSIST_ONLY = 0x01,
    SOC_ROLE_DISTRESS_ONLY = 0x02,
    SOC_ROLE_ASSIST_DISTRESS = 0x03,
};

enum sid_device_capabilities_config_network_topologies_supported {
    NETWORK_WAN_ONLY = 0x00,
    NETWORK_HAN_ONLY = 0x01,
    NETWORK_WAN_HAN = 0x02,
};

struct sid_device_capabilities_config_keys {
    uint8_t version;
    uint8_t device_role;
    uint8_t gateway_modes;
    uint8_t offline_connectivity_support;
    uint8_t network_topologies_supported;
    uint32_t link_supported_mask;
};

struct sid_device_capabilities_config {
    uint32_t sid_device_capability_config_keys_mask;
    struct sid_device_capabilities_config_keys conf_keys;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_DEVICE_CAPABILITIES_H */
