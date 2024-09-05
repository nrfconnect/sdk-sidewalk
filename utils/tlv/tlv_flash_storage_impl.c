/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/drivers/flash.h>
int tlv_storage_flash_write(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size)
{
	const struct device *flash_dev = (const struct device *)ctx;
	if (flash_dev == NULL) {
		return -ENODEV;
	}
	return flash_write(flash_dev, offset, data, data_size);
}

int tlv_storage_flash_read(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size)
{
	const struct device *flash_dev = (const struct device *)ctx;
	if (flash_dev == NULL) {
		return -ENODEV;
	}
	return flash_read(flash_dev, offset, data, data_size);
}

int tlv_storage_flash_erase(void *ctx, uint32_t offset, uint32_t size)
{
	const struct device *flash_dev = (const struct device *)ctx;
	if (flash_dev == NULL) {
		return -ENODEV;
	}
	return flash_erase(flash_dev, offset, size);
}
