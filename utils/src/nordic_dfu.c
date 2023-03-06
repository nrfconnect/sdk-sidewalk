/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "nordic_dfu.h"
#include <zephyr/bluetooth/bluetooth.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nordic_dfu, CONFIG_SIDEWALK_LOG_LEVEL);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		      0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86,
		      0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
};

int nordic_dfu_ble_start(void)
{
	int err = bt_enable(NULL);
	if (err && err != -EALREADY) {
		LOG_ERR("Bluetooth enable failed (err %d)", err);
		return err;
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Bluetooth advertising start failed (err %d)", err);
		return err;
	}

	LOG_INF("Advertising successfully started");

	return 0;
}
