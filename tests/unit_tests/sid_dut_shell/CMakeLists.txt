#
#  Copyright (c) 2023 Nordic Semiconductor ASA
#
#  SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
project(state_notifier)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

target_sources(testbinary PRIVATE
        src/main.c
        ${SIDEWALK_BASE}/samples/sid_dut/src/sid_dut_shell.c
        mock/sidewalk_version.c
)

add_definitions(--include ztest.h)

target_include_directories(testbinary PRIVATE
        .
        ${SIDEWALK_BASE}/samples/sid_dut/src
        ${SIDEWALK_BASE}/subsys/sal/common/sid_ifc
        ${SIDEWALK_BASE}/subsys/sal/common/sid_time_ops
        ${SIDEWALK_BASE}/subsys/sal/common/sid_pal_ifc
        ${SIDEWALK_BASE}/utils/include
        ${SIDEWALK_BASE}/utils
)
