/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <bt_app_callbacks.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <sid_ble_uuid.h>
#include <zephyr/mgmt/mcumgr/transport/smp_bt.h>

static uint32_t bt_enable_count = 0;

int sid_ble_bt_enable(bt_ready_cb_t cb)
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

int sid_ble_bt_disable()
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

bool sid_ble_bt_attr_is_SMP(const struct bt_gatt_attr *attr)
{
	if (bt_uuid_cmp(attr->uuid, BT_UUID_DECLARE_128(SMP_BT_CHR_UUID_VAL)) == 0) {
		return true;
	}
	return false;
}

bool sid_ble_bt_attr_is_SIDEWALK(const struct bt_gatt_attr *attr)
{
	if (bt_uuid_cmp(attr->uuid, BT_UUID_DECLARE_128(AMA_CHARACTERISTIC_UUID_VAL_WRITE)) == 0) {
		return true;
	}
	if (bt_uuid_cmp(attr->uuid, BT_UUID_DECLARE_128(AMA_CHARACTERISTIC_UUID_VAL_NOTIFY)) == 0) {
		return true;
	}

	if (bt_uuid_cmp(attr->uuid,
			BT_UUID_DECLARE_128(VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE)) == 0) {
		return true;
	}
	if (bt_uuid_cmp(attr->uuid,
			BT_UUID_DECLARE_128(VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY)) == 0) {
		return true;
	}

	if (bt_uuid_cmp(attr->uuid,
			BT_UUID_DECLARE_128(LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE)) == 0) {
		return true;
	}
	if (bt_uuid_cmp(attr->uuid,
			BT_UUID_DECLARE_128(LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY)) == 0) {
		return true;
	}
	return false;
}
