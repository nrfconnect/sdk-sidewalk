/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_bulk_data_transfer_api.h>

struct data_received_args {
	struct sid_bulk_data_transfer_desc *desc;
	struct sid_bulk_data_transfer_buffer *buffer;
	void *context;
};

void app_file_transfer_demo_init(struct sid_handle *handle);

void app_file_transfer_demo_deinit(struct sid_handle *handle);
