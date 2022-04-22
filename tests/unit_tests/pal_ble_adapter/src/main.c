/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <fff.h>

#include <sid_pal_ble_adapter_ifc.h>

#include <bluetooth/bluetooth.h>
#include <mock_settings.h>
#include <errno.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, bt_enable, bt_ready_cb_t);
FAKE_VALUE_FUNC(int, bt_le_adv_start, const struct bt_le_adv_param *, const struct bt_data *, size_t, const struct bt_data *, size_t);
FAKE_VOID_FUNC(bt_conn_cb_register, struct bt_conn_cb *);

static sid_pal_ble_adapter_interface_t p_test_ble_ifc;
static sid_ble_config_t test_ble_cfg;

void setUp(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
}

void tearDown(void)
{
	p_test_ble_ifc = NULL;
	RESET_FAKE(bt_enable);
}

void test_sid_pal_ble_adapter_create_negative(void)
{
	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_ble_adapter_create(NULL));
}

void test_sid_pal_ble_adapter_init(void)
{
	bt_enable_fake.return_val = 0;
	__wrap_settings_load_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->init(&test_ble_cfg));

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->init(NULL));

	bt_enable_fake.return_val = -ENOENT;
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, p_test_ble_ifc->init(&test_ble_cfg));

	bt_enable_fake.return_val = 0;
	__wrap_settings_load_ExpectAndReturn(-ENOENT);
	TEST_ASSERT_EQUAL(SID_ERROR_GENERIC, p_test_ble_ifc->init(&test_ble_cfg));
}

void test_sid_pal_ble_adapter_init_conn_callbacks(void)
{
	bt_enable_fake.return_val = 0;
	__wrap_settings_load_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->init(&test_ble_cfg));
	TEST_ASSERT_EQUAL(1, bt_conn_cb_register_fake.call_count);

	struct bt_conn_cb * p_test_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	TEST_ASSERT_NOT_NULL(p_test_conn_cb);
	TEST_ASSERT_NOT_NULL(p_test_conn_cb->connected);
	TEST_ASSERT_NOT_NULL(p_test_conn_cb->disconnected);

	struct bt_conn *p_test_conn=NULL;
	uint8_t test_err=0, test_reason=0;
	p_test_conn_cb->connected(p_test_conn, test_err);
	test_err = -1;
	p_test_conn_cb->connected(p_test_conn, test_err);
	p_test_conn_cb->disconnected(p_test_conn, test_reason);
	TEST_PASS_MESSAGE("Run connection callbacks.");
}

void test_sid_pal_ble_adapter_deinit(void)
{
	bt_enable_fake.return_val = 0;
	__wrap_settings_load_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->init(&test_ble_cfg));

	TEST_ASSERT_EQUAL(SID_ERROR_NONE, p_test_ble_ifc->deinit());
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
