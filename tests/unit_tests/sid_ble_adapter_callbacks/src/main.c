/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>

#include <sid_ble_adapter_callbacks.h>

#include <zephyr/bluetooth/conn.h>
#include <stdbool.h>

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

void setUp(void)
{
	ble_indication_callback_call_cnt = 0;
	ble_notify_callback_call_cnt = 0;
	ble_write_data_callback_call_cnt = 0;

	memset(&ble_connection_callback_test, 0x00, sizeof(ble_connection_callback_test));
}

/**
 * Callbacks
 */
static void ble_write_data_callback(sid_ble_cfg_service_identifier_t id, uint8_t *data,
				    uint16_t length)
{
	++ble_write_data_callback_call_cnt;
}

static void ble_notify_callback(sid_ble_cfg_service_identifier_t id, bool state)
{
	++ble_notify_callback_call_cnt;
}

static void ble_mtu_callback(uint16_t mtu_size)
{
	++ble_mtu_callback_call_cnt;
}

static void ble_adv_start_callback(void)
{
	++ble_adv_start_callback_call_cnt;
}

static void ble_connection_callback(bool state, uint8_t *addr)
{
	++ble_connection_callback_test.call_cnt;
	ble_connection_callback_test.state = state;
}

static void ble_indication_callback(bool status)
{
	++ble_indication_callback_call_cnt;
}

/**
 * Test functions
 */
void test_sid_ble_adapter_notification_sent_wo_callback(void)
{
	sid_ble_adapter_notification_sent();
	TEST_ASSERT_EQUAL(0, ble_indication_callback_call_cnt);
}

void test_sid_ble_adapter_notification_cb_set(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_ble_adapter_notification_cb_set(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_ble_adapter_notification_cb_set(ble_indication_callback));
}

void test_sid_ble_notification_sent_pass(void)
{
	sid_ble_adapter_notification_sent();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_ble_adapter_notification_cb_set(ble_indication_callback));
	TEST_ASSERT_EQUAL(1, ble_indication_callback_call_cnt);
}

void test_sid_ble_adapter_data_write_wo_callback(void)
{
	uint8_t data[TEST_DATA_CHUNK];

	sid_ble_adapter_data_write(0, data, sizeof(data));
	TEST_ASSERT_EQUAL(0, ble_write_data_callback_call_cnt);
}

void test_sid_ble_adapter_data_cb_set(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_ble_adapter_data_cb_set(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_ble_adapter_data_cb_set(ble_write_data_callback));
}

void test_sid_ble_adapter_data_write_pass(void)
{
	uint8_t data[TEST_DATA_CHUNK];

	sid_ble_adapter_data_write(0, data, sizeof(data));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_ble_adapter_data_cb_set(ble_write_data_callback));
	TEST_ASSERT_EQUAL(1, ble_write_data_callback_call_cnt);
}

void test_sid_ble_adapter_notification_changed_wo_callback(void)
{
	sid_ble_adapter_notification_changed(0, true);
	TEST_ASSERT_EQUAL(0, ble_notify_callback_call_cnt);
}

void test_sid_ble_adapter_notification_changed_cb_set(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS,
			  sid_ble_adapter_notification_changed_cb_set(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_ble_adapter_notification_changed_cb_set(ble_notify_callback));
}

void test_sid_ble_adapter_notification_changed_pass(void)
{
	sid_ble_adapter_notification_changed(0, true);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE,
			  sid_ble_adapter_notification_changed_cb_set(ble_notify_callback));
	TEST_ASSERT_EQUAL(1, ble_notify_callback_call_cnt);
}

void test_sid_ble_adapter_connection_changed_wo_callback(void)
{
	uint8_t ble_addr[BT_ADDR_SIZE] = { 0 };

	sid_ble_adapter_conn_connected(ble_addr);
	TEST_ASSERT_EQUAL(0, ble_connection_callback_test.call_cnt);

	sid_ble_adapter_conn_disconnected(ble_addr);
	TEST_ASSERT_EQUAL(0, ble_connection_callback_test.call_cnt);
}

void test_sid_ble_adapter_conn_cb_set(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_ble_adapter_conn_cb_set(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_ble_adapter_conn_cb_set(ble_connection_callback));
}

void test_sid_ble_connection_changed_pass(void)
{
	uint8_t ble_addr[BT_ADDR_SIZE] = { 0 };

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_ble_adapter_conn_cb_set(ble_connection_callback));

	sid_ble_adapter_conn_connected(ble_addr);
	TEST_ASSERT_EQUAL(1, ble_connection_callback_test.call_cnt);
	TEST_ASSERT_TRUE(ble_connection_callback_test.state);

	sid_ble_adapter_conn_disconnected(ble_addr);
	TEST_ASSERT_EQUAL(2, ble_connection_callback_test.call_cnt);
	TEST_ASSERT_FALSE(ble_connection_callback_test.state);
}

void test_sid_ble_adapter_mtu_changed_wo_callback(void)
{
	sid_ble_adapter_mtu_changed(0);
	TEST_ASSERT_EQUAL(0, ble_mtu_callback_call_cnt);
}

void test_sid_ble_adapter_mtu_changed_cb_set(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_ble_adapter_mtu_cb_set(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_ble_adapter_mtu_cb_set(ble_mtu_callback));
}

void test_sid_ble_adapter_mtu_changed_pass(void)
{
	sid_ble_adapter_mtu_changed(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_ble_adapter_mtu_cb_set(ble_mtu_callback));
	TEST_ASSERT_EQUAL(1, ble_mtu_callback_call_cnt);
}

void test_sid_ble_adapter_adv_started_wo_callback(void)
{
	sid_ble_adapter_adv_started();
	TEST_ASSERT_EQUAL(0, ble_adv_start_callback_call_cnt);
}

void test_sid_ble_adapter_adv_start_changed_cb_set(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_ble_adapter_adv_start_cb_set(NULL));
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_ble_adapter_adv_start_cb_set(ble_adv_start_callback));
}

void test_sid_ble_adapter_adv_started_pass(void)
{
	sid_ble_adapter_adv_started();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_ble_adapter_adv_start_cb_set(ble_adv_start_callback));
	TEST_ASSERT_EQUAL(1, ble_adv_start_callback_call_cnt);
}

extern int unity_main(void);

int main(void)
{
	return unity_main();
}
