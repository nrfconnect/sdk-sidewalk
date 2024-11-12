/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <bt_app_callbacks.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <stdbool.h>
#include <zephyr/kernel.h>

static uint32_t bt_enable_count = 0;

int app_bt_enable(bt_ready_cb_t cb)
{
	if (bt_enable_count == 0) {
		int ret = bt_enable(cb);
		if (ret == 0) {
			bt_enable_count++;
		}
		return ret;
	}

	bt_enable_count++;
	if (cb) {
		cb(0);
	}
	return 0;
}

int app_bt_disable()
{
	if (bt_enable_count <= 0) {
		bt_enable_count = 0;
		return -EALREADY;
	}
	if (bt_enable_count == 1) {
		bt_enable_count = 0;
		return bt_disable();
	} else {
		bt_enable_count--;
		return 0;
	}
}
