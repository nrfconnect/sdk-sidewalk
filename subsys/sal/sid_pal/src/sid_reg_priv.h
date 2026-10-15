/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_reg_priv.h
 *  @brief Sidewalk regulatory framework functions provided by the prebuilt Sidewalk library.
 *
 *  The Sidewalk SDK does not ship a public header for these functions. Declarations match
 *  sid_protocol_regulatory.c from the Sidewalk 2.0 libraries.
 */

#ifndef SID_REG_PRIV_H
#define SID_REG_PRIV_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Regional information held by the regulatory framework. */
struct sid_reg_regional_info {
	uint8_t region; /**< Region code, same values as sid_pal_radio_region_code_t. */
	char iso_country[3]; /**< ISO 3166-1 alpha-2 country code, NUL terminated. */
};

/**
 * @brief Mark the regional information as valid.
 *
 * Sub-GHz MAC creation fails with SID_ERROR_UNINITIALIZED until the region is ready.
 *
 * @param ready true when the region was set, false to reset the regulatory state.
 */
void sid_reg_set_region_ready(bool ready);

/**
 * @brief Set the region and country used by the regulatory framework.
 *
 * @param region Region code, same values as sid_pal_radio_region_code_t.
 * @param iso_country ISO 3166-1 alpha-2 country code or NULL.
 *
 * @return 0 on success, non-zero when the region is not supported by the library.
 */
int sid_reg_set_regional_info(uint8_t region, const char *iso_country);

/**
 * @brief Get the regional information held by the regulatory framework.
 *
 * @return Pointer to the regional information.
 */
const struct sid_reg_regional_info *sid_reg_get_regional_info(void);

#ifdef __cplusplus
}
#endif

#endif /* SID_REG_PRIV_H */
