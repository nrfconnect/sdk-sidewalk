/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
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


#ifndef SID_PAL_MFG_STORE_IFC_H
#define SID_PAL_MFG_STORE_IFC_H

/** @file
 *
 * @defgroup sid_pal_mfg_store_ifc SID Manufacturing Store interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details  Provides manufacturing store interface to be implemented by platform
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The current version of the MFG storage. The version is stored during generating MFG.
 * It can be used to migrate between different versions of MFG when changing its data structure.
 */
#define SID_PAL_MFG_STORE_EMPTY_VERSION_NUMBER 0xFFFFFFFF
#define SID_PAL_MFG_STORE_FIXED_OFFSETS_VERSION 7 // Last version with fixed offsets
#define SID_PAL_MFG_STORE_TLV_VERSION 8

enum { SID_PAL_MFG_STORE_INVALID_OFFSET = UINT32_MAX };

/* Values available to all users of the manufacturing store.
 */
typedef enum {
    SID_PAL_MFG_STORE_DEVID = 1, // use sid_pal_mfg_store_dev_id_get
    /* Version is stored in network order */
    SID_PAL_MFG_STORE_VERSION = 2, // use sid_pal_mfg_store_get_version
    SID_PAL_MFG_STORE_SERIAL_NUM = 3, // use sid_pal_mfg_store_dev_id_get
    SID_PAL_MFG_STORE_SMSN = 4,
    SID_PAL_MFG_STORE_APP_PUB_ED25519 = 5,
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519 = 6,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519 = 7,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE = 8,
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1 = 9,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1 = 10,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE = 11,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519 = 12,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE = 13,
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL = 14,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1 = 15,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE = 16,
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL = 17,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519 = 18,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE = 19,
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL = 20,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1 = 21,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE = 22,
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL = 23,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519 = 24,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE = 25,
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL = 26,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1 = 27,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE = 28,
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL = 29,
    SID_PAL_MFG_STORE_SW_PUB_ED25519 = 30,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE = 31,
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL = 32,
    SID_PAL_MFG_STORE_SW_PUB_P256R1 = 33,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE = 34,
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL = 35,
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519 = 36,
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1 = 37,
    SID_PAL_MFG_STORE_APID = 38,

    /*
     * This arbitrary value is the number of value identifiers
     * reserved by Sidewalk. The range of these value identifiers is:
     * [0, SID_PAL_MFG_STORE_CORE_VALUE_MAX].
     * Applications may use identifiers outside of that range.
     */
    SID_PAL_MFG_STORE_CORE_VALUE_MAX = 4000,
    /* The value 0x6FFF is reserved for internal use */
    SID_PAL_MFG_STORE_VALUE_MAX = 0x6FFE,
} sid_pal_mfg_store_value_t;


/* Value sizes in bytes */
typedef enum {
    SID_PAL_MFG_STORE_DEVID_SIZE                        = 5,
    SID_PAL_MFG_STORE_VERSION_SIZE                      = 4,
    SID_PAL_MFG_STORE_SERIAL_NUM_SIZE                   = 17,
    SID_PAL_MFG_STORE_SMSN_SIZE                         = 32,
    SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_SIZE          = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIZE           = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_SIZE = 64,
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_SIZE           = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIZE            = 64,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_SIZE  = 64,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_SIZE    = 64,
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_SIZE           = 4,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIZE               = 64,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIZE          = 32,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_SIZE= 64,
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_SIZE       = 4,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIZE           = 64,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_SIZE = 64,
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_SIZE        = 4,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_SIZE    = 64,
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_SIZE           = 4,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIZE               = 64,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIZE               = 32,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIZE                = 64,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_SIZE      = 64,
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL_SIZE             = 4,
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519_SIZE             = 32,
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1_SIZE              = 64,
    SID_PAL_MFG_STORE_APID_SIZE                         = 4,
} sid_pal_mfg_store_value_size_t;


#define SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN 64

/* Basic I/O and setup */

typedef uint32_t (*sid_pal_mfg_store_app_value_to_offset_t)(int value);

/* Type which holds the start and end addresses of the manufacturing store */
typedef struct {
    uintptr_t addr_start;
    uintptr_t addr_end;
    /*
     * This function allows applications to extend the manufacturing store to be
     * be used for non-Sidewalk values. Applications should provide an
     * implementation of this function if they wish to extend the manufacturing
     * store for their own use. Its responsibility is to convert a value identifier
     * to an offset (in bytes) from the beginning of the manufacturing store.
     * Sidewalk owns the first SID_PAL_MFG_STORE_CORE_VALUE_MAX identifiers, So this
     * function's input should be greater than SID_PAL_MFG_STORE_CORE_VALUE_MAX and
     * its output should a valid offset below the mfg_store end address.
     *  If no mapping from the value identifier to an offset can be found, this
     *  function should return SID_PAL_MFG_STORE_INVALID_OFFSET value, which will
     *  cause the manufacturing store implementation to reject any operation on the
     *  provided value.
     */
    sid_pal_mfg_store_app_value_to_offset_t app_value_to_offset;
} sid_pal_mfg_store_region_t;

/** Prepare the manufacturing store for use. Must be called before
 *  any of the other sid_pal_mfg_store functions.
 *
 * @param[in]  mfg_store_region Structure containing start and end addresses
 *                              of the manufacturing store.
 */
void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region);

/** Deinitialize previously initialized mfg region.
 *
 */
void sid_pal_mfg_store_deinit(void);

/** Erase the manufacturing store.
 *  Because the manufacturing store is backed by flash memory, and flash memory
 *  can only be erased in large chunks (pages), this interface only supports
 *  erasing the entire manufacturing store.
 *
 *  @note This function is only supported for diagnostic builds.
 *
 *  @return  0 on success, negative value on failure.
 */
int32_t sid_pal_mfg_store_erase(void);

/** Check if the manufacturing store is empty.
 *
 *  @note This function is only supported for diagnostic builds.
 *
 * @retval  true if the entire manufacturing store is empty,
 *          such as just after an erase.
 */
bool sid_pal_mfg_store_is_empty(void);

/** Write to mfg store.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *  @param[in]  buffer Buffer containing the value to be stored.
 *  @param[in]  length Length of the value in bytes. Use values from
 *                     sid_pal_mfg_store_value_size_t here.
 *
 *  @retval  0 on success, negative value on failure.
 */
int32_t sid_pal_mfg_store_write(uint16_t value, const uint8_t *buffer, uint16_t length);


/** Read from mfg store.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *  @param[out] buffer Buffer to which the value will be copied.
 *  @param[in]  length Length of the value in bytes. Use values from
 *                     sid_pal_mfg_store_value_size_t here.
 *
 *
 */
void sid_pal_mfg_store_read(uint16_t value, uint8_t *buffer, uint16_t length);


/** Get length of a tag ID.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *
 *  @retval  Length of the value in bytes for the tag that is requested on success,
 *           0 on failure (not found)
 */
uint16_t sid_pal_mfg_store_get_length_for_value(uint16_t value);


/** Check if the manufacturing store supports TLV based storage.
 *
 *  @note This function only indicates that the platform supports TLV,
 *        but the device may have storage with fixed offsets that was
 *        flashed during production.
 *
 *  @retval  true if the manufacturing store supports TLV based storage
 */
bool sid_pal_mfg_store_is_tlv_support(void);


/* Functions specific to Sidewalk with special handling */

/** Get version of values stored in mfg store.
 *  The version of the mfg values is stored along with all the values
 *  in mfg store. This API retrieves the value by reading the
 *  address at which the version is stored.
 *
 *  @retval   version of mfg store.
 */
uint32_t sid_pal_mfg_store_get_version(void);


/** Get the device ID from the mfg store.
 *
 *  @param[out] dev_id The device ID
 *
 *  @retval true if the device ID could be found
 */
bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE]);


/** Get the device serial number from the mfg store.
 *
 *  @param[out] serial_num The device serial number
 *
 *  @retval true if the device serial number could be found
 */
bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE]);

/** Get the APID.
 *  Applicable only for products with short form certificate chain.
 *
 *  @param[out] apid The apid
 */
void sid_pal_mfg_store_apid_get(uint8_t apid[SID_PAL_MFG_STORE_APID_SIZE]);


/** Get the Application public key.
 *  Applicable only for products with short form certificate chain.
 *
 *  @param[out] app_pub The Application public key
 */
void sid_pal_mfg_store_app_pub_key_get(uint8_t app_pub[SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE]);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SID_PAL_MFG_STORE_IFC_H */
