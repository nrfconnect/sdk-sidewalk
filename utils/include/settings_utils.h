/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>
#include <stdint.h>

#if defined(DEPRECATED_DFU_FLAG_SETTINGS_KEY)
typedef enum { DFU_APPLICATION, SIDEWALK_APPLICATION } app_start_t;
app_start_t application_to_start(void);
#endif

/** @brief Read a link mask value form settings (non-volatile memory).
 *
 *  @param link_mask Pointer to a buffer where link mask should be loaded.
 *
 *  @return 0 in case of success, negative error code otherwise.
 */
int settings_utils_link_mask_get(uint32_t *link_mask);

/** @brief Write a link mask value to settings (non-volatile memory).
 *
 *  @param link_mask New value to be load to settings.
 *
 *  @return 0 in case of success, negative error code otherwise.
 */
int settings_utils_link_mask_set(uint32_t link_mask);
