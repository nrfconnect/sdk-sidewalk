/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <zephyr/fff.h>

#include <sid_ble_connection.h>

#include <mock/ble_callbacks_mock.h>

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
	FFF_FAKES_LIST_BLE_CALLBACKS(FAKE)                                                         \
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

static void before_test(void *fixture)
{
	ARG_UNUSED(fixture);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
	memset(&conn_cb_test, 0x00, sizeof(conn_cb_test));
}

ZTEST_SUITE(sid_ble_connection, NULL, NULL, before_test, NULL, NULL);

static void connection_callback_connected(const uint8_t *ble_addr)
{
	conn_cb_test.num_calls++;
	conn_cb_test.addr = (uint8_t *)ble_addr;
}

static void connection_callback_disconnected(const uint8_t *ble_addr)
{
	conn_cb_test.num_calls++;
	conn_cb_test.addr = (uint8_t *)ble_addr;
}

static int bt_conn_get_info_fake1(const struct bt_conn *a, struct bt_conn_info *b)
{
	b->id = BT_ID_SIDEWALK;
	return 0;
}

static uint16_t param_get_fake_interval = 24;
static uint16_t param_get_fake_latency = 1;
static uint16_t param_get_fake_timeout = 400;

static int bt_conn_get_info_fake_param_get(const struct bt_conn *a, struct bt_conn_info *b)
{
	b->id = BT_ID_SIDEWALK;
	b->le.interval_us = (uint32_t)param_get_fake_interval * 1250U;
	b->le.latency = param_get_fake_latency;
	b->le.timeout = param_get_fake_timeout;
	return 0;
}

static int bt_conn_get_info_fake_error(const struct bt_conn *a, struct bt_conn_info *b)
{
	ARG_UNUSED(a);
	ARG_UNUSED(b);
	return -ENOTCONN;
}

ZTEST(sid_ble_connection, test_01_sid_ble_conn_init)
{
	sid_ble_conn_init();
	zassert_equal(bt_conn_cb_register_fake.call_count, 1);
	zassert_equal(bt_gatt_cb_register_fake.call_count, 1);

	sid_bt_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	zassert_not_null(sid_bt_conn_cb);
	zassert_not_null(sid_bt_conn_cb->connected);
	zassert_not_null(sid_bt_conn_cb->disconnected);
	sid_bt_gatt_cb = bt_gatt_cb_register_fake.arg0_history[0];
	zassert_not_null(sid_bt_gatt_cb);
	zassert_not_null(sid_bt_gatt_cb->att_mtu_updated);
}

ZTEST(sid_ble_connection, test_02_sid_ble_conn_data_get)
{
	const sid_ble_conn_data_t *params = NULL;

	sid_ble_conn_init();
	sid_ble_conn_deinit();
	sid_ble_conn_deinit();

	sid_ble_conn_init();
	params = sid_ble_conn_data_get();
	zassert_not_null(params);

	sid_ble_conn_deinit();
	params = sid_ble_conn_data_get();
	zassert_is_null(params);
}

ZTEST(sid_ble_connection, test_03_sid_ble_conn_positive)
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

	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	zassert_equal(sid_ble_adapter_conn_connected_fake.call_count, 1);
	zassert_mem_equal(test_addr.a.val, sid_ble_adapter_conn_connected_fake.arg0_val,
			  BT_ADDR_SIZE);

	params = sid_ble_conn_data_get();
	zassert_equal(params->conn, &test_conn);
	zassert_mem_equal(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	zassert_equal(sid_ble_adapter_conn_disconnected_fake.call_count, 1);
	zassert_mem_equal(test_addr.a.val, sid_ble_adapter_conn_disconnected_fake.arg0_val,
			  BT_ADDR_SIZE);
}

ZTEST(sid_ble_connection, test_04_sid_ble_set_conn_cb_positive)
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
	sid_ble_adapter_conn_connected_fake.custom_fake = connection_callback_connected;
	sid_ble_adapter_conn_disconnected_fake.custom_fake = connection_callback_disconnected;

	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	zassert_mem_equal(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);

	params = sid_ble_conn_data_get();
	zassert_equal(params->conn, &test_conn);
	zassert_mem_equal(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	zassert_equal(conn_cb_test.state, DISCONNECTED);
	zassert_mem_equal(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);
}

ZTEST(sid_ble_connection, test_05_sid_ble_conn_cb_set_call_count)
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
	sid_ble_adapter_conn_connected_fake.custom_fake = connection_callback_connected;
	sid_ble_adapter_conn_disconnected_fake.custom_fake = connection_callback_disconnected;

	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_test.num_calls, conn_cb_cnt_expected);

	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_test.num_calls, conn_cb_cnt_expected);

	sid_bt_conn_cb->connected(&test_conn, test_error_timeout);

	bt_conn_get_dst_fake.return_val = NULL;
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_test.num_calls, conn_cb_cnt_expected);
}

ZTEST(sid_ble_connection, test_06_sid_ble_disconnected_wrong_conn)
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
	sid_ble_adapter_conn_connected_fake.custom_fake = connection_callback_connected;
	sid_ble_adapter_conn_disconnected_fake.custom_fake = connection_callback_disconnected;

	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_test.num_calls, conn_cb_cnt_expected);

	sid_bt_conn_cb->disconnected(&test_wrong_conn, test_no_error);
	zassert_equal(conn_cb_test.num_calls, conn_cb_cnt_expected);

	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_test.num_calls, conn_cb_cnt_expected);
}

ZTEST(sid_ble_connection, test_07_sid_ble_cb_set_before_init)
{
	size_t conn_cb_cnt_expected = 0;
	struct bt_conn test_conn = { .dummy = 0xDC };

	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	sid_ble_adapter_conn_connected_fake.custom_fake = connection_callback_connected;
	sid_ble_adapter_conn_disconnected_fake.custom_fake = connection_callback_disconnected;

	sid_bt_conn_cb->connected(&test_conn, 0);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_test.num_calls, conn_cb_cnt_expected);

	sid_bt_conn_cb->disconnected(&test_conn, 19);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_test.num_calls, conn_cb_cnt_expected);
}

ZTEST(sid_ble_connection, test_08_sid_ble_conn_mtu_callback)
{
	struct bt_conn test_conn = { .dummy = 0xDC };

	sid_ble_conn_init();

	uint16_t tx_mtu = 32, rx_mtu = 44;

	sid_bt_gatt_cb->att_mtu_updated(&test_conn, tx_mtu, rx_mtu);
	zassert_equal(sid_ble_adapter_mtu_changed_fake.call_count, 1);
	zassert_equal(sid_ble_adapter_mtu_changed_fake.arg0_val, tx_mtu);
}

ZTEST(sid_ble_connection, test_09_sid_ble_conn_mtu_callback_curent_connection)
{
	struct bt_conn curr_conn = { 0 };
	struct bt_conn unknow_conn = { 0 };

	sid_ble_conn_init();
	bt_conn_ref_fake.return_val = &curr_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	sid_bt_conn_cb->connected(&curr_conn, 0);

	uint16_t tx_mtu = 32, rx_mtu = 44;

	sid_bt_gatt_cb->att_mtu_updated(&curr_conn, tx_mtu, rx_mtu);
	zassert_equal(sid_ble_adapter_mtu_changed_fake.call_count, 1);
	zassert_equal(sid_ble_adapter_mtu_changed_fake.arg0_val, tx_mtu);

	sid_bt_gatt_cb->att_mtu_updated(&unknow_conn, tx_mtu, rx_mtu);
	zassert_equal(sid_ble_adapter_mtu_changed_fake.call_count, 1);
}

ZTEST(sid_ble_connection, test_10_sid_ble_conn_disconnect)
{
	struct bt_conn test_conn = { .dummy = 0xDC };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	sid_ble_conn_init();
	const sid_ble_conn_data_t *data = sid_ble_conn_data_get();
	if (data != NULL && data->conn != NULL) {
		int (*get_info_fakes[])(const struct bt_conn *, struct bt_conn_info *) = {
			bt_conn_get_info_fake1,
			bt_conn_get_info_fake1,
		};
		SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, get_info_fakes, 2);
		sid_bt_conn_cb->disconnected(data->conn, 0);
	}
	sid_ble_conn_deinit();
	sid_ble_conn_init();

	zassert_equal(sid_ble_conn_disconnect(), -ENOENT);

	sid_ble_conn_init();
	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;
	int (*custom_fakes[])(const struct bt_conn *,
			      struct bt_conn_info *) = { bt_conn_get_info_fake1 };
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 1);

	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);

	bt_conn_disconnect_fake.return_val = ESUCCESS;
	zassert_equal(sid_ble_conn_disconnect(), ESUCCESS);

	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);
	bt_conn_disconnect_fake.return_val = -ENOTCONN;
	zassert_equal(sid_ble_conn_disconnect(), -ENOTCONN);
}

ZTEST(sid_ble_connection, test_11_sid_ble_disconnect_cb_still_cleans_up_when_conn_param_get_fails)
{
	uint8_t test_reason = BT_HCI_ERR_CONN_TIMEOUT;
	struct bt_conn test_conn = { .dummy = 0xAB };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	int (*custom_fakes[])(const struct bt_conn *, struct bt_conn_info *) = {
		bt_conn_get_info_fake1,
		bt_conn_get_info_fake1,
		bt_conn_get_info_fake_error,
	};
	SET_CUSTOM_FAKE_SEQ(bt_conn_get_info, custom_fakes, 3);

	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);

	const sid_ble_conn_data_t *params = sid_ble_conn_data_get();
	zassert_not_null(params);
	zassert_is_null(params->conn);
}

ZTEST(sid_ble_connection, test_12_sid_ble_conn_param_get)
{
	struct bt_le_conn_param param_out = { 0 };

	zassert_equal(sid_ble_conn_param_get(NULL), -EINVAL);

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	zassert_equal(sid_ble_conn_param_get(&param_out), ESUCCESS);

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

	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);

	memset(&param_out, 0, sizeof(param_out));
	zassert_equal(sid_ble_conn_param_get(&param_out), ESUCCESS);
	zassert_equal(param_out.interval_min, param_get_fake_interval);
	zassert_equal(param_out.interval_max, param_get_fake_interval);
	zassert_equal(param_out.latency, param_get_fake_latency);
	zassert_equal(param_out.timeout, param_get_fake_timeout);
}

ZTEST(sid_ble_connection, test_13_sid_ble_conn_param_update)
{
	const struct bt_le_conn_param param_in = {
		.interval_min = 18,
		.interval_max = 24,
		.latency = 0,
		.timeout = 500,
	};

	zassert_equal(sid_ble_conn_param_update(NULL), -EINVAL);

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	zassert_equal(sid_ble_conn_param_update(&param_in), ESUCCESS);

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

	sid_bt_conn_cb->connected(&test_conn, BT_HCI_ERR_SUCCESS);

	bt_conn_le_param_update_fake.call_count = 0;
	bt_conn_le_param_update_fake.return_val = ESUCCESS;
	zassert_equal(sid_ble_conn_param_update(&param_in), ESUCCESS);
	zassert_equal(bt_conn_le_param_update_fake.call_count, 1);
	zassert_equal(bt_conn_le_param_update_fake.arg0_val, &test_conn);
	zassert_equal(bt_conn_le_param_update_fake.arg1_val->interval_min, param_in.interval_min);
	zassert_equal(bt_conn_le_param_update_fake.arg1_val->interval_max, param_in.interval_max);

	bt_conn_le_param_update_fake.return_val = -EINVAL;
	zassert_equal(sid_ble_conn_param_update(&param_in), ESUCCESS);
	zassert_equal(bt_conn_le_param_update_fake.call_count, 2);
}
