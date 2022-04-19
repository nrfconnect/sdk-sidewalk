/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <unity.h>
#include <sid_pal_ble_adapter_ifc.h>

void test_sid_ble_adadpter(void){
	sid_pal_ble_adapter_interface_t test_handle;

	TEST_ASSERT_EQUAL(SID_ERROR_INVALID_ARGS, sid_pal_ble_adapter_create(NULL)); 
	TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_ble_adapter_create(&test_handle)); 
}

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
