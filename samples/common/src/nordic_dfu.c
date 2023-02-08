/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "nordic_dfu.h"
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/mgmt/mcumgr/smp_bt.h>
#include "os_mgmt/os_mgmt.h"
#include "img_mgmt/img_mgmt.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smp_bt_sample);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		      0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86,
		      0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
};

static void bt_ready(int bt_err_code)
{
	if (bt_err_code) {
		LOG_ERR("Bluetooth init failed (err %d)", bt_err_code);
		return;
	}

	LOG_INF("Bluetooth initialized");

	int err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return;
	}

	LOG_INF("Advertising successfully started");
}

int nordic_dfu_ble_start(void)
{
	/* Register the built-in mcumgr command handlers. */
	os_mgmt_register_group();
	img_mgmt_register_group();

	/* Enable Bluetooth. */
	int err = bt_enable(bt_ready);

	switch (err) {
	case -EALREADY:
		LOG_INF("Bluetooth already initialized");
		bt_ready(0);
	case 0:
		break;
	default:
		LOG_ERR("Bluetooth enable failed (err %d)", err);
		return err;
	}

	/* Initialize the Bluetooth mcumgr transport. */
	return smp_bt_register();
}
