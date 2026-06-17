/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <zephyr/fff.h>

#include <sid_ble_service.h>
#include <sid_ble_connection.h>
#include <sid_ble_ama_service.h>
#include <sid_ble_adapter_callbacks.h>

#include <zephyr/bluetooth/conn.h>

#include <stdbool.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(sid_ble_adapter_notification_sent);
FAKE_VOID_FUNC(sid_ble_adapter_notification_changed, sid_ble_cfg_service_identifier_t, bool);
FAKE_VOID_FUNC(sid_ble_adapter_data_write, sid_ble_cfg_service_identifier_t, uint8_t *, uint16_t);
FAKE_VALUE_FUNC(uint16_t, bt_gatt_get_mtu, struct bt_conn *);
FAKE_VALUE_FUNC(bool, bt_gatt_is_subscribed, struct bt_conn *, const struct bt_gatt_attr *,
		uint16_t);
FAKE_VALUE_FUNC(int, bt_gatt_notify_cb, struct bt_conn *, struct bt_gatt_notify_params *);
FAKE_VALUE_FUNC(struct bt_gatt_attr *, bt_gatt_find_by_uuid, const struct bt_gatt_attr *, uint16_t,
		const struct bt_uuid *);

FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_service, struct bt_conn *, const struct bt_gatt_attr *,
		void *, uint16_t, uint16_t);

FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_chrc, struct bt_conn *, const struct bt_gatt_attr *,
		void *, uint16_t, uint16_t);

FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_ccc, struct bt_conn *, const struct bt_gatt_attr *,
		void *, uint16_t, uint16_t);

FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_write_ccc, struct bt_conn *, const struct bt_gatt_attr *,
		const void *, uint16_t, uint16_t, uint8_t);

#define FFF_FAKES_LIST(FAKE)                                                                       \
	FAKE(sid_ble_adapter_notification_sent)                                                    \
	FAKE(sid_ble_adapter_notification_changed)                                                 \
	FAKE(sid_ble_adapter_data_write)                                                           \
	FAKE(bt_gatt_get_mtu)                                                                      \
	FAKE(bt_gatt_is_subscribed)                                                                \
	FAKE(bt_gatt_notify_cb)                                                                    \
	FAKE(bt_gatt_find_by_uuid)                                                                 \
	FAKE(bt_gatt_attr_read_service)                                                            \
	FAKE(bt_gatt_attr_read_chrc)                                                               \
	FAKE(bt_gatt_attr_read_ccc)                                                                \
	FAKE(bt_gatt_attr_write_ccc)

#define TEST_DATA_CHUNK (128)

struct bt_conn {
	uint8_t dummy;
};

static void before_test(void *fixture)
{
	ARG_UNUSED(fixture);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}

ZTEST_SUITE(ble_service, NULL, NULL, before_test, NULL, NULL);

ZTEST(ble_service, test_sid_ble_send_data_null_ptr)
{
	uint8_t data[TEST_DATA_CHUNK];

	zassert_equal(-ENOENT, sid_ble_send_data(NULL, data, sizeof(data)));
}

ZTEST(ble_service, test_sid_ble_send_data_pass)
{
	struct bt_gatt_notify_params *notify_params;
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(0, sid_ble_send_data(&params, data, sizeof(data)));

	zassert_not_null(bt_gatt_notify_cb_fake.arg1_val);
	notify_params = (struct bt_gatt_notify_params *)bt_gatt_notify_cb_fake.arg1_val;
	notify_params->func(&conn, NULL);
	zassert_equal(1, sid_ble_adapter_notification_sent_fake.call_count);
}

ZTEST(ble_service, test_sid_ble_send_data_attr_fail)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = NULL;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(-ENOENT, sid_ble_send_data(&params, data, sizeof(data)));
}

ZTEST(ble_service, test_sid_ble_send_data_wo_subscription)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = false;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(-EINVAL, sid_ble_send_data(&params, data, sizeof(data)));
}

ZTEST(ble_service, test_sid_ble_send_data_incorrect_data_len)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data) - 5;
	bt_gatt_is_subscribed_fake.return_val = false;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(-EINVAL, sid_ble_send_data(&params, data, sizeof(data)));

	bt_gatt_get_mtu_fake.return_val = sizeof(data) - 1;
	zassert_equal(-EINVAL, sid_ble_send_data(&params, data, sizeof(data)));
}

ZTEST(ble_service, test_sid_ble_send_data_fail)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	int test_error_code = -ENOENT;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = test_error_code;
	zassert_equal(test_error_code, sid_ble_send_data(&params, data, sizeof(data)));
}

ZTEST(ble_service, test_sid_ble_send_data_incorrect_arguments)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = false;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(-EINVAL, sid_ble_send_data(&params, NULL, 0));
	zassert_equal(-EINVAL, sid_ble_send_data(&params, data, 0));
	zassert_equal(-EINVAL, sid_ble_send_data(&params, NULL, sizeof(data)));
}
