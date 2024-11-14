/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app_ble_config.h>
#include <sid_ble_uuid.h>

#include <zephyr/types.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/util.h>

#define MS_TO_INTERVAL_VAL(ms) (uint16_t)((ms) / 0.625f)

#if defined(CONFIG_BT_CTLR_TX_PWR_DBM)
#define TX_PWR_DBM (CONFIG_BT_CTLR_TX_PWR_DBM)
#else
#define TX_PWR_DBM (0)
#endif

static const sid_ble_cfg_service_t ble_service = {
    .type = AMA_SERVICE,
    .id = {
        .type = UUID_TYPE_16,
        .uu = { AMA_SERVICE_UUID_VAL & 0xFF, (AMA_SERVICE_UUID_VAL >> 8) & 0xFF },
    },
};

static const sid_ble_cfg_descriptor_t ble_desc[] = {
    {
        .id = {
            .type = UUID_TYPE_16,
            .uu = { 0x29, 0x02 },
        },
        .perm = {
            .is_write = true,
        },
    }
};

static const sid_ble_cfg_characteristics_t ble_characteristics[] = {
    {
        .id = {
            .type = UUID_TYPE_128,
            .uu = { AMA_CHARACTERISTIC_UUID_VAL_WRITE },
        },
        .perm = {
            .is_read = true,
            .is_write = true,
        },
    },
	{
		.id = {
            .type = UUID_TYPE_128,
            .uu = { AMA_CHARACTERISTIC_UUID_VAL_NOTIFY },
        },
        .perm = {
            .is_read = true,
            .is_write = true,
        },
    }
};

static const sid_ble_cfg_adv_param_t adv_param = {
	.type = AMA_SERVICE,
	.fast_enabled = true,
	.slow_enabled = true,
	.fast_interval = 256,
	.fast_timeout = 3000,
	.slow_interval = 1600,
	.slow_timeout = 0,
};
static const sid_ble_cfg_conn_param_t conn_param = {
	.min_conn_interval = MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_SLOW),
	.max_conn_interval = MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_FAST),
	.slave_latency = CONFIG_BT_PERIPHERAL_PREF_LATENCY,
	.conn_sup_timeout = CONFIG_BT_PERIPHERAL_PREF_TIMEOUT,
};
static const sid_ble_cfg_gatt_profile_t ble_profile[] = {
	{
		.service = ble_service,
		.char_count = ARRAY_SIZE(ble_characteristics),
		.characteristic = ble_characteristics,
		.desc_count = ARRAY_SIZE(ble_desc),
		.desc = ble_desc,
	},
};

static const sid_ble_config_t ble_cfg = {
	.name = CONFIG_SIDEWALK_BLE_NAME,
	.mtu = CONFIG_BT_L2CAP_TX_MTU,
	.is_adv_available = true,
	.mac_addr_type = SID_BLE_CFG_MAC_ADDRESS_TYPE_STATIC_RANDOM,
	.adv_param = adv_param,
	.is_conn_available = true,
	.conn_param = conn_param,
	.num_profile = ARRAY_SIZE(ble_profile),
	.profile = ble_profile,
	.max_tx_power_in_dbm = TX_PWR_DBM,
	.enable_link_metrics = true,
	.metrics_msg_retries = 3,
};

static const sid_ble_link_config_t ble_config = {
	.create_ble_adapter = sid_pal_ble_adapter_create,
	.config = &ble_cfg,
};

const sid_ble_link_config_t *app_get_ble_config(void)
{
	return &ble_config;
}
