/*
 * Copyright 2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_ON_DEV_CERT_H
#define SID_ON_DEV_CERT_H

/**
 * @file sid_on_dev_cert.h
 *
 * @brief On-Device Certificate Generation Library
 *
 * @details This library is used for generating Sidewalk certificates on-device
 *          during manufacturing and writing the device manufacturing page.
 *
 * @note The user must ensure that the MFG storage and crypto library are initialized
 * before using this library. See functions sid_pal_mfg_store_init() and sid_pal_crypto_init().
 *
 * Library usage scenario:
 *
 *  1. Initialize the library by calling the sid_on_dev_cert_init() function.
 *
 *  2. Generate a Sidewalk Manufacturing Serial Number using the
 *     sid_on_dev_cert_generate_smsn() function.
 *
 *  3. Generate a Certificate Signing Request for each supported
 *     curves (ED25519 or P256r1).
 *
 *  4. Pass the CSRs to the Sidewalk Signing Tool and get back
 *     the signed Sidewalk Certificate Chains from the tool.
 *
 *  5. Write the received signed certificate chains to the device
 *     using the sid_on_dev_cert_write_cert_chain() function.
 *     Do this for both ED25519 and P256r1 crypto algorithms.
 *
 *  6. Write to the device application server ED25519 public key
 *     by sid_on_dev_cert_write_app_server_key() function.
 *
 *  7. Check the certificates and save all the data in the MFG storage.
 *     Use a sid_on_dev_cert_verify_and_store() function for this.
 *
 *  8. Deinitialize the library with the sid_on_dev_cert_deinit()
 *     function to free resources.
 */

#include <sid_error.h>
#include <stdlib.h>
#include <stdint.h>

#define SID_ODC_SMSN_SIZE 32
#define SID_ODC_CA_SERIAL_MIN_SIZE 4
#define SID_ODC_CA_SERIAL_MAX_SIZE (2 + 127)

#define SID_ODC_ED25519_PRK_SIZE 32
#define SID_ODC_ED25519_PUK_SIZE 32
#define SID_ODC_P256R1_PRK_SIZE 32
#define SID_ODC_P256R1_PUK_SIZE 64
#define SID_ODC_SIGNATURE_SIZE 64

// Certificate Signing Request size
#define SID_ODC_ED25519_CSR_SIZE                                                                   \
	(SID_ODC_SMSN_SIZE + SID_ODC_ED25519_PUK_SIZE + SID_ODC_SIGNATURE_SIZE)
#define SID_ODC_P256R1_CSR_SIZE                                                                    \
	(SID_ODC_SMSN_SIZE + SID_ODC_P256R1_PUK_SIZE + SID_ODC_SIGNATURE_SIZE)
#define SID_ODC_CSR_MAX_SIZE SID_ODC_P256R1_CSR_SIZE

// Sidewalk Certificate size
#define SID_ODC_ED25519_DEVICE_CERT_SIZE                                                           \
	(SID_ODC_SMSN_SIZE + SID_ODC_ED25519_PUK_SIZE + SID_ODC_SIGNATURE_SIZE)
#define SID_ODC_P256R1_DEVICE_CERT_SIZE                                                            \
	(SID_ODC_SMSN_SIZE + SID_ODC_P256R1_PUK_SIZE + SID_ODC_SIGNATURE_SIZE)

#define SID_ODC_ED25519_CA_MIN_SIZE                                                                \
	(SID_ODC_CA_SERIAL_MIN_SIZE + SID_ODC_ED25519_PUK_SIZE + SID_ODC_SIGNATURE_SIZE)
#define SID_ODC_P256R1_CA_MIN_SIZE                                                                 \
	(SID_ODC_CA_SERIAL_MIN_SIZE + SID_ODC_P256R1_PUK_SIZE + SID_ODC_SIGNATURE_SIZE)
#define SID_ODC_ED25519_CA_MAX_SIZE                                                                \
	(SID_ODC_CA_SERIAL_MAX_SIZE + SID_ODC_ED25519_PUK_SIZE + SID_ODC_SIGNATURE_SIZE)
#define SID_ODC_P256R1_CA_MAX_SIZE                                                                 \
	(SID_ODC_CA_SERIAL_MAX_SIZE + SID_ODC_P256R1_PUK_SIZE + SID_ODC_SIGNATURE_SIZE)

// Sidewalk Certificate Chain size
#define SID_ODC_ED25519_SCC_MIN_SIZE                                                               \
	(5 * SID_ODC_ED25519_CA_MIN_SIZE + SID_ODC_ED25519_DEVICE_CERT_SIZE)
#define SID_ODC_P256R1_SCC_MIN_SIZE                                                                \
	(5 * SID_ODC_P256R1_CA_MIN_SIZE + SID_ODC_P256R1_DEVICE_CERT_SIZE)
#define SID_ODC_ED25519_SCC_MAX_SIZE                                                               \
	(5 * SID_ODC_ED25519_CA_MAX_SIZE + SID_ODC_ED25519_DEVICE_CERT_SIZE)
#define SID_ODC_P256R1_SCC_MAX_SIZE                                                                \
	(5 * SID_ODC_P256R1_CA_MAX_SIZE + SID_ODC_P256R1_DEVICE_CERT_SIZE)
#define SID_ODC_SCC_MAX_SIZE SID_ODC_P256R1_SCC_MAX_SIZE

/**
 * @brief The structure contains input parameters that are used to generate
 *        Sidewalk Manufacturing Serial Number (SMSN)
 *
 * @see sid_on_dev_cert_create_smsn()
 */
struct sid_on_dev_cert_info {
	/** DMS Device type */
	const char *dev_type;
	/** Device serial number */
	const char *dsn;
	/** Advertised product ID */
	const char *apid;
	/** Board ID (optional) */
	const char *board_id;
};

/**
 * @brief Enum that defines a possible crypto algorithm used by the library.
 */
enum sid_on_dev_cert_algo_type {
	SID_ODC_CRYPT_ALGO_ED25519 = 1,
	SID_ODC_CRYPT_ALGO_P256R1,
};

/**
 * @brief The structure that defines Sidewalk Certificate Chain parameters
 */
struct sid_on_dev_cert_chain_params {
	/** Crypto algorithm */
	enum sid_on_dev_cert_algo_type algo;
	/** Sidewalk Certificate Chain in binary format */
	uint8_t *cert_chain;
	/** The size of the Sidewalk Certificate Chain */
	size_t cert_chain_size;
};

/**
 * @brief On-Device Certificate Generation Library initialization
 *
 * @details This function must be called before using the library to allocate
 *          the necessary internal resources.
 *
 * @return SID_ERROR_NONE on success, otherwise error code.
 */
sid_error_t sid_on_dev_cert_init(void);

/**
 * @brief On-Device Certificate Generation Library deinitialization
 *
 * @details This function must be called after using the library to free
 * the used internal resources.
 */
void sid_on_dev_cert_deinit(void);

/**
 * @brief Generate Sidewalk Manufacturing Serial Number (SMSN)
 *
 * @details The SMSN is calculated by calculating the hash over a number device-unique fields:
 *
 * SMSN = SHA256(DMS-DeviceType, [DSN|unique serial], Advertised-Product-ID, BoardID[optional])
 *
 * Call this function once before generating the Certificate signing request
 *
 * @param[in]     dev_info  Input data containing device-unique fields
 * @param[in,out] smsn      Buffer to save the generated SMSN. The caller must provide enough
 *                          buffer size for the output SMSN.
 *
 * @return SID_ERROR_NONE on success, otherwise error code.
 */
sid_error_t sid_on_dev_cert_generate_smsn(const struct sid_on_dev_cert_info *dev_info,
					  uint8_t *smsn);

/**
 * @brief Generate Certificate Signing Request (CSR)
 *
 * @details The function generates a device key pair for the supported curves and returns a signed
 *          Certificate Signing Request.
 *
 * The returned CSR has the following format:
 *
 *  CSR = Public Key || SMSN || Signature
 *
 * where Signature is the result of signing of [Public key || SMSN] by generated private key.
 *
 * @param[in]     algo      Crypto algorithm.
 * @param[in,out] csr       Buffer to save the generated CSR.
 * @param[in,out] csr_size  The size of the input buffer. The caller must provide enough
 *                          buffer size for the output CSR. As a result of the function call, the parameter
 *                          will contain the size of the output CSR.
 *
 * @return SID_ERROR_NONE on success, otherwise error code.
 */
sid_error_t sid_on_dev_cert_generate_csr(enum sid_on_dev_cert_algo_type algo, uint8_t *csr,
					 size_t *csr_size);

/**
 * @brief Write Sidewalk Certificate Chain
 *
 * @details The function writes a signed Sidewalk Certificate Chain to the device
 *          for appropriate curve (ED25519 or P256r1).
 *
 * @param[in] chain_params Sidewalk Certificate Chain to be written to the library.
 *
 * @return SID_ERROR_NONE on success, otherwise error code.
 */
sid_error_t
sid_on_dev_cert_write_cert_chain(const struct sid_on_dev_cert_chain_params *chain_params);

/**
 * @brief Write application server ED25519 public key to the device
 *
 * @see SID_PAL_MFG_STORE_APP_PUB_ED25519
 *
 * @param[in] app_key Application server ED25519 public key.
 *
 * @return SID_ERROR_NONE on success, otherwise error code.
 */
sid_error_t sid_on_dev_cert_write_app_server_key(const uint8_t *app_key);

/**
 * @brief Verify and store Sidewalk certificates
 *
 * @details The function verifies previously downloaded certificates and saves them in the MFG storage.
 *
 * Note: If verification is successful, the library erases the MFG storage before writing certificates.
 *
 * @return SID_ERROR_NONE on success, otherwise error code.
 */
sid_error_t sid_on_dev_cert_verify_and_store(void);

#endif // SID_ON_DEV_CERT_H
