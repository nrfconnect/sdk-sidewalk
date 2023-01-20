# 
#  Copyright (c) 2023 Nordic Semiconductor ASA
# 
#  SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
# 

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(state_notifier)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})


target_sources(testbinary PRIVATE 
        src/main.c
        $ENV{ZEPHYR_BASE}/../sidewalk/tests/manual/sid_dut/src/sid_dut_shell.c
)

add_definitions(--include ztest.h)

target_include_directories(testbinary PRIVATE 
. 
$ENV{ZEPHYR_BASE}/../sidewalk/tests/manual/sid_dut/src
$ENV{ZEPHYR_BASE}/../sidewalk/lib/include
)
