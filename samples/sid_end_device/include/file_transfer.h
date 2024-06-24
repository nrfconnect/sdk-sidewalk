/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <sid_bulk_data_transfer_api.h>

void app_file_transfer_demo_init(struct sid_handle *handle);

void app_file_transfer_demo_deinit(struct sid_handle *handle);

#endif /* FILE_TRANSFER_H */
