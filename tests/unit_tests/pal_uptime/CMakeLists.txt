#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(BOARD unit_testing)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_uptime)
get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)
include(${SIDEWALK_BASE}/tests/cmake/sidewalk_test_ifc.cmake)

add_definitions(--include ztest.h)
add_definitions(-DARCH_STACK_PTR_ALIGN=8)
target_compile_definitions(testbinary PRIVATE CONFIG_SIDEWALK_LOG_LEVEL=0)

target_sources(testbinary PRIVATE
	src/main.c
	mock/zephyr_time.c
	mock/log_minimal.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_uptime.c
)

target_include_directories(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/sal/sid_pal/include
)

sidewalk_link_test_ifc(testbinary ${SIDEWALK_BASE} ${CMAKE_CURRENT_BINARY_DIR} sid_pal_uptime_ifc)
