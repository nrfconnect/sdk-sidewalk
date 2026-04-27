/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <zephyr/fff.h>

#include <sid_ble_connection.h>

#include <cmock_sid_ble_adapter_callbacks.h>

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <bt_app_callbacks.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, bt_conn_cb_register, struct bt_conn_cb *);
FAKE_VOID_FUNC(bt_gatt_cb_register, struct bt_gatt_cb *);
FAKE_VALUE_FUNC(struct bt_conn *, bt_conn_ref, struct bt_conn *);
FAKE_VOID_FUNC(bt_conn_unref, struct bt_conn *);
FAKE_VALUE_FUNC(const bt_addr_le_t *, bt_conn_get_dst, const struct bt_conn *);
FAKE_VALUE_FUNC(int, bt_conn_disconnect, struct bt_conn *, uint8_t);
FAKE_VALUE_FUNC(int, bt_conn_get_info, const struct bt_conn *, struct bt_conn_info *);
FAKE_VOID_FUNC(sid_ble_advert_notify_connection);
FAKE_VALUE_FUNC(int, bt_conn_le_param_update, struct bt_conn *, const struct bt_le_conn_param *);

#define FFF_FAKES_LIST(FAKE)                                                                       \
	FAKE(bt_conn_cb_register)                                                                  \
	FAKE(bt_gatt_cb_register)                                                                  \
	FAKE(bt_conn_ref)                                                                          \
	FAKE(bt_conn_unref)                                                                        \
	FAKE(bt_conn_get_dst)                                                                      \
	FAKE(bt_conn_disconnect)                                                                   \
	FAKE(bt_conn_get_info)                                                                     \
	FAKE(sid_ble_advert_notify_connection)                                                     \
	FAKE(bt_conn_le_param_update)

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
static struct bt_conn_cb *sid_bt_conn_cb;
static struct bt_gatt_cb *sid_bt_gatt_cb;

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

	sid_bt_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL(sid_bt_conn_cb);
	TEST_ASSERT_NOT_NULL(sid_bt_conn_cb->connected);
	TEST_ASSERT_NOT_NULL(sid_bt_conn_cb->disconnected);
	sid_bt_gatt_cb = bt_gatt_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL(sid_bt_gatt_cb);
	TEST_ASSERT_NOT_NULL(sid_bt_gatt_cb->att_mtu_updated);
}

void test_sid_ble_conn_data_get(void)
{
	const sid_ble_conn_data_t *params = NULL;

	sid_ble_conn_init();
	sid_ble_conn_deinit();
	sid_ble_conn_deinit();

	sid_ble_conn_init();
	params = sid_ble_conn_data_get();
	TEST_ASSERT_NOT_NULL(params);

	sid_ble_conn_deinit();
	params = sid_ble_conn_data_get();
	TEST_ASSERT_NULL(params);
}

int bt_conn_get_info_fake1(const struct bt_conn *a, struct bt_conn_info *b)
{
	b->id = BT_ID_SIDEWALK;
	return 0;
}

static uint16_t param_get_fake_interval = 24;
static uint16_t param_get_fake_latency = 1;
static uint16_t param_get_fake_timeout = 400;

int bt_conn_get_info_fake_param_get(const struct bt_conn *a, struct bt_conn_info *b)
{
	b->id = BT_ID_SIDEWALK;
	b->le.interval_us = (uint32_t)param_get_fake_interval * 1250U;
	b->le.latency = param_get_fake_latency;
	b->le.timeout = param_get_fake_timeout;
	return 0;
}

void test_sid_ble_conn_positive(void)
{
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const sid_ble_conn_data_t *params = NULL;
	struct bt_conn test_conn = { .dummy = 0xDC };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	sid_ble_conn_deinit();
	sid_ble_conn_init();

	__cmock_sid_ble_adapter_conn_connected_ExpectWithArray(test_addr.a.val, BT_ADDR_SIZE);
	sid_bt_conn_cb->connected(&test_conn, test_no_error);

	params = sid_ble_conn_data_get();
	TEST_ASSERT_EQUAL_PTR(&test_conn, params->conn);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectWithArray(test_addr.a.val, BT_ADDR_SIZE);
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
}

void test_sid_ble_set_conn_cb_positive(void)
{
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const sid_ble_conn_data_t *params = NULL;
	struct bt_conn test_conn = { .dummy = 0xDC };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);

	params = sid_ble_conn_data_get();
	TEST_ASSERT_EQUAL_PTR(&test_conn, params->conn);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
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
	struct bt_conn test_conn = { .dummy = 0xDC };

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, test_error_timeout);

	bt_conn_get_dst_fake.return_val = NULL;
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
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
	struct bt_conn test_conn = { .dummy = 0xDC };

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	sid_bt_conn_cb->disconnected(&test_wrong_conn, test_no_error);
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

void test_sid_ble_cb_set_before_init(void)
{
	size_t conn_cb_cnt_expected = 0;
	struct bt_conn test_conn = { .dummy = 0xDC };

	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, 0);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	sid_bt_conn_cb->disconnected(&test_conn, 19);
	conn_cb_cnt_expected++;
	TEST_ASSERT_EQUAL(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

void test_sid_ble_conn_mtu_callback(void)
{
	struct bt_conn test_conn = { .dummy = 0xDC };

	sid_ble_conn_init();

	uint16_t tx_mtu = 32, rx_mtu = 44;

	__cmock_sid_ble_adapter_mtu_changed_Expect(tx_mtu);
	sid_bt_gatt_cb->att_mtu_updated(&test_conn, tx_mtu, rx_mtu);
}

void test_sid_ble_conn_mtu_callback_curent_connection(void)
{
	struct bt_conn curr_conn = { 0 };
	struct bt_conn unknow_conn = { 0 };

	sid_ble_conn_init();
	bt_conn_ref_fake.return_val = &curr_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&curr_conn, 0);

	uint16_t tx_mtu = 32, rx_mtu = 44;

	__cmock_sid_ble_adapter_mtu_changed_Expect(tx_mtu);
	sid_bt_gatt_cb->att_mtu_updated(&curr_conn, tx_mtu, rx_mtu);

	sid_bt_gatt_cb->att_mtu_updated(&unknow_conn, tx_mtu, rx_mtu);
}

void test_sid_ble_conn_disconnect(void)
{
	struct bt_conn test_conn = { .dummy = 0xDC };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	/* Ensure no connection so first disconnect yields -ENOENT (clear from previous test) */
	sid_ble_conn_init();
	const sid_ble_conn_data_t *data = sid_ble_conn_data_get();
	if (data != NULL && data->conn != NULL) {
		/* disconnected callback uses ble_conn_is_valid() -> bt_conn_get_info(); must pass */
		int (*get_info_fakes[])(const struct bt_conn *, struct bt_conn_info *) = {
			bt_conn_get_info_fake1,
			bt_conn_get_info_fake1,
		};
		SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, get_info_fakes, 2);
		__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
		sid_bt_conn_cb->disconnected(data->conn, 0);
	}
	/* Deinit so conn_data_ptr is NULL; conn_data.conn cleared by disconnected above */
	sid_ble_conn_deinit();
	sid_ble_conn_init();

	/* No connection: expect -ENOENT */
	TEST_ASSERT_EQUAL(-ENOENT, sid_ble_conn_disconnect());

	/* Establish connection then test disconnect return values */
	sid_ble_conn_init();
	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);

	bt_conn_disconnect_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_conn_disconnect());

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);
	bt_conn_disconnect_fake.return_val = -ENOTCONN;
	TEST_ASSERT_EQUAL(-ENOTCONN, sid_ble_conn_disconnect());
}

void test_sid_ble_conn_param_get(void)
{
	struct bt_le_conn_param param_out = { 0 };

	TEST_ASSERT_EQUAL(-EINVAL, sid_ble_conn_param_get(NULL));

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_conn_param_get(&param_out));
	/* With no connection, returns static defaults (conn_params_prev) */

	struct bt_conn test_conn = { .dummy = 0xDC };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } },
	};
	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake_param_get };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);

	memset(&param_out, 0, sizeof(param_out));
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_conn_param_get(&param_out));
	TEST_ASSERT_EQUAL_UINT16(param_get_fake_interval, param_out.interval_min);
	TEST_ASSERT_EQUAL_UINT16(param_get_fake_interval, param_out.interval_max);
	TEST_ASSERT_EQUAL_UINT16(param_get_fake_latency, param_out.latency);
	TEST_ASSERT_EQUAL_UINT16(param_get_fake_timeout, param_out.timeout);
}

void test_sid_ble_conn_param_update(void)
{
	const struct bt_le_conn_param param_in = {
		.interval_min = 18,
		.interval_max = 24,
		.latency = 0,
		.timeout = 500,
	};

	TEST_ASSERT_EQUAL(-EINVAL, sid_ble_conn_param_update(NULL));

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_conn_param_update(&param_in));

	struct bt_conn test_conn = { .dummy = 0xDD };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f } },
	};
	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);

	/* Connect callback also calls bt_conn_le_param_update; count only our calls from here */
	bt_conn_le_param_update_fake.call_count = 0;
	bt_conn_le_param_update_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_conn_param_update(&param_in));
	TEST_ASSERT_EQUAL(1, bt_conn_le_param_update_fake.call_count);
	TEST_ASSERT_EQUAL_PTR(&test_conn, bt_conn_le_param_update_fake.arg0_val);
	TEST_ASSERT_EQUAL_UINT16(param_in.interval_min,
				 bt_conn_le_param_update_fake.arg1_val->interval_min);
	TEST_ASSERT_EQUAL_UINT16(param_in.interval_max,
				 bt_conn_le_param_update_fake.arg1_val->interval_max);

	/* Implementation always returns 0; it only logs when bt_conn_le_param_update fails */
	bt_conn_le_param_update_fake.return_val = -EINVAL;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_conn_param_update(&param_in));
	TEST_ASSERT_EQUAL(2, bt_conn_le_param_update_fake.call_count);
}

extern int unity_main(void);

int main(void)
{
	return unity_main();
}
