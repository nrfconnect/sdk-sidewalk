#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sid_validation_timer)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE src)
target_compile_definitions(app PRIVATE SID_PAL_PLATFORM_SW_SPIN_COUNT_PER_MS=5890)

# generate runner for the test
test_runner_generate(${app_sources})
