/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>

#if CONFIG_SIDEWALK_TLV_RAM
int tlv_storage_ram_write(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
int tlv_storage_ram_read(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
#endif

#if CONFIG_SIDEWALK_TLV_FLASH
int tlv_storage_flash_write(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
int tlv_storage_flash_read(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
#endif
