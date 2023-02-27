# 
#  Copyright (c) 2023 Nordic Semiconductor ASA
# 
#  SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
# 

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
set(SIDEWALK_DIR $ENV{ZEPHYR_BASE}/../sidewalk)
project(state_notifier)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})


target_sources(testbinary PRIVATE 
        src/main.c
        ${SIDEWALK_DIR}/samples/sid_dut/src/sid_dut_shell.c
        mock/sidewalk_version.c
)

add_definitions(--include ztest.h)

target_include_directories(testbinary PRIVATE 
        . 
        ${SIDEWALK_DIR}/samples/sid_dut/src
        ${SIDEWALK_DIR}/lib/include
        ${SIDEWALK_DIR}/hal/include
        ${SIDEWALK_DIR}/include
)
