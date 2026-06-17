#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(BOARD unit_testing)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_sid_ble_adapter_callbacks)
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

add_definitions(--include ztest.h)
target_compile_definitions(testbinary PRIVATE CONFIG_SIDEWALK_BLE_ADAPTER_LOG_LEVEL=0)

target_include_directories(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/sal/common/sid_pal_ifc
	${SIDEWALK_BASE}/subsys/sal/sid_pal/include
	${SIDEWALK_BASE}/subsys/sal/common/sid_ifc
)

target_sources(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_ble_adapter_callbacks.c
	src/main.c
	mock/log_minimal.c
)
