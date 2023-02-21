#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_timer)

cmock_handle($ENV{ZEPHYR_BASE}/../sidewalk/subsys/include/sid_pal_uptime_ifc.h)
cmock_handle($ENV{ZEPHYR_BASE}/../sidewalk/subsys/include/sid_pal_critical_region_ifc.h)
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/include)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# generate runner for the test
test_runner_generate(${app_sources})
