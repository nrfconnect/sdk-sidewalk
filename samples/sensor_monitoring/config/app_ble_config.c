/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <app_ble_config.h>

static const sid_ble_link_config_t ble_config = {
	.create_ble_adapter = sid_pal_ble_adapter_create,
};

const sid_ble_link_config_t *app_get_ble_config(void)
{
	return &ble_config;
}
