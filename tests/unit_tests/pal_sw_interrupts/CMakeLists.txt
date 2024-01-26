#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
project(sidewalk_test_critical_region)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

add_definitions(--include ztest.h)

target_include_directories(testbinary PRIVATE
    src
    ${SIDEWALK_BASE}/subsys/sal/common/sid_pal_ifc
    ${SIDEWALK_BASE}/subsys/sal/common/sid_ifc
)

target_compile_definitions(testbinary PRIVATE CONFIG_SIDEWALK_SWI_PRIORITY=1 CONFIG_SIDEWALK_SWI_STACK_SIZE=2048)

target_sources(testbinary PRIVATE ${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_sw_interrupts.c)

target_sources(testbinary PRIVATE
    src/main.c
    src/mock_zephyr.c
)
