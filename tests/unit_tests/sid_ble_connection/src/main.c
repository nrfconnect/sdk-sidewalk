/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <zephyr/fff.h>

#include <sid_ble_connection.h>

#include <cmock_sid_ble_adapter_callbacks.h>

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci_err.h>

#include <stdbool.h>
#include <errno.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(bt_conn_cb_register, struct bt_conn_cb *);
FAKE_VOID_FUNC(bt_gatt_cb_register, struct bt_gatt_cb *);
FAKE_VALUE_FUNC(struct bt_conn *, bt_conn_ref, struct bt_conn *);
FAKE_VOID_FUNC(bt_conn_unref, struct bt_conn *);
FAKE_VALUE_FUNC(const bt_addr_le_t *, bt_conn_get_dst, const struct bt_conn *);
FAKE_VALUE_FUNC(int, bt_conn_disconnect, struct bt_conn *, uint8_t);

#define FFF_FAKES_LIST(FAKE)	  \
	FAKE(bt_conn_cb_register) \
	FAKE(bt_gatt_cb_register) \
	FAKE(bt_conn_ref)	  \
	FAKE(bt_conn_unref)	  \
	FAKE(bt_conn_get_dst)	  \
	FAKE(bt_conn_disconnect)

#define CONNECTED (true)
#define DISCONNECTED (false)
#define ESUCCESS (0)

struct bt_conn {
	uint8_t dummy;
};

typedef struct {
	size_t num_calls;
	uint8_t *addr;
	bool state;
} connection_callback_test_t;

static connection_callback_test_t conn_cb_test;

void setUp(void)
{
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
	memset(&conn_cb_test, 0x00, sizeof(conn_cb_test));
	cmock_sid_ble_adapter_callbacks_Init();
}

void tearDown(void)
{
	cmock_sid_ble_adapter_callbacks_Verify();
}

static void connection_callback(const uint8_t *ble_addr, int cmock_num_calls)
{
	conn_cb_test.num_calls++;
	conn_cb_test.addr = (uint8_t *)ble_addr;
}

void test_sid_ble_conn_init(void)
{
	sid_ble_conn_init();
	TEST_ASSERT_EQUAL(1, bt_conn_cb_register_fake.call_count);
	TEST_ASSERT_EQUAL(1, bt_gatt_cb_register_fake.call_count);

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL(p_test_conn_cb);
	TEST_ASSERT_NOT_NULL(p_test_conn_cb->connected);
	TEST_ASSERT_NOT_NULL(p_test_conn_cb->disconnected);
	struct bt_gatt_cb *p_test_gatt_cb = bt_gatt_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL(p_test_gatt_cb);
	TEST_ASSERT_NOT_NULL(p_test_gatt_cb->att_mtu_updated);
}

void test_sid_ble_conn_params_get(void)
{
	const sid_ble_conn_params_t *params = NULL;

	sid_ble_conn_init();
	sid_ble_conn_deinit();
	sid_ble_conn_deinit();

	sid_ble_conn_init();
	params = sid_ble_conn_params_get();
	TEST_ASSERT_NOT_NULL(params);

	sid_ble_conn_deinit();
	params = sid_ble_conn_params_get();
	TEST_ASSERT_NULL(params);
}

void test_sid_ble_conn_positive(void)
{
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const sid_ble_conn_params_t *params = NULL;
	struct bt_conn test_conn = {
		.dummy = 0xDC
	};
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_deinit();
	sid_ble_conn_init();

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");

	__cmock_sid_ble_adapter_conn_connected_ExpectWithArray(test_addr.a.val, BT_ADDR_SIZE);
	p_test_conn_cb->connected(&test_conn, test_no_error);

	params = sid_ble_conn_params_get();
	TEST_ASSERT_EQUAL_PTR(&test_conn, params->conn);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectWithArray(test_addr.a.val, BT_ADDR_SIZE);
	p_test_conn_cb->disconnected(&test_conn, test_reason);
}

void test_sid_ble_set_conn_cb_positive(void)
{
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const sid_ble_conn_params_t *params = NULL;
	struct bt_conn test_conn = {
		.dummy = 0xDC
	};
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	p_test_conn_cb->connected(&test_conn, test_no_error);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);

	params = sid_ble_conn_params_get();
	TEST_ASSERT_EQUAL_PTR(&test_conn, params->conn);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	p_test_conn_cb->disconnected(&test_conn, test_reason);
	TEST_ASSERT_EQUAL(DISCONNECTED, conn_cb_test.state);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);
}

void test_sid_ble_conn_cb_set_call_count(void)
{
	size_t conn_cb_cnt_expected = 0;
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_error_timeout = BT_HCI_ERR_CONN_TIMEOUT;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const bt_addr_le_t test_addr;
	struct bt_conn test_conn = {
		.dummy = 0xDC
	};

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	p_test_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	p_test_conn_cb->disconnected(&test_conn, test_reason);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	p_test_conn_cb->connected(&test_conn, test_error_timeout);

	bt_conn_get_dst_fake.return_val = NULL;
	p_test_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

void test_sid_ble_disconnected_wrong_conn(void)
{
	size_t conn_cb_cnt_expected = 0;
	struct bt_conn test_wrong_conn;
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const bt_addr_le_t test_addr;
	struct bt_conn test_conn = {
		.dummy = 0xDC
	};

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	p_test_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	p_test_conn_cb->disconnected(&test_wrong_conn, test_no_error);
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	p_test_conn_cb->disconnected(&test_conn, test_reason);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

void test_sid_ble_cb_set_before_init(void)
{
	size_t conn_cb_cnt_expected = 0;
	struct bt_conn test_conn = {
		.dummy = 0xDC
	};

	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL_MESSAGE(p_test_conn_cb, "Can't get bluetooth connection callback");

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	p_test_conn_cb->connected(&test_conn, 0);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	p_test_conn_cb->disconnected(&test_conn, 19);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

void test_sid_ble_conn_mtu_callback(void)
{
	struct bt_conn test_conn = {
		.dummy = 0xDC
	};

	sid_ble_conn_init();
	TEST_ASSERT_EQUAL(1, bt_gatt_cb_register_fake.call_count);

	struct bt_gatt_cb *p_test_gatt_cb = bt_gatt_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL(p_test_gatt_cb);
	TEST_ASSERT_NOT_NULL(p_test_gatt_cb->att_mtu_updated);

	uint16_t tx_mtu = 32, rx_mtu = 44;
	__cmock_sid_ble_adapter_mtu_changed_Expect(tx_mtu);
	p_test_gatt_cb->att_mtu_updated(&test_conn, tx_mtu, rx_mtu);
}

void test_sid_ble_conn_mtu_callback_curent_connection(void)
{
	struct bt_conn curr_conn = { 0 };
	struct bt_conn unknow_conn = { 0 };

	sid_ble_conn_init();
	bt_conn_ref_fake.return_val = &curr_conn;

	struct bt_conn_cb *p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	struct bt_gatt_cb *p_test_gatt_cb = bt_gatt_cb_register_fake.arg0_history[0];

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	p_test_conn_cb->connected(&curr_conn, 0);

	uint16_t tx_mtu = 32, rx_mtu = 44;
	__cmock_sid_ble_adapter_mtu_changed_Expect(tx_mtu);
	p_test_gatt_cb->att_mtu_updated(&curr_conn, tx_mtu, rx_mtu);

	p_test_gatt_cb->att_mtu_updated(&unknow_conn, tx_mtu, rx_mtu);
}

void test_sid_ble_conn_disconnect(void)
{
	bt_conn_disconnect_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_conn_disconnect());

	bt_conn_disconnect_fake.return_val = -ENOTCONN;
	TEST_ASSERT_NOT_EQUAL(ESUCCESS, sid_ble_conn_disconnect());
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
