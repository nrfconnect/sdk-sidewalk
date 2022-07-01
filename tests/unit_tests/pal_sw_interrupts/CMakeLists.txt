#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_critical_region)

target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/include)
target_sources(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/pal/src/sid_sw_interrupts.c)

# add test file
target_sources(app PRIVATE src/main.c)
target_include_directories(app PRIVATE src)

# generate runner for the test
test_runner_generate(src/main.c)
