#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_uptime)
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

cmock_handle(${SIDEWALK_BASE}/subsys/sal/sid_pal/include/zephyr_time.h)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# generate runner for the test
test_runner_generate(${app_sources})
