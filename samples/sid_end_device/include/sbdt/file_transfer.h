/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <sid_api.h>

/**
 * @brief Initilize Sidewalk Bulk Data Transfer module.
 * 
 * @param handle Sidewalk handle given by sid_init.
 */
void app_file_transfer_demo_init(struct sid_handle *handle);

/**
 * @brief Deinitilize Sidewalk Bulk Data Transfer module.
 * 
 * @param handle Sidewalk handle given by sid_init.
 */
void app_file_transfer_demo_deinit(struct sid_handle *handle);

#endif /* FILE_TRANSFER_H */
