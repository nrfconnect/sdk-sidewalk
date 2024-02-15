/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "zephyr/ztest_assert.h"
#include <sid_pal_storage_kv_ifc.h>

#include <stdint.h>
#include <zephyr/ztest.h>

#include <zephyr/settings/settings.h>

ZTEST(pal_storage, test_init_save_read)
{
	sid_pal_storage_kv_init();
	const int value = 123;
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_set(0, 1, &value, sizeof(value)),
		      "record_set returned invalid value");
	uint32_t len = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_get_len(0, 1, &len));
	zassert_equal(4, len);

	int value2 = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_get(0, 1, &value2, len));
	zassert_equal(value, value2);
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_delete(0, 1));
	int value3 = 0;
	zassert_equal(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get(0, 1, &value3, len));
	const int value4 = 567;
	const int value5 = 890;
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_record_set(2, 15, &value4, sizeof(value4)),
		      "record_set returned invalid value");
	zassert_equal(SID_ERROR_NONE,
		      sid_pal_storage_kv_record_set(2, 0xff, &value5, sizeof(value5)),
		      "record_set returned invalid value");

	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_group_delete(2));
	int value44 = 0;
	zassert_equal(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get(2, 15, &value44, len));
	zassert_not_equal(value44, value4);
	int value55 = 0;
	zassert_equal(SID_ERROR_NOT_FOUND, sid_pal_storage_kv_record_get(2, 0xff, &value55, len));
	zassert_not_equal(value55, value5);
}

ZTEST(pal_storage, test_sanity)
{
	zassert_true(true);
}

ZTEST_SUITE(pal_storage, NULL, NULL, NULL, NULL, NULL);
