/*
 * Copyright 2022-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SID_SDK_CONFIG_H
#define SID_SDK_CONFIG_H

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup SIDEWALK_SDK_CONFIG Sidewalk SDK Config
 * @brief Sidewalk SDK configs for the Sidewalk network
 * @{
 * @ingroup  SIDEWALK_SDK_CONFIG
 */

#if defined(SID_SDK_USE_APP_CONFIG) && SID_SDK_USE_APP_CONFIG
#include <sid_sdk_app_config.h>
#endif   // defined(SID_SDK_USE_APP_CONFIG) && SID_SDK_USE_APP_CONFIG

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief: Enable/Disable link type 1 (ble)
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_1
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_1 1
#endif

/*
 * @brief: Enable/Disable link type 2 (fsk)
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 1
#endif

/*
 * @brief: Enable/Disable link type 3 (LoRa)
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_3
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_3 1
#endif

#ifndef SID_SDK_CONFIG_ENABLE_METRICS_PERSISTENCE
#define SID_SDK_CONFIG_ENABLE_METRICS_PERSISTENCE 0
#endif
/*
 * @brief: Enable/Disable link type 3 (LoRa) low latency feature
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_3_LOW_LATENCY
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_3_LOW_LATENCY 0
#endif

/*
 * @brief: Enable/Disable link type 3 (LoRa) transmit power control feature
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_3_TRANSMIT_POWER_CONTROL
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_3_TRANSMIT_POWER_CONTROL 0
#endif

/*
 * @brief: Enable/Disable link type 2 (FSK) transmit power control feature
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_2_TRANSMIT_POWER_CONTROL
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_2_TRANSMIT_POWER_CONTROL 0
#endif

/*
 * @brief: Enable/Disable LINK_TYPE_1 D2D
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_1_D2D
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_1_D2D SID_SDK_CONFIG_ENABLE_LINK_TYPE_1
#endif   // SID_SDK_CONFIG_ENABLE_LINK_TYPE_1_D2D

/*
 * @brief: Enable/Disable network coverage test
 */
#ifndef SID_SDK_CONFIG_NETWORK_COVERAGE_TEST
#define SID_SDK_CONFIG_NETWORK_COVERAGE_TEST 0
#endif

#ifndef SID_SDK_CONFIG_NUMBER_OF_COVERAGE_LINKS
#define SID_SDK_CONFIG_NUMBER_OF_COVERAGE_LINKS 1
#endif

/*
 * @brief: Enable/Disable BLE user advertising & connection parameter configuration
 */
#ifndef SID_SDK_CONFIG_ENABLE_BLE_USER_CONFIG
#define SID_SDK_CONFIG_ENABLE_BLE_USER_CONFIG 1
#endif

/*
 * @brief: Minimum link type 2 (FSK) registration periodicity, 12 hours
 */
#define SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_MIN_SEC (12 * 60 * 60)
/*
 * @brief: Maximum link type 2 (FSK) registration periodicity, (2^32 - 1) / 1000
 */
#define SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_MAX_SEC (4294967295 / 1000)

/*
 * @brief: Configuration flag to set periodicity of registration
 * attempts over link type 2 (FSK)
 */
#if defined(SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_SEC) && \
           ((SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_SEC < SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_MIN_SEC) || \
           (SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_SEC > SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_MAX_SEC))
#undef SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_SEC
#endif

/*
 * @brief: Default link type 2 (FSK) registration periodicity, 24 hours
 */
#ifndef SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_SEC
#define SID_SDK_CONFIG_LINK_TYPE_2_REGISTRATION_PERIODICITY_SEC (24 * 60 * 60)
#endif

/*
 * @brief: Enable registration over link type 2(FSK)
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_2_REGISTRATION
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_2_REGISTRATION SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
#endif

/*
 * @brief Disable link type 2 registration if link type 2 is disabled
 */
#if !SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
#undef SID_SDK_CONFIG_ENABLE_LINK_TYPE_2_REGISTRATION
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_2_REGISTRATION 0
#endif

/*
 * @brief Enable multi link
 */
#ifndef SID_SDK_CONFIG_ENABLE_MULTI_LINK
#define SID_SDK_CONFIG_ENABLE_MULTI_LINK 0
#endif

/*
 * @brief Enable auto connect
 */
#ifndef SID_SDK_CONFIG_ENABLE_AUTO_CONNECT
#define SID_SDK_CONFIG_ENABLE_AUTO_CONNECT 0
#endif

/*
 * @brief: Configuration to reserve memory for sidewalk application usage
 * The minimum bytes needed for sidewalk data-pipe usage is defined by SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE,
 * if the SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE is less than SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE_MIN
 * then it is overwritten to be SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE_MIN
 */
#define SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE_MIN 1500

#if defined(SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE) \
    && SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE < SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE_MIN
#undef SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE
#define SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE_MIN
#endif

#ifndef SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE
#define SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE 3000
#endif

/*
 * @brief: Configuration to reserve memory from SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE for sidewalk internal usage
 * The minimum bytes needed for sidewalk usage is defined by
 * SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE_MIN, if the
 * SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE is less than
 * SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE_MIN then it is overwritten to be
 * SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE_MIN
 */
#define SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE_MIN 150

#ifndef SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE
#define SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE (SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_SIZE / 10)
#endif

#if SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE \
    < SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE_MIN
#undef SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE
#define SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE \
    SID_SDK_CONFIG_APPLICATION_MEMORY_POOL_INTERNAL_RESERVE_SIZE_MIN
#endif

/*
 * @brief: Configuration to set max size of duplicate detection table
 * The duplicate detection stores the unique message IDs of the last configured size of
 * the table. If an incoming message's ID matches any of the entries of the table, the
 * message is detected as a duplicate
 */
#define SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE_MIN 5

#if defined(SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE) \
    && SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE < SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE_MIN
#undef SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE
#define SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE_MIN
#endif

#ifndef SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE
#define SID_SDK_CONFIG_DUPLICATE_DETECTION_SIZE 10
#endif

/*
 * @brief: BLE configuration flag to control how long the connection remains
 * active when there is no sidewalk tx/rx activity. Max inactivity timeout is 5
 * minutes.
 */
#if defined(SID_SDK_CONFIG_BLE_INACTIVITY_TIMEOUT_SEC) && (SID_SDK_CONFIG_BLE_INACTIVITY_TIMEOUT_SEC > 300)
#undef SID_SDK_CONFIG_BLE_INACTIVITY_TIMEOUT_SEC
#endif

#ifndef SID_SDK_CONFIG_BLE_INACTIVITY_TIMEOUT_SEC
#define SID_SDK_CONFIG_BLE_INACTIVITY_TIMEOUT_SEC 30
#endif

/**
 * Configure transmission of Sub-Ghz EndPoint link metrics to the Sidewalk cloud services
 */
#ifndef SID_SDK_CONFIG_ENABLE_LINK_METRICS
#define SID_SDK_CONFIG_ENABLE_LINK_METRICS (SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_3)
#endif

/**
 * Defines the duty cycle which must be met in order to comply with Specific Absorption
 * Rate (SAR) requirements as set by the FCC for wearable devices.
 */
#ifndef SID_SDK_CONFIG_RADIO_SAR_DCR
#define SID_SDK_CONFIG_RADIO_SAR_DCR (100)
#endif

/*
 * @brief Enable Sidewalk Bulk Data Transfer.
 */
#ifndef SID_SDK_CONFIG_ENABLE_BULK_DATA_TRANSFER
#define SID_SDK_CONFIG_ENABLE_BULK_DATA_TRANSFER 0
#endif

/**
 * Defines the max allowable output power in dBm allowed for each of the data rates
 * supported. The data rate mappings are as follows:
 *    ___________________________
 *   |Data Rates | Index in Table|
 *   |---------------------------|
 *   | 2 KBPS    |      0        |
 *   | 22 KBPS   |      1        |
 *   | 50 KBPS   |      2        |
 *   | 150 KBPS  |      3        |
 *   | 250 KBPS  |      4        |
 *   | 12.5 KBPS |      5        |
 *    ---------------------------
 */
#ifndef SID_SDK_CONFIG_RF_POWER_MAX_TABLE
#define SID_SDK_CONFIG_RF_POWER_MAX_TABLE \
    {                                     \
        20, 20, 20, 20, 20, 20            \
    }
#endif

/**
 * Defines the output power in dBm allowed for each of the data rates
 * supported. The data rate mappings are as follows:
 *    ___________________________
 *   |Data Rates | Index in Table|
 *   |---------------------------|
 *   | 2 KBPS    |      0        |
 *   | 22 KBPS   |      1        |
 *   | 50 KBPS   |      2        |
 *   | 150 KBPS  |      3        |
 *   | 250 KBPS  |      4        |
 *   | 12.5 KBPS |      5        |
 *    ---------------------------
 */

#ifndef SID_SDK_CONFIG_RF_POWER_TABLE
#define SID_SDK_CONFIG_RF_POWER_TABLE \
    {                                 \
        20, 20, 20, 20, 20, 20        \
    }
#endif

/*
 * @brief: Enable/Disable setup diagnostics
 */
#ifndef SID_SDK_CONFIG_ENABLE_SETUP_DIAGNOSTICS
#define SID_SDK_CONFIG_ENABLE_SETUP_DIAGNOSTICS 0
#endif

/*
 * @brief: Enable/Disable diagnostic logging
 */
#ifndef SID_SDK_CONFIG_ENABLE_DIAG_LOGGING
#define SID_SDK_CONFIG_ENABLE_DIAG_LOGGING 0
#endif

/*
 * @brief: Minimum size that should be allocated by the application for the diagnostic logging buffer.
 */
#ifndef SID_SDK_CONFIG_DIAG_LOG_BUFFER_MIN_SIZE
#define SID_SDK_CONFIG_DIAG_LOG_BUFFER_MIN_SIZE 1024
#endif

/*
 * @brief: Enable/Disable the Sidewalk Location library.
 */
#ifndef SID_SDK_CONFIG_ENABLE_LOCATION
#define SID_SDK_CONFIG_ENABLE_LOCATION 0
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

/// @endcond

#endif /* SID_SDK_CONFIG_H */
