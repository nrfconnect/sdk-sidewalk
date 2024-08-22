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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")

target_sources(testbinary PRIVATE
        src/main.c
        ${SIDEWALK_BASE}/samples/sid_end_device/src/cli/app_shell.c
        $ENV{ZEPHYR_BASE}/lib/utils/hex.c
        mock/sidewalk_version.c
)

add_definitions(--include ztest.h)

target_include_directories(testbinary PRIVATE
        ${SIDEWALK_BASE}/samples/sid_end_device/include
        ${SIDEWALK_BASE}/subsys/sal/common/sid_ifc
        ${SIDEWALK_BASE}/subsys/sal/common/sid_time_ops
        ${SIDEWALK_BASE}/subsys/sal/common/sid_pal_ifc
        ${SIDEWALK_BASE}/subsys/hal/include
        ${SIDEWALK_BASE}/utils/include
        ${SIDEWALK_BASE}/utils
        .
        src
)
