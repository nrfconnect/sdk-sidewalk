/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <zephyr/fff.h>

#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <errno.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, bt_le_adv_start, const struct bt_le_adv_param *, const struct bt_data *, size_t,
		const struct bt_data *, size_t);
FAKE_VALUE_FUNC(int, bt_le_adv_stop);
FAKE_VALUE_FUNC(int, bt_le_adv_update_data, const struct bt_data *, size_t, const struct bt_data *, size_t);

#define FFF_FAKES_LIST(FAKE)  \
	FAKE(bt_le_adv_start) \
	FAKE(bt_le_adv_stop)  \
	FAKE(bt_le_adv_update_data)

#define ESUCCESS (0)
#define TEST_BUFFER_LEN (100)
#define BT_COMP_ID_LEN 2

void setUp(void)
{
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}

void test_sid_ble_advert_start(void)
{
	size_t adv_start_call_count = 0;

	bt_le_adv_start_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_start());
	adv_start_call_count++;
	TEST_ASSERT_EQUAL(adv_start_call_count, bt_le_adv_start_fake.call_count);

	bt_le_adv_start_fake.return_val = -ENOENT;
	TEST_ASSERT_EQUAL(-ENOENT, sid_ble_advert_start());
	adv_start_call_count++;
	TEST_ASSERT_EQUAL(adv_start_call_count, bt_le_adv_start_fake.call_count);
}

void test_sid_ble_advert_stop(void)
{
	size_t adv_stop_call_count = 0;

	bt_le_adv_stop_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_stop());
	adv_stop_call_count++;
	TEST_ASSERT_EQUAL(adv_stop_call_count, bt_le_adv_stop_fake.call_count);

	bt_le_adv_stop_fake.return_val = -ENOENT;
	TEST_ASSERT_EQUAL(-ENOENT, sid_ble_advert_stop());
	adv_stop_call_count++;
	TEST_ASSERT_EQUAL(adv_stop_call_count, bt_le_adv_stop_fake.call_count);
}

void test_sid_ble_advert_update(void)
{
	uint8_t test_data[] = "Lorem ipsum.";
	size_t adv_update_call_count = 0;

	bt_le_adv_start_fake.return_val = ESUCCESS;
	bt_le_adv_update_data_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_start());
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));
	adv_update_call_count++;
	TEST_ASSERT_EQUAL(adv_update_call_count, bt_le_adv_update_data_fake.call_count);

	bt_le_adv_update_data_fake.return_val = -ENOENT;
	TEST_ASSERT_EQUAL(-ENOENT, sid_ble_advert_update(test_data, sizeof(test_data)));
	adv_update_call_count++;
	TEST_ASSERT_EQUAL(adv_update_call_count, bt_le_adv_update_data_fake.call_count);

	bt_le_adv_update_data_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(-EINVAL, sid_ble_advert_update(NULL, sizeof(test_data)));
	TEST_ASSERT_EQUAL(-EINVAL, sid_ble_advert_update(test_data, 0));
}

void test_sid_ble_advert_update_in_every_state(void)
{
	uint8_t test_data[] = "Lorem ipsum.";

	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));

	bt_le_adv_start_fake.return_val = ESUCCESS;
	bt_le_adv_update_data_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_start());
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));

	bt_le_adv_stop_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_stop());
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));
}

bool advert_data_manuf_data_get(const struct bt_data *ad, size_t ad_len, uint8_t *result, uint8_t *result_len)
{
	for (size_t i = 0; i < ad_len; i++) {
		if (ad[i].type == BT_DATA_MANUFACTURER_DATA) {
			TEST_ASSERT_GREATER_OR_EQUAL_UINT8(BT_COMP_ID_LEN, ad[i].data_len);
			TEST_ASSERT_EQUAL_UINT8(((BT_COMP_ID_AMA) & 0xff), ad[i].data[0]);
			TEST_ASSERT_EQUAL_UINT8((((BT_COMP_ID_AMA) >> 8) & 0xff), ad[i].data[1]);

			*result_len = ad[i].data_len - BT_COMP_ID_LEN;
			memcpy(result, &ad[i].data[BT_COMP_ID_LEN], *result_len);
			return true;
		}
	}
	return false;
}

void test_sid_ble_advert_update_before_start(void)
{
	uint8_t test_result[TEST_BUFFER_LEN] = { 0 };
	uint8_t test_result_size;
	uint8_t test_data[] = "Lorem ipsum.";
	const struct bt_data *advert_data;
	size_t advert_data_size;
	bool found;

	bt_le_adv_stop_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_stop());

	bt_le_adv_update_data_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)));

	bt_le_adv_start_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_start());

	advert_data = bt_le_adv_start_fake.arg1_val;
	advert_data_size = bt_le_adv_start_fake.arg2_val;
	TEST_ASSERT_NOT_NULL(advert_data);
	TEST_ASSERT_GREATER_THAN_size_t(0, advert_data_size);

	found = advert_data_manuf_data_get(advert_data, advert_data_size, test_result, &test_result_size);
	TEST_ASSERT_MESSAGE(found, "Manufacturer data not found in advertising data.");
	TEST_ASSERT_EQUAL_UINT8(sizeof(test_data), test_result_size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data, test_result, sizeof(test_data));
}

void check_sid_ble_advert_update(uint8_t *data, uint8_t data_len)
{
	uint8_t test_result[TEST_BUFFER_LEN] = { 0 };
	uint8_t test_result_size;
	const struct bt_data *advert_data;
	size_t advert_data_size;
	bool found;

	bt_le_adv_start_fake.return_val = ESUCCESS;
	bt_le_adv_update_data_fake.return_val = ESUCCESS;
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_start());
	TEST_ASSERT_EQUAL(ESUCCESS, sid_ble_advert_update(data, data_len));

	advert_data = bt_le_adv_update_data_fake.arg0_val;
	advert_data_size = bt_le_adv_update_data_fake.arg1_val;
	TEST_ASSERT_NOT_NULL(advert_data);
	TEST_ASSERT_GREATER_THAN_size_t(0, advert_data_size);

	found = advert_data_manuf_data_get(advert_data, advert_data_size, test_result, &test_result_size);
	TEST_ASSERT_MESSAGE(found, "Manufacturer data not found in advertising data.");
	TEST_ASSERT_EQUAL_UINT8(data_len, test_result_size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(data, test_result, data_len);
}

void test_sid_ble_advert_update_value(void)
{
	uint8_t test_data[] = "normal";
	uint8_t test_data_short[] = "s";
	uint8_t test_data_very_long[] = "very long data";

	check_sid_ble_advert_update(test_data, sizeof(test_data));
	check_sid_ble_advert_update(test_data_short, sizeof(test_data_short));
	check_sid_ble_advert_update(test_data_very_long, sizeof(test_data_very_long));
}

extern int unity_main(void);

int main(void)
{
	(void)unity_main();

	return 0;
}
