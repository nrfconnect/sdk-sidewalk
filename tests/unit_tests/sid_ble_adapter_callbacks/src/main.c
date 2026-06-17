/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>

#include <sid_ble_adapter_callbacks.h>

#include <zephyr/bluetooth/conn.h>
#include <stdbool.h>
#include <string.h>

#define TEST_DATA_CHUNK (16)

typedef struct {
	int call_cnt;
	bool state;
} test_conn_t;

static int ble_indication_callback_call_cnt = 0;
static test_conn_t ble_connection_callback_test = { 0, false };
static int ble_notify_callback_call_cnt = 0;
static int ble_write_data_callback_call_cnt = 0;
static int ble_mtu_callback_call_cnt = 0;
static int ble_adv_start_callback_call_cnt = 0;

static void before_test(void *fixture)
{
	ARG_UNUSED(fixture);

	ble_indication_callback_call_cnt = 0;
	ble_notify_callback_call_cnt = 0;
	ble_write_data_callback_call_cnt = 0;
	ble_mtu_callback_call_cnt = 0;
	ble_adv_start_callback_call_cnt = 0;

	memset(&ble_connection_callback_test, 0x00, sizeof(ble_connection_callback_test));
}

ZTEST_SUITE(ble_adapter_callbacks, NULL, NULL, before_test, NULL, NULL);

static void ble_write_data_callback(sid_ble_cfg_service_identifier_t id, uint8_t *data,
				    uint16_t length)
{
	ARG_UNUSED(id);
	ARG_UNUSED(data);
	ARG_UNUSED(length);
	++ble_write_data_callback_call_cnt;
}

static void ble_notify_callback(sid_ble_cfg_service_identifier_t id, bool state)
{
	ARG_UNUSED(id);
	ARG_UNUSED(state);
	++ble_notify_callback_call_cnt;
}

static void ble_mtu_callback(uint16_t mtu_size)
{
	ARG_UNUSED(mtu_size);
	++ble_mtu_callback_call_cnt;
}

static void ble_adv_start_callback(void)
{
	++ble_adv_start_callback_call_cnt;
}

static void ble_connection_callback(bool state, uint8_t *addr)
{
	ARG_UNUSED(addr);
	++ble_connection_callback_test.call_cnt;
	ble_connection_callback_test.state = state;
}

static void ble_indication_callback(bool status)
{
	ARG_UNUSED(status);
	++ble_indication_callback_call_cnt;
}

/* Tests ordered with numeric prefixes: ztest runs alphabetically. */

ZTEST(ble_adapter_callbacks, test_01_notification_sent_wo_callback)
{
	sid_ble_adapter_notification_sent();
	zassert_equal(0, ble_indication_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_02_notification_cb_set)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_ble_adapter_notification_cb_set(NULL));
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_notification_cb_set(ble_indication_callback));
}

ZTEST(ble_adapter_callbacks, test_03_notification_sent_pass)
{
	sid_ble_adapter_notification_sent();
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_notification_cb_set(ble_indication_callback));
	zassert_equal(1, ble_indication_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_04_data_write_wo_callback)
{
	uint8_t data[TEST_DATA_CHUNK];

	sid_ble_adapter_data_write(0, data, sizeof(data));
	zassert_equal(0, ble_write_data_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_05_data_cb_set)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_ble_adapter_data_cb_set(NULL));
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_data_cb_set(ble_write_data_callback));
}

ZTEST(ble_adapter_callbacks, test_06_data_write_pass)
{
	uint8_t data[TEST_DATA_CHUNK];

	sid_ble_adapter_data_write(0, data, sizeof(data));
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_data_cb_set(ble_write_data_callback));
	zassert_equal(1, ble_write_data_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_07_notification_changed_wo_callback)
{
	sid_ble_adapter_notification_changed(0, true);
	zassert_equal(0, ble_notify_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_08_notification_changed_cb_set)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_ble_adapter_notification_changed_cb_set(NULL));
	zassert_equal(SID_ERROR_NONE,
		      sid_ble_adapter_notification_changed_cb_set(ble_notify_callback));
}

ZTEST(ble_adapter_callbacks, test_09_notification_changed_pass)
{
	sid_ble_adapter_notification_changed(0, true);
	zassert_equal(SID_ERROR_NONE,
		      sid_ble_adapter_notification_changed_cb_set(ble_notify_callback));
	zassert_equal(1, ble_notify_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_10_connection_changed_wo_callback)
{
	uint8_t ble_addr[BT_ADDR_SIZE] = { 0 };

	sid_ble_adapter_conn_connected(ble_addr);
	zassert_equal(0, ble_connection_callback_test.call_cnt);

	sid_ble_adapter_conn_disconnected(ble_addr);
	zassert_equal(0, ble_connection_callback_test.call_cnt);
}

ZTEST(ble_adapter_callbacks, test_11_conn_cb_set)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_ble_adapter_conn_cb_set(NULL));
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_conn_cb_set(ble_connection_callback));
}

ZTEST(ble_adapter_callbacks, test_12_connection_changed_pass)
{
	uint8_t ble_addr[BT_ADDR_SIZE] = { 0 };

	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_conn_cb_set(ble_connection_callback));

	sid_ble_adapter_conn_connected(ble_addr);
	zassert_equal(1, ble_connection_callback_test.call_cnt);
	zassert_true(ble_connection_callback_test.state);

	sid_ble_adapter_conn_disconnected(ble_addr);
	zassert_equal(2, ble_connection_callback_test.call_cnt);
	zassert_false(ble_connection_callback_test.state);
}

ZTEST(ble_adapter_callbacks, test_13_mtu_changed_wo_callback)
{
	sid_ble_adapter_mtu_changed(0);
	zassert_equal(0, ble_mtu_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_14_mtu_changed_cb_set)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_ble_adapter_mtu_cb_set(NULL));
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_mtu_cb_set(ble_mtu_callback));
}

ZTEST(ble_adapter_callbacks, test_15_mtu_changed_pass)
{
	sid_ble_adapter_mtu_changed(0);
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_mtu_cb_set(ble_mtu_callback));
	zassert_equal(1, ble_mtu_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_16_adv_started_wo_callback)
{
	sid_ble_adapter_adv_started();
	zassert_equal(0, ble_adv_start_callback_call_cnt);
}

ZTEST(ble_adapter_callbacks, test_17_adv_start_changed_cb_set)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_ble_adapter_adv_start_cb_set(NULL));
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_adv_start_cb_set(ble_adv_start_callback));
}

ZTEST(ble_adapter_callbacks, test_18_adv_started_pass)
{
	sid_ble_adapter_adv_started();
	zassert_equal(SID_ERROR_NONE, sid_ble_adapter_adv_start_cb_set(ble_adv_start_callback));
	zassert_equal(1, ble_adv_start_callback_call_cnt);
}
