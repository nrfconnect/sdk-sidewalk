/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <string.h>
#include <tlv/tlv_storage_impl.h>

int tlv_storage_ram_write(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size)
{
	uint8_t *ram_buffer = (uint8_t *)ctx;
	memcpy(ram_buffer + offset, data, data_size);
	return 0;
}
int tlv_storage_ram_read(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size)
{
	uint8_t *ram_buffer = (uint8_t *)ctx;
	memcpy(data, ram_buffer + offset, data_size);
	return 0;
}
