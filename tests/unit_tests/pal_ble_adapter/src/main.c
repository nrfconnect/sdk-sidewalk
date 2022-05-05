/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <fff.h>

#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_service.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/addr.h>
#include <mock_settings.h>
#include <mock_sid_ble_advert.h>
#include <mock_sid_ble_connection.h>
#include <errno.h>

#include <stdbool.h>

#define ESUCCESS (0)

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, bt_enable, bt_ready_cb_t);
FAKE_VALUE_FUNC(int, bt_le_adv_start, const struct bt_le_adv_param *, const struct bt_data *, size_t, const struct bt_data *, size_t);
FAKE_VOID_FUNC(bt_conn_cb_register, struct bt_conn_cb *);
FAKE_VALUE_FUNC(struct bt_conn *, bt_conn_ref, struct bt_conn *);
FAKE_VOID_FUNC(bt_conn_unref, struct bt_conn *);
FAKE_VALUE_FUNC(const bt_addr_le_t *, bt_conn_get_dst, const struct bt_conn *);
FAKE_VALUE_FUNC(uint16_t, bt_gatt_get_mtu, struct bt_conn *);
FAKE_VALUE_FUNC(bool, bt_gatt_is_subscribed, struct bt_conn *,
		const struct bt_gatt_attr *, uint16_t);
FAKE_VALUE_FUNC(int, bt_gatt_notify_cb, struct bt_conn *,
		struct bt_gatt_notify_params *);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_service, struct bt_conn *,
		const struct bt_gatt_attr *,
		void *, uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_chrc, struct bt_conn *,
		const struct bt_gatt_attr *, void *,
		uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_ccc, struct bt_conn *,
		const struct bt_gatt_attr *, void *,
		uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_write_ccc, struct bt_conn *,
		const struct bt_gatt_attr *, const void *,
		uint16_t, uint16_t, uint8_t);
FAKE_VALUE_FUNC(struct bt_gatt_attr *, bt_gatt_find_by_uuid, const struct bt_gatt_attr *,
		uint16_t,
		const struct bt_uuid *);


#define FFF_FAKES_LIST(FAKE)		\
	FAKE(bt_enable)			\
	FAKE(bt_le_adv_start)		\
	FAKE(bt_conn_cb_register)	\
	FAKE(bt_conn_ref)		\
	FAKE(bt_conn_get_dst)		\
	FAKE(bt_gatt_get_mtu)		\
	FAKE(bt_gatt_is_subscribed)	\
	FAKE(bt_gatt_notify_cb)		\
	FAKE(bt_gatt_attr_read_service)	\
	FAKE(bt_gatt_attr_read_chrc)	\
	FAKE(bt_gatt_attr_read_ccc)	\
	FAKE(bt_gatt_attr_write_ccc)	\
	FAKE(bt_gatt_find_by_uuid)

typedef struct {
	uint8_t num_calls;
	uint8_t *ptr_data;
	uint8_t data_len;
} data_callback_test_t;

static sid_pal_ble_adapter_interface_t p_test_ble_ifc;
static sid_ble_config_t test_ble_cfg;
static data_callback_test_t data_cb_test;

void setUp(void)
{
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	memset(&data_cb_test, 0x00, sizeof(data_cb_test));
}

void tearDown(void)
{
	p_test_ble_ifc = NULL;
}

static void ble_data_callback(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
	++data_cb_test.num_calls;
	data_cb_test.ptr_data = data;
	data_cb_test.data_len = length;
}

static void ble_notify_callback(sid_ble_cfg_service_identifier_t id, bool state)
{
}

static void connection_callback(bool state, uint8_t *addr)
{
}

static void ble_indication_callback(bool status)
{
}

static void ble_mtu_callback(uint16_t size)
{
}

static void ble_adv_start_callback(void)
{
}

static void set_callbacks(sid_pal_ble_adapter_callbacks_t *cb)
{
	cb->conn_callback = connection_callback;
	cb->mtu_callback = ble_mtu_callback;
	cb->adv_start_callback = ble_adv_start_callback;
	cb->ind_callback = ble_indication_callback;
	cb->data_callback = ble_data_callback;
	cb->notify_callback = ble_notify_callback;
}

static void reset_callbacks(sid_pal_ble_adapter_callbacks_t *cb)
{
	memset(cb, 0x00, sizeof(sid_pal_ble_adapter_callbacks_t));
	RESET_FAKE(bt_enable);
	RESET_FAKE(bt_conn_cb_register);
}

void test_sid_pal_ble_adapter_create_negative(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_ble_adapter_create(NULL));
}

void test_sid_pal_ble_adapter_init(void)
{
	bt_enable_fake.return_val = ESUCCESS;
	__wrap_settings_load_ExpectAndReturn(0);
	__wrap_sid_ble_conn_init_Expect();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->init(&test_ble_cfg));

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->init(NULL));

	bt_enable_fake.return_val = -ENOENT;
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, p_test_ble_ifc->init(&test_ble_cfg));

	bt_enable_fake.return_val = ESUCCESS;
	__wrap_settings_load_ExpectAndReturn(-ENOENT);
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, p_test_ble_ifc->init(&test_ble_cfg));
}

void test_sid_pal_ble_adapter_deinit(void)
{
	__wrap_settings_load_ExpectAndReturn(ESUCCESS);
	__wrap_sid_ble_conn_init_Expect();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->init(&test_ble_cfg));

	__wrap_sid_ble_conn_deinit_Expect();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->deinit());
}

void test_ble_adapter_start_advertisement(void)
{
	__wrap_sid_ble_advert_start_ExpectAndReturn(ESUCCESS);
	__wrap_sid_ble_conn_init_Expect();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->start_adv());

	__wrap_sid_ble_advert_start_ExpectAndReturn(-ENOENT);
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, p_test_ble_ifc->start_adv());
}

const struct bt_gatt_attr *ama_svrc_attr_get_by_uuid_128(const struct bt_uuid *uuid)
{
	const struct bt_gatt_service_static *p_ama_svrc = sid_ble_get_ama_service();

	if (p_ama_svrc) {
		for (int i = 0; i < p_ama_svrc->attr_count; i++) {
			if (BT_UUID_TYPE_128 == p_ama_svrc->attrs[i].uuid->type) {
				struct bt_uuid_128 *p_ama_uuid = (struct bt_uuid_128 *)p_ama_svrc->attrs[i].uuid;
				struct bt_uuid_128 *p_searched_uuid = (struct bt_uuid_128 *)uuid;

				if (0 == memcmp(p_ama_uuid->val, p_searched_uuid->val, BT_UUID_SIZE_128)) {
					return &p_ama_svrc->attrs[i];
				}
			}
		}
	}

	return NULL;
}

void test_ble_adapter_data_receive_wo_callback(void)
{
	uint8_t buffer[128];
	uint16_t offset = 0;
	uint8_t flags = 0;

	const struct bt_gatt_attr *p_ama_write_attr = ama_svrc_attr_get_by_uuid_128(AMA_SID_BT_CHARACTERISTIC_WRITE);

	TEST_ASSERT_NOT_NULL_MESSAGE(p_ama_write_attr, "Can't find Sidewalk service with write attribute");

	p_ama_write_attr->write(NULL, NULL, buffer, sizeof(buffer), offset, flags);
	TEST_ASSERT_EQUAL(0, data_cb_test.num_calls);
	TEST_ASSERT_EQUAL(0, data_cb_test.data_len);
	TEST_ASSERT_NULL(data_cb_test.ptr_data);
}

void test_ble_adapter_set_callback_null(void)
{
	sid_pal_ble_adapter_callbacks_t cb;

	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, p_test_ble_ifc->set_callback(NULL));

	reset_callbacks(&cb);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));

	reset_callbacks(&cb);
	set_callbacks(&cb);
	cb.conn_callback = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));

	reset_callbacks(&cb);
	set_callbacks(&cb);
	cb.mtu_callback = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));

	reset_callbacks(&cb);
	set_callbacks(&cb);
	cb.adv_start_callback = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));

	reset_callbacks(&cb);
	set_callbacks(&cb);
	cb.ind_callback = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));

	reset_callbacks(&cb);
	set_callbacks(&cb);
	cb.data_callback = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));

	reset_callbacks(&cb);
	set_callbacks(&cb);
	cb.notify_callback = NULL;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));

	reset_callbacks(&cb);
	cb.conn_callback = connection_callback;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));
	cb.mtu_callback = ble_mtu_callback;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));
	cb.adv_start_callback = ble_adv_start_callback;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));
	cb.ind_callback = ble_indication_callback;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));
	cb.data_callback = ble_data_callback;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_callback(&cb));
}

void test_ble_adapter_set_callback_pass(void)
{
	sid_pal_ble_adapter_callbacks_t cb;

	set_callbacks(&cb);
	__wrap_sid_ble_conn_cb_set_Ignore();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
}

void test_sid_pal_ble_set_conn_callbacks(void)
{
	sid_pal_ble_adapter_callbacks_t cb;

	set_callbacks(&cb);

	__wrap_sid_ble_conn_cb_set_Expect(connection_callback);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));

}

void test_ble_adapter_start_service(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->start_service());
}

void test_ble_adapter_send_data_unsuppored_id(void)
{
	sid_pal_ble_adapter_callbacks_t cb;
	uint8_t data[128];

	set_callbacks(&cb);

	__wrap_sid_ble_conn_cb_set_Ignore();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, p_test_ble_ifc->send(VENDOR_SERVICE, data, sizeof(data)));
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, p_test_ble_ifc->send(LOGGING_SERVICE, data, sizeof(data)));
	TEST_ASSERT_EQUAL(SID_ERROR_NOSUPPORT, p_test_ble_ifc->send(9, data, sizeof(data)));
}

void test_ble_adapter_send_data_invalid_args(void)
{
	sid_pal_ble_adapter_callbacks_t cb;
	struct bt_gatt_attr attr;
	uint8_t data[128];
	const sid_ble_conn_params_t test_conn_params;

	set_callbacks(&cb);
	bt_gatt_find_by_uuid_fake.return_val = &attr;

	__wrap_sid_ble_conn_cb_set_Ignore();
	__wrap_sid_ble_conn_params_get_IgnoreAndReturn(&test_conn_params);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->send(AMA_SERVICE, NULL, 0));

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->send(AMA_SERVICE, NULL, sizeof(data)));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->send(AMA_SERVICE, data, 0));
	bt_gatt_get_mtu_fake.return_val = (sizeof(data) - 1);
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->send(AMA_SERVICE, data, sizeof(data)));
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = false;
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->send(AMA_SERVICE, data, sizeof(data)));
}

void test_ble_adapter_send_data_invalid_fail(void)
{
	sid_pal_ble_adapter_callbacks_t cb;
	struct bt_gatt_attr attr;
	uint8_t data[128];

	set_callbacks(&cb);

	__wrap_sid_ble_conn_cb_set_Ignore();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = -ENOENT;
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, p_test_ble_ifc->send(AMA_SERVICE, data, sizeof(data)));
}

void test_ble_adapter_send_data_invalid_uuid(void)
{
	sid_pal_ble_adapter_callbacks_t cb;
	uint8_t data[128];

	set_callbacks(&cb);

	__wrap_sid_ble_conn_cb_set_Ignore();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	bt_gatt_find_by_uuid_fake.return_val = NULL;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NULL_POINTER, p_test_ble_ifc->send(AMA_SERVICE, data, sizeof(data)));
}

void test_ble_adapter_send_data_pass(void)
{
	sid_pal_ble_adapter_callbacks_t cb;
	struct bt_gatt_attr attr;
	uint8_t data[128];

	set_callbacks(&cb);

	__wrap_sid_ble_conn_cb_set_Ignore();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = 0;
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->send(AMA_SERVICE, data, sizeof(data)));
}

void test_ble_adapter_data_receive_pass(void)
{
	sid_pal_ble_adapter_callbacks_t cb;
	uint8_t buffer[128];
	uint16_t offset = 0;
	uint8_t flags = 0;

	memset(buffer, 0xCC, sizeof(buffer));

	set_callbacks(&cb);
	__wrap_sid_ble_conn_cb_set_Ignore();
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));

	const struct bt_gatt_attr *p_ama_write_attr = ama_svrc_attr_get_by_uuid_128(AMA_SID_BT_CHARACTERISTIC_WRITE);
	TEST_ASSERT_NOT_NULL_MESSAGE(p_ama_write_attr, "Can't find Sidewalk service with write attribute");

	p_ama_write_attr->write(NULL, NULL, buffer, sizeof(buffer), offset, flags);
	TEST_ASSERT_EQUAL(1, data_cb_test.num_calls);
	TEST_ASSERT_EQUAL(sizeof(buffer), data_cb_test.data_len);
	TEST_ASSERT_EQUAL_MEMORY(buffer, data_cb_test.ptr_data, sizeof(buffer));
}

void test_ble_adapter_stop_advertisement(void)
{
	__wrap_sid_ble_advert_stop_ExpectAndReturn(ESUCCESS);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->stop_adv());

	__wrap_sid_ble_advert_stop_ExpectAndReturn(-ENOENT);
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, p_test_ble_ifc->stop_adv());
}

void test_ble_adapter_set_adv_data(void)
{
	uint8_t test_data[] = "Lorem ipsum.";

	__wrap_sid_ble_advert_update_IgnoreAndReturn(ESUCCESS);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->set_adv_data(test_data, sizeof(test_data)));

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_adv_data(NULL, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_adv_data(test_data, 0));
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_adv_data(NULL, sizeof(test_data)));

	__wrap_sid_ble_advert_update_IgnoreAndReturn(-ENOENT);
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, p_test_ble_ifc->set_adv_data(test_data, sizeof(test_data)));
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
