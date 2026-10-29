/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/sys/crc.h>

#include <sid_utils.h>

/* Required by the Sidewalk QA SBDT commands, but missing in the Bluetooth LE only Sidewalk libraries. */
uint32_t crc32_calculate(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc)
{
	return crc32_ieee_update((p_crc != NULL) ? *p_crc : 0, p_data, size);
}
