/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "sid_error.h"
#include "zephyr/random/random.h"
#include "zephyr/sys/printk.h"
#include <stdlib.h>
#include <zephyr/ztest.h>

#include <sid_pal_storage_kv_ifc.h>

#include <zephyr/device.h>
#include <string.h>
#include <zephyr/kernel.h>

#define GROUP_ID_TEST 123
#define GROUP_ID_TEST_2 124
#define NUMBER_OF_READ_ATTEMPTS 3
#define NUMBER_OF_ELEMENTS_TO_SAVE 200
uint32_t random_elements[NUMBER_OF_ELEMENTS_TO_SAVE] = {};

static void *sid_pal_storage_test_init(void)
{
	zassert_equal(SID_ERROR_NONE, sid_pal_storage_kv_init());

	return NULL;
}

ZTEST(functional_storage, test_1_save_many_elements)
{
	sid_pal_storage_kv_group_delete(GROUP_ID_TEST);
	sid_pal_storage_kv_group_delete(GROUP_ID_TEST_2);
	uint32_t time[NUMBER_OF_ELEMENTS_TO_SAVE] = { 0 };
	for (uint32_t i = 0; i < NUMBER_OF_ELEMENTS_TO_SAVE; i++) {
		random_elements[i] = sys_rand32_get();
		int64_t start = k_uptime_get();
		sid_error_t e = sid_pal_storage_kv_record_set(
			GROUP_ID_TEST, i, (void *)&random_elements[i], sizeof(random_elements[i]));
		int64_t stop = k_uptime_get();
		time[i] = MAX(stop - start, 0);
		zassert_equal(SID_ERROR_NONE, e, "Failed at iteration %d with error %d", i, e);
	}
	printk("{");
	for (uint32_t i = 0; i < NUMBER_OF_ELEMENTS_TO_SAVE; i++) {
		printk("%d, ", time[i]);
	}
	printk("}\n");
}

ZTEST(functional_storage, test_2_sequential_read)
{
	uint32_t read_elements[NUMBER_OF_ELEMENTS_TO_SAVE] = {};
	uint32_t time[NUMBER_OF_ELEMENTS_TO_SAVE] = { 0 };
	for (uint32_t i = 0; i < NUMBER_OF_ELEMENTS_TO_SAVE; i++) {
		int64_t start = k_uptime_get();
		sid_error_t e = sid_pal_storage_kv_record_get(
			GROUP_ID_TEST, i, (void *)&read_elements[i], sizeof(read_elements[i]));
		int64_t stop = k_uptime_get();
		zassert_equal(SID_ERROR_NONE, e, "Failed at iteration %d with error %d", i, e);
		time[i] = MAX(stop - start, 0);
	}
	printk("{");
	for (uint32_t i = 0; i < NUMBER_OF_ELEMENTS_TO_SAVE; i++) {
		printk("%d, ", time[i]);
	}
	printk("}\n");
}

ZTEST(functional_storage, test_3_save_few_elements_multiple)
{
	uint32_t time[NUMBER_OF_ELEMENTS_TO_SAVE] = { 0 };
	for (uint32_t i = 0; i < NUMBER_OF_ELEMENTS_TO_SAVE; i++) {
		int64_t start = k_uptime_get();
		sid_error_t e = sid_pal_storage_kv_record_set(GROUP_ID_TEST_2, i % 10,
							      (void *)&random_elements[i],
							      sizeof(random_elements[i]));
		int64_t stop = k_uptime_get();
		zassert_equal(SID_ERROR_NONE, e, "Failed at iteration %d with error %d", i, e);
		time[i] = MAX(stop - start, 0);
	}
	printk("{");
	for (uint32_t i = 0; i < NUMBER_OF_ELEMENTS_TO_SAVE; i++) {
		if (i % 10 == 0) {
			printk("\n");
		}
		printk("%.4d, ", time[i]);
	}
	printk("}\n");
}

ZTEST(functional_storage, test_4_read_few_elements_multiple)
{
	uint32_t read_elements[10] = {};
	uint32_t time[10] = { 0 };
	for (uint32_t i = 0; i < 10; i++) {
		random_elements[i] = sys_rand32_get();
		int64_t start = k_uptime_get();
		sid_error_t e = sid_pal_storage_kv_record_get(
			GROUP_ID_TEST_2, i, (void *)&read_elements[i], sizeof(read_elements[i]));
		int64_t stop = k_uptime_get();
		zassert_equal(SID_ERROR_NONE, e, "Failed at iteration %d with error %d", i, e);
		time[i] = MAX(stop - start, 0);
	}
	printk("{");
	for (uint32_t i = 0; i < 10; i++) {
		printk("%d, ", time[i]);
	}
	printk("}\n");
}

ZTEST(functional_storage, test_5_cleanup)
{
	sid_pal_storage_kv_group_delete(GROUP_ID_TEST);
	sid_pal_storage_kv_group_delete(GROUP_ID_TEST_2);
}

ZTEST_SUITE(functional_storage, NULL, sid_pal_storage_test_init, NULL, NULL, NULL);
