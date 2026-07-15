#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(BOARD unit_testing)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_sid_ble_adapter_callbacks)
get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)
include(${SIDEWALK_BASE}/tests/cmake/sidewalk_test_ifc.cmake)

add_definitions(--include ztest.h)
target_compile_definitions(testbinary PRIVATE CONFIG_SIDEWALK_BLE_ADAPTER_LOG_LEVEL=0)

target_include_directories(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/sal/sid_pal/include
)

sidewalk_link_test_ifc(testbinary ${SIDEWALK_BASE} ${CMAKE_CURRENT_BINARY_DIR} sid_pal_ble_adapter_ifc)

target_sources(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_ble_adapter_callbacks.c
	src/main.c
	mock/log_minimal.c
)
