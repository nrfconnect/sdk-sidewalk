/*
 * Copyright 2022-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PAL_COMMON_IFC_H
#define SID_PAL_COMMON_IFC_H

#include <sid_pal_platform_init_types.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_error.h>

/** @file
 *
 * @defgroup sid_pal_common_ifc SID common interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details Provides common and generic platform interface code not specific to one PAL module.
 */
#include <sid_error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  sid_pal_mfg_store_region_t mfg_store_region;
  platform_specific_init_parameters_t platform_init_parameters;

} platform_parameters_t;

/**
 * Implements a platform generic initialization function.
 *
 * This function is only implemented on platforms that require additional vendor
 * specific initialization routines. It provides a generic entry point to the
 * platform implementation of the sid_pal components and is intended to be called
 * at start of day.
 *
 * @param[in] platform_init_parameters pointer to platform specific parameters.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_common_init(const platform_specific_init_parameters_t *platform_init_parameters);


/**
 * Implements a platform generic deinitialization function.
 *
 * This function is only implemented on platforms that require additional vendor
 * specific deinitialization routines. It provides a generic entry point to the
 * platform implementation of the sid_pal components.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_common_deinit(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
