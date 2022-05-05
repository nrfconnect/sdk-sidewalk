/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <fff.h>

#include <sid_ble_connection.h>

#include <bluetooth/conn.h>
#include <bluetooth/hci_err.h>

#include <stdbool.h>

#define CONNECTED (true)
#define DISCONNECTED (false)

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(bt_conn_cb_register, struct bt_conn_cb *);
FAKE_VALUE_FUNC(struct bt_conn *, bt_conn_ref, struct bt_conn *);
FAKE_VOID_FUNC(bt_conn_unref, struct bt_conn *);
FAKE_VALUE_FUNC(const bt_addr_le_t *, bt_conn_get_dst, const struct bt_conn *);

struct bt_conn {
	uint8_t dummy;
};

typedef struct {
	size_t num_calls;
	uint8_t *addr;
	bool state;
} connection_callback_test_t;

static connection_callback_test_t conn_cb_test;

static void connection_callback(bool state, uint8_t *addr)
{
	conn_cb_test.num_calls++;
	conn_cb_test.state = state;
	conn_cb_test.addr = addr;
}

void setUp(void)
{
	RESET_FAKE(bt_conn_cb_register);
	RESET_FAKE(bt_conn_ref);
	RESET_FAKE(bt_conn_unref);
	RESET_FAKE(bt_conn_get_dst);
	FFF_RESET_HISTORY();
	memset(&conn_cb_test, 0x00, sizeof(conn_cb_test));
}

void test_sid_ble_conn_init(void)
{
	sid_ble_conn_init();
	TEST_ASSERT_EQUAL(1, bt_conn_cb_register_fake.call_count);

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL(p_test_conn_cb);
	TEST_ASSERT_NOT_NULL(p_test_conn_cb->connected);
	TEST_ASSERT_NOT_NULL(p_test_conn_cb->disconnected);
}

void test_sid_ble_conn_deinit(void)
{
	sid_ble_conn_init();
	sid_ble_conn_deinit();
	sid_ble_conn_init();
	sid_ble_conn_deinit();
	sid_ble_conn_deinit();
	TEST_PASS();
}

void test_sid_ble_conn_params_get(void)
{
	const sid_ble_conn_params_t *params = NULL;

	sid_ble_conn_init();
	params = sid_ble_conn_params_get();
	TEST_ASSERT_NOT_NULL(params);

	sid_ble_conn_deinit();
	params = sid_ble_conn_params_get();
	TEST_ASSERT_NULL(params);
}

void test_sid_ble_conn_positive(void)
{
	// Prepare paramters
	struct bt_conn *p_test_conn = NULL;
	uint8_t test_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const sid_ble_conn_params_t *params = NULL;
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	bt_conn_get_dst_fake.return_val = &test_addr;

	// Initialize connection
	sid_ble_conn_deinit();
	sid_ble_conn_init();

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");

	// Connect
	p_test_conn_cb->connected(p_test_conn, test_error);

	// Connection paramters
	params = sid_ble_conn_params_get();
	TEST_ASSERT_EQUAL_PTR(p_test_conn, params->conn);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	// Disconnect
	p_test_conn_cb->disconnected(p_test_conn, test_reason);
}

void test_sid_ble_conn_cb_set_positive(void)
{
	// Prepare paramters
	struct bt_conn *p_test_conn = NULL;
	uint8_t test_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const sid_ble_conn_params_t *params = NULL;
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	bt_conn_get_dst_fake.return_val = &test_addr;

	// Initialize connection
	sid_ble_conn_init();
	sid_ble_conn_cb_set(connection_callback);

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");

	// Connect
	p_test_conn_cb->connected(p_test_conn, test_error);
	TEST_ASSERT_EQUAL(CONNECTED, conn_cb_test.state);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);

	// Connection paramters
	params = sid_ble_conn_params_get();
	TEST_ASSERT_EQUAL_PTR(p_test_conn, params->conn);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	// Disconnect
	p_test_conn_cb->disconnected(p_test_conn, test_reason);
	TEST_ASSERT_EQUAL(DISCONNECTED, conn_cb_test.state);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);
}

void test_sid_ble_conn_cb_set_call_count(void)
{
	// Preparations
	size_t conn_cb_cnt_expected = 0;
	struct bt_conn *p_test_conn = NULL;
	uint8_t test_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const bt_addr_le_t test_addr;

	bt_conn_get_dst_fake.return_val = &test_addr;

	sid_ble_conn_init();
	sid_ble_conn_cb_set(connection_callback);

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");

	// Positive scenario
	p_test_conn_cb->connected(p_test_conn, test_error);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	p_test_conn_cb->disconnected(p_test_conn, test_reason);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	// Connet error
	test_error = BT_HCI_ERR_CONN_TIMEOUT;
	p_test_conn_cb->connected(p_test_conn, test_error);
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
	test_error = BT_HCI_ERR_SUCCESS;

	// Disconnect wrong connection
	struct bt_conn test_wrong_conn;
	p_test_conn_cb->connected(p_test_conn, test_error);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
	p_test_conn_cb->disconnected(&test_wrong_conn, test_error);
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
	p_test_conn_cb->disconnected(p_test_conn, test_reason);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	// Connection with malformed ble addr
	bt_conn_get_dst_fake.return_val = NULL;
	p_test_conn_cb->connected(p_test_conn, test_error);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

void test_sid_ble_cb_set_before_init(void)
{
	size_t conn_cb_cnt_expected = 0;
	struct bt_conn *p_test_conn = NULL;

	sid_ble_conn_deinit();
	sid_ble_conn_cb_set(connection_callback);
	sid_ble_conn_init();
	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");
	p_test_conn_cb->connected(p_test_conn, 0);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
	p_test_conn_cb->disconnected(p_test_conn, 19);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
