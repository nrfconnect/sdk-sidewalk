#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(BOARD unit_testing)
project(sidewalk_test_ace_alloc)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

add_definitions(--include ztest.h)
target_compile_definitions(testbinary PRIVATE CONFIG_SIDEWALK_HEAP_SIZE=1024 CONFIG_SIDEWALK_LOG_LEVEL=0)
add_definitions(-DARCH_STACK_PTR_ALIGN=8)

FILE(GLOB app_sources src/*.c)
target_sources(testbinary PRIVATE
	${app_sources}
	${SIDEWALK_BASE}/subsys/ace/osal/osal_alloc.c
	${ZEPHYR_BASE}/lib/heap/heap.c
)

target_include_directories(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/ace/osal
	${SIDEWALK_BASE}/subsys/ace/include
	${SIDEWALK_BASE}/subsys/hal/include
)
