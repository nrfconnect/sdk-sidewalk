#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(BOARD unit_testing)
project(sidewalk_test_ace_alloc)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

add_definitions(--include ztest.h)
target_compile_definitions(testbinary PRIVATE CONFIG_SIDEWALK_HEAP_SIZE=1024 CONFIG_SIDEWALK_LOG_LEVEL=0)
add_definitions(-DARCH_STACK_PTR_ALIGN=8)

target_sources(testbinary PRIVATE
	src/main.c
	${SIDEWALK_BASE}/subsys/sal/common/ace/osal_alloc.c
	${ZEPHYR_BASE}/lib/heap/heap.c
)

target_include_directories(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/sal/common/ace/include
	${SIDEWALK_BASE}/subsys/sal/common/ace/include/ace
	${SIDEWALK_BASE}/subsys/app_utils/hal/include
)
