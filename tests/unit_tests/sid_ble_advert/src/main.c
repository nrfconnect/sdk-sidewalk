/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <zephyr/fff.h>

#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>
#include <sid_ble_connection.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <errno.h>
#include <string.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, bt_le_ext_adv_start, struct bt_le_ext_adv *,
		const struct bt_le_ext_adv_start_param *);
FAKE_VALUE_FUNC(int, bt_le_ext_adv_stop, struct bt_le_ext_adv *);
FAKE_VALUE_FUNC(int, bt_le_ext_adv_set_data, struct bt_le_ext_adv *, const struct bt_data *, size_t,
		const struct bt_data *, size_t);
FAKE_VALUE_FUNC(int, bt_le_ext_adv_delete, struct bt_le_ext_adv *);
FAKE_VALUE_FUNC(int, bt_le_ext_adv_create, const struct bt_le_adv_param *,
		const struct bt_le_ext_adv_cb *, struct bt_le_ext_adv **);
FAKE_VALUE_FUNC(const sid_ble_conn_data_t *, sid_ble_conn_data_get);

#define FFF_FAKES_LIST(FAKE)                                                                       \
	FAKE(bt_le_ext_adv_start)                                                                  \
	FAKE(bt_le_ext_adv_stop)                                                                   \
	FAKE(bt_le_ext_adv_set_data)                                                               \
	FAKE(bt_le_ext_adv_delete)                                                                 \
	FAKE(bt_le_ext_adv_create)                                                                 \
	FAKE(sid_ble_conn_data_get)

#define ESUCCESS (0)
#define TEST_BUFFER_LEN (100)
#define BT_COMP_ID_LEN 2

/* Opaque type: use a byte so create fake can set a non-NULL adv_set for the module */
static char dummy_adv_set_storage;

static int bt_le_ext_adv_create_custom_fake(const struct bt_le_adv_param *a,
					    const struct bt_le_ext_adv_cb *b,
					    struct bt_le_ext_adv **out)
{
	(void)a;
	(void)b;
	if (out != NULL) {
		*out = (struct bt_le_ext_adv *)&dummy_adv_set_storage;
	}
	return bt_le_ext_adv_create_fake.return_val;
}

static void advert_start_with_fakes(void)
{
	bt_le_ext_adv_create_fake.custom_fake = bt_le_ext_adv_create_custom_fake;
	bt_le_ext_adv_create_fake.return_val = 0;
	bt_le_ext_adv_start_fake.return_val = ESUCCESS;
	bt_le_ext_adv_set_data_fake.return_val = ESUCCESS;
	bt_le_ext_adv_stop_fake.return_val = ESUCCESS;
	bt_le_ext_adv_delete_fake.return_val = ESUCCESS;
	zassert_equal(ESUCCESS, sid_ble_advert_start());
}

static void before_test(void *fixture)
{
	ARG_UNUSED(fixture);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();

	bt_le_ext_adv_stop_fake.return_val = ESUCCESS;
	bt_le_ext_adv_delete_fake.return_val = ESUCCESS;
	(void)sid_ble_advert_deinit();
	FFF_RESET_HISTORY();
}

ZTEST_SUITE(ble_advert, NULL, NULL, before_test, NULL, NULL);

ZTEST(ble_advert, test_sid_ble_advert_start)
{
	size_t adv_start_call_count = 0;

	advert_start_with_fakes();
	adv_start_call_count = bt_le_ext_adv_start_fake.call_count;

	bt_le_ext_adv_create_fake.custom_fake = bt_le_ext_adv_create_custom_fake;
	bt_le_ext_adv_create_fake.return_val = 0;
	bt_le_ext_adv_start_fake.return_val = -ENOENT;
	bt_le_ext_adv_set_data_fake.return_val = ESUCCESS;
	zassert_equal(-ENOENT, sid_ble_advert_start());
	adv_start_call_count++;
	zassert_equal(adv_start_call_count, bt_le_ext_adv_start_fake.call_count);
}

ZTEST(ble_advert, test_sid_ble_advert_stop)
{
	/* No adv set or clear leftover: stop() returns 0 (may or may not call API) */
	zassert_equal(ESUCCESS, sid_ble_advert_stop());
	size_t count_after_first = bt_le_ext_adv_stop_fake.call_count;

	/* Start then stop: one more stop call, success (create fake sets adv_set so stop() calls API) */
	bt_le_ext_adv_create_fake.custom_fake = bt_le_ext_adv_create_custom_fake;
	bt_le_ext_adv_create_fake.return_val = 0;
	bt_le_ext_adv_start_fake.return_val = ESUCCESS;
	bt_le_ext_adv_set_data_fake.return_val = ESUCCESS;
	zassert_equal(ESUCCESS, sid_ble_advert_start());
	bt_le_ext_adv_stop_fake.return_val = ESUCCESS;
	zassert_equal(ESUCCESS, sid_ble_advert_stop());
	zassert_equal(count_after_first + 1, bt_le_ext_adv_stop_fake.call_count);

	/* Start then stop with error return (start() also calls stop() to clear previous adv_set) */
	bt_le_ext_adv_create_fake.custom_fake = bt_le_ext_adv_create_custom_fake;
	zassert_equal(ESUCCESS, sid_ble_advert_start());
	bt_le_ext_adv_stop_fake.return_val = -ENOENT;
	zassert_equal(-ENOENT, sid_ble_advert_stop());
	zassert_equal(count_after_first + 3, bt_le_ext_adv_stop_fake.call_count);
}

ZTEST(ble_advert, test_sid_ble_advert_update)
{
	uint8_t test_data[] = "Lorem ipsum.";
	size_t adv_update_call_count = 0;

	advert_start_with_fakes();
	adv_update_call_count = bt_le_ext_adv_set_data_fake.call_count;
	zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));
	adv_update_call_count++;
	zassert_equal(adv_update_call_count, bt_le_ext_adv_set_data_fake.call_count);

	bt_le_ext_adv_set_data_fake.return_val = -ENOENT;
	zassert_equal(-ENOENT, sid_ble_advert_update(test_data, sizeof(test_data)));
	adv_update_call_count++;
	zassert_equal(adv_update_call_count, bt_le_ext_adv_set_data_fake.call_count);

	bt_le_ext_adv_set_data_fake.return_val = ESUCCESS;
	zassert_equal(-EINVAL, sid_ble_advert_update(NULL, sizeof(test_data)));
	zassert_equal(-EINVAL, sid_ble_advert_update(test_data, 0));
}

ZTEST(ble_advert, test_sid_ble_advert_update_in_every_state)
{
	uint8_t test_data[] = "Lorem ipsum.";

	zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));

	bt_le_ext_adv_start_fake.return_val = ESUCCESS;
	bt_le_ext_adv_set_data_fake.return_val = ESUCCESS;
	advert_start_with_fakes();
	zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));

	bt_le_ext_adv_stop_fake.return_val = ESUCCESS;
	zassert_equal(ESUCCESS, sid_ble_advert_stop());
	zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));
}

static bool advert_data_manuf_data_get(const struct bt_data *ad, size_t ad_len, uint8_t *result,
				       uint8_t *result_len)
{
	for (size_t i = 0; i < ad_len; i++) {
		if (ad[i].type == BT_DATA_MANUFACTURER_DATA) {
			zassert_true(ad[i].data_len >= BT_COMP_ID_LEN);
			zassert_equal(((BT_COMP_ID_AMA) & 0xff), ad[i].data[0]);
			zassert_equal((((BT_COMP_ID_AMA) >> 8) & 0xff), ad[i].data[1]);

			*result_len = ad[i].data_len - BT_COMP_ID_LEN;
			memcpy(result, &ad[i].data[BT_COMP_ID_LEN], *result_len);
			return true;
		}
	}
	return false;
}

ZTEST(ble_advert, test_sid_ble_advert_update_before_start)
{
	uint8_t test_result[TEST_BUFFER_LEN] = { 0 };
	uint8_t test_result_size;
	uint8_t test_data[] = "Lorem ipsum.";
	const struct bt_data *advert_data;
	size_t advert_data_size;
	bool found;

	bt_le_ext_adv_stop_fake.return_val = ESUCCESS;
	zassert_equal(ESUCCESS, sid_ble_advert_stop());

	bt_le_ext_adv_set_data_fake.return_val = ESUCCESS;
	zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));

	advert_start_with_fakes();

	advert_data = bt_le_ext_adv_set_data_fake.arg1_val;
	advert_data_size = bt_le_ext_adv_set_data_fake.arg2_val;
	zassert_not_null(advert_data);
	zassert_true(advert_data_size > 0);

	found = advert_data_manuf_data_get(advert_data, advert_data_size, test_result,
					   &test_result_size);
	zassert_true(found, "Manufacturer data not found in advertising data.");
	zassert_equal(sizeof(test_data), test_result_size);
	zassert_mem_equal(test_data, test_result, sizeof(test_data));
}

static void check_sid_ble_advert_update(uint8_t *data, uint8_t data_len)
{
	uint8_t test_result[TEST_BUFFER_LEN] = { 0 };
	uint8_t test_result_size;
	const struct bt_data *advert_data;
	size_t advert_data_size;
	bool found;

	bt_le_ext_adv_start_fake.return_val = ESUCCESS;
	bt_le_ext_adv_set_data_fake.return_val = ESUCCESS;
	advert_start_with_fakes();
	zassert_equal(ESUCCESS, sid_ble_advert_update(data, data_len));

	advert_data = bt_le_ext_adv_set_data_fake.arg1_val;
	advert_data_size = bt_le_ext_adv_set_data_fake.arg2_val;
	zassert_not_null(advert_data);
	zassert_true(advert_data_size > 0);

	found = advert_data_manuf_data_get(advert_data, advert_data_size, test_result,
					   &test_result_size);
	zassert_true(found, "Manufacturer data not found in advertising data.");
	zassert_equal(data_len, test_result_size);
	zassert_mem_equal(data, test_result, data_len);
}

ZTEST(ble_advert, test_sid_ble_advert_update_value)
{
	uint8_t test_data[] = "normal";
	uint8_t test_data_short[] = "s";
	uint8_t test_data_very_long[] = "very long data";

	check_sid_ble_advert_update(test_data, sizeof(test_data));
	check_sid_ble_advert_update(test_data_short, sizeof(test_data_short));
	check_sid_ble_advert_update(test_data_very_long, sizeof(test_data_very_long));
}

ZTEST(ble_advert, test_sid_ble_advert_params_set_get)
{
	sid_ble_advert_params_t in = {
		.fast_enabled = true,
		.slow_enabled = false,
		.fast_interval = 100,
		.fast_timeout = 200,
		.slow_interval = 500,
		.slow_timeout = 0,
	};
	sid_ble_advert_params_t out = { 0 };

	zassert_equal(ESUCCESS, sid_ble_advert_params_set(&in));
	zassert_equal(ESUCCESS, sid_ble_advert_params_get(&out));
	zassert_equal(in.fast_enabled, out.fast_enabled);
	zassert_equal(in.slow_enabled, out.slow_enabled);
	zassert_equal(in.fast_interval, out.fast_interval);
	zassert_equal(in.fast_timeout, out.fast_timeout);
	zassert_equal(in.slow_interval, out.slow_interval);
	zassert_equal(in.slow_timeout, out.slow_timeout);

	zassert_equal(-EINVAL, sid_ble_advert_params_set(NULL));
	zassert_equal(-EINVAL, sid_ble_advert_params_get(NULL));
}

ZTEST(ble_advert, test_sid_ble_advert_notify_connection)
{
	uint8_t test_data[] = "connected";
	size_t set_data_calls;

	advert_start_with_fakes();
	set_data_calls = bt_le_ext_adv_set_data_fake.call_count;

	sid_ble_advert_notify_connection();

	zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));
	zassert_equal(set_data_calls, bt_le_ext_adv_set_data_fake.call_count,
		      "update must not touch HCI after connection");
}

ZTEST(ble_advert, test_sid_ble_advert_deinit_idle)
{
	zassert_equal(ESUCCESS, sid_ble_advert_deinit());
	zassert_equal(0, bt_le_ext_adv_stop_fake.call_count);
	zassert_equal(0, bt_le_ext_adv_delete_fake.call_count);
}

ZTEST(ble_advert, test_sid_ble_advert_deinit_active)
{
	size_t stop_before;
	size_t delete_before;

	advert_start_with_fakes();
	stop_before = bt_le_ext_adv_stop_fake.call_count;
	delete_before = bt_le_ext_adv_delete_fake.call_count;
	zassert_equal(ESUCCESS, sid_ble_advert_deinit());
	zassert_equal(stop_before + 1, bt_le_ext_adv_stop_fake.call_count,
		      "deinit must stop active advertising once");
	zassert_equal(delete_before + 1, bt_le_ext_adv_delete_fake.call_count,
		      "deinit must delete active advertising set once");
}

ZTEST(ble_advert, test_sid_ble_advert_deinit_stop_error)
{
	advert_start_with_fakes();
	bt_le_ext_adv_stop_fake.return_val = -EIO;
	bt_le_ext_adv_delete_fake.return_val = ESUCCESS;

	zassert_equal(ESUCCESS, sid_ble_advert_deinit());
	zassert_equal(1, bt_le_ext_adv_stop_fake.call_count);
	zassert_equal(1, bt_le_ext_adv_delete_fake.call_count);
}

ZTEST(ble_advert, test_sid_ble_advert_deinit_delete_fails)
{
	advert_start_with_fakes();
	bt_le_ext_adv_delete_fake.return_val = -EINVAL;

	zassert_equal(-EINVAL, sid_ble_advert_deinit());
	zassert_equal(1, bt_le_ext_adv_stop_fake.call_count);
	zassert_equal(1, bt_le_ext_adv_delete_fake.call_count);
}

ZTEST(ble_advert, test_sid_ble_advert_connected_deinit_restart)
{
	uint8_t test_data[] = "reinit";
	size_t set_data_calls;

	advert_start_with_fakes();
	sid_ble_advert_notify_connection();

	zassert_equal(ESUCCESS, sid_ble_advert_deinit());
	zassert_equal(1, bt_le_ext_adv_stop_fake.call_count);
	zassert_equal(1, bt_le_ext_adv_delete_fake.call_count);

	set_data_calls = bt_le_ext_adv_set_data_fake.call_count;
	zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));
	zassert_equal(set_data_calls, bt_le_ext_adv_set_data_fake.call_count,
		      "update must cache mfg data only before restart");

	advert_start_with_fakes();
}

ZTEST(ble_advert, test_sid_ble_advert_stop_fail_deinit_restart)
{
	uint8_t test_data[] = "after stop fail";
	size_t set_data_calls;

	advert_start_with_fakes();
	bt_le_ext_adv_stop_fake.return_val = -EIO;
	zassert_equal(-EIO, sid_ble_advert_stop());

	bt_le_ext_adv_delete_fake.return_val = ESUCCESS;
	zassert_equal(ESUCCESS, sid_ble_advert_deinit());

	set_data_calls = bt_le_ext_adv_set_data_fake.call_count;
	zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));
	zassert_equal(set_data_calls, bt_le_ext_adv_set_data_fake.call_count,
		      "deinit must reset stale state after failed stop");

	bt_le_ext_adv_stop_fake.return_val = ESUCCESS;
	advert_start_with_fakes();
}
