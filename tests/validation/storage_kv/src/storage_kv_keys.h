/*
 * Copyright 2018-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef STORAGE_KV_KEYS_H
#define STORAGE_KV_KEYS_H

#ifdef __cplusplus
extern "C" {
#endif

// Note per:
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v14.2.0%2Flib_fds_functionality.html&cp=4_0_0_3_50_1_4&anchor=lib_fds_functionality_keys
// Record keys need to be in the range 0x0001-0xBFFF

// FDS key IDs used for storing RingNet configuration records in an FDS file
typedef enum {
    STORAGE_KV_VERSION = 1,
    STORAGE_KV_PEERING_CHANNEL,                 // 2
    STORAGE_KV_PAN_ID,                          // 3
    STORAGE_KV_P2P_BROADCAST_KEY,               // 4
    STORAGE_KV_P2P_UNICAST_KEY,                 // 5
    STORAGE_KV_DBG_DEV_ID,                      // 6
    STORAGE_KV_UNCONFIRMED_MULTICAST_RETRIES,   // 7
    STORAGE_KV_GROUP_NUM,                       // 7
    STORAGE_KV_SEC_BLE_CONTEXT_KEY,             // 9  // GCMKEY(16B)+PeerNonce(8B)+Counter(6B)
    STORAGE_KV_SEC_CLOUD_CONTEXT_KEY,           // 10  // GCMKEY(16B)+PeerNonce(8B)+Counter(6B)
    STORAGE_KV_PAIRING_STATE_KEY,               // 11
    STORAGE_KV_KA_INTERVAL,                     // 12
    STORAGE_KV_REJOIN_PARAMS,                   // 13
    STORAGE_KV_DFU_PARAMS,                      // 14
    STORAGE_KV_CAD_PARAMS,                      // 15
    STORAGE_KV_RF_SECURITY_SETTINGS,            // 16
    STORAGE_KV_SLEEPY_DEVICE_PARAMS,            // 17
    STORAGE_KV_PROTOCOL_SWITCH,                 // 18
    STORAGE_KV_MAX_BCN_MISS,                    // 19
    STORAGE_KV_BCN_INTERVAL_FACTOR,             // 20
    STORAGE_KV_LDR_CHANNEL,                     // 21
    STORAGE_KV_INTERNAL_MAC_SWITCH,             // 22
    STORAGE_KV_DEV_VERSION,                     // 23
    STORAGE_KV_NG_DISCOVERY_PARAMS,             // 24
    STORAGE_KV_HOMEPAN_SCAN_INTVL,              // 25
    STORAGE_KV_PROTOCOL_VERSION,                // 26
    STORAGE_KV_SIDEWALK_NW_FLAG,                // 27
    STORAGE_KV_WAN_MASTER_KEY,                  // 28
    STORAGE_KV_WAN_KEY_PARAMS,                  // 29
    STORAGE_KV_APP_MASTER_KEY,                  // 30
    STORAGE_KV_APP_KEY_PARAMS,                  // 31
    STORAGE_KV_TSYNC_COUNTER_SEED,              // 32
    STORAGE_KV_TSYNC_COUNTER,                   // 33
    STORAGE_KV_TX_UUID_PARAMS,                  // 34
    STORAGE_KV_RF_POWER_LORA,                   // 35
    STORAGE_KV_RF_POWER_FSK,                    // 36
    STORAGE_KV_HDR_PRDCTY_INFO,                 // 37
    STORAGE_KV_HDR_OFFSET_INFO,                 // 38
    STORAGE_KV_CONFIGURATION_COMPLETE,          // 39
    STORAGE_KV_LAST_OPERATING_GW_MODE,          // 40
    STORAGE_KV_DISTRESS_GWD_PARAMS,             // 41
    STORAGE_KV_TMP_PRTCL_EXP,                   // 42   record for temporary protocol configuration needs
    STORAGE_KV_SIDEWALK_ID,                     // 43
    STORAGE_KV_HDR_WAKEUP_INFO,                 // 44
    STORAGE_KV_RF_POWER,                        // 45
    STORAGE_KV_DEVICE_CLASS_FSK,                // 46
    STORAGE_KV_DEVICE_CLASS_LDR,                // 47
    STORAGE_KV_D2D_MASTER_KEY,                  // 48
    STORAGE_KV_D2D_KEY_PARAMS,                  // 49
    STORAGE_KV_DIAG_PHY_MODEM,                  // 50
    STORAGE_KV_DIAG_PHY_FP,                     // 51
    STORAGE_KV_DIAG_PHY_MOD_FSK,                // 52
    STORAGE_KV_DIAG_PHY_MOD_LORA,               // 53
    STORAGE_KV_DIAG_PHY_PKT_FSK,                // 54
    STORAGE_KV_DIAG_PHY_PKT_LORA,               // 55
    STORAGE_KV_DIAG_MODULATION,                 // 56
    STORAGE_KV_LORA_LOW_LATENCY_PARAMS,         // 57
    STORAGE_KV_LAST_ENTRY,
    STORAGE_KV_EXTRY_MAX_VALUE = 0x6FFE
} storage_kv_key_t;

// These are separate, because these might be removed
typedef enum {
    STORAGE_KV_BULK_DATA_TRANSFER_NUMBER_OF_ONGOING_FILE_TRANSFERS = 1,

    STORAGE_KV_BULK_DATA_TRANSFER_FILE_ID = 20,
    STORAGE_KV_BULK_DATA_TRANSFER_BLOCK_SIZE,
    STORAGE_KV_BULK_DATA_TRANSFER_TOTAL_BLOCKS,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_BLOCK_SIZE,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_BLOCK_ID,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_BLOCK_SEQ_NUM,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_BLOCK_CHUNK_SIZE,
    STORAGE_KV_BULK_DATA_TRANSFER_FILE_DESCRIPTOR,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_ENTRY = 40,
    STORAGE_KV_BULK_DATA_TRANSFER_ENTRY_MAX = 0x6FFE
} storage_kv_bulk_data_transfer_key_t;

typedef enum {
    STORAGE_KV_BULK_DATA_TRANSFER_NUMBER_OF_ONGOING_FILE_TRANSFERS_SIZE = 4,
    STORAGE_KV_BULK_DATA_TRANSFER_FILE_ID_SIZE = 4,
    STORAGE_KV_BULK_DATA_TRANSFER_BLOCK_SIZE_SIZE = 2,
    STORAGE_KV_BULK_DATA_TRANSFER_TOTAL_BLOCKS_SIZE = 2,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_BLOCK_SIZE_SIZE = 2,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_BLOCK_ID_SIZE = 2,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_BLOCK_SEQ_NUM_SIZE = 4,
    STORAGE_KV_BULK_DATA_TRANSFER_LAST_BLOCK_CHUNK_SIZE_SIZE = 2,
} storage_kv_bulk_data_transfer_value_size_t;

#ifdef __cplusplus
}
#endif

#endif
