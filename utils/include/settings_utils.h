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

#ifdef CONFIG_PERSISTENT_LINK_MASK_SETTINGS_KEY

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
#endif /* CONFIG_PERSISTENT_LINK_MASK_SETTINGS_KEY */

/**
 * @brief Get size of the element stored under #name in settings
 * 
 * @param name [IN] name to look for
 * @param len [OUT] readed length
 * @return int 0 on success negative errno on fail.
 */
int settings_utils_get_value_size(const char *name, size_t *len);

/**
 * @brief Get the element stored under #name in settings
 * 
 * @param name [IN] name to look for
 * @param dest [OUT] readed value 
 * @param len [IN] capacity of dest
 * @return int number of readed bytes or negative errno on fail.
 */
int settings_utils_load_immediate_value(const char *name, void *dest, size_t len);
