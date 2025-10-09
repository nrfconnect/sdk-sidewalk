/***************************************************************************//**
 * @file
 * @brief app_subghz_config.h
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 * Your use of this software is governed by the terms of
 * Silicon Labs Master Software License Agreement (MSLA)available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software contains Third Party Software licensed by Silicon Labs from
 * Amazon.com Services LLC and its affiliates and is governed by the sections
 * of the MSLA applicable to Third Party Software and the additional terms set
 * forth in amazon_sidewalk_license.txt.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef APP_SUBGHZ_CONFIG_H
#define APP_SUBGHZ_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#ifdef LR11XX
#include "lr11xx_config.h"
#else
#include "sx126x_config.h"
#endif
#include "sid_pal_mfg_store_ifc.h"
#include <sid_900_cfg.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Function to get mfg configuration
 ******************************************************************************/
const sid_pal_mfg_store_region_t *get_mfg_cfg(void);

/***************************************************************************//**
 * Function to get subghz config
 ******************************************************************************/
const struct sid_sub_ghz_links_config *app_get_sub_ghz_config(void);

/***************************************************************************//**
 * Function to get radio configuration
 ******************************************************************************/
#ifdef LR11XX
const radio_lr11xx_device_config_t *get_radio_cfg(void);
#else
const radio_sx126x_device_config_t *get_radio_cfg(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* APP_SUBGHZ_CONFIG_H */
