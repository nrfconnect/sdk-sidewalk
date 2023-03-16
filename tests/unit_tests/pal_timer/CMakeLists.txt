#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_timer)
set(SIDEAWLK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

cmock_handle(${SIDEAWLK_BASE}/subsys/sal/common/sid_pal_ifc/sid_pal_uptime_ifc.h)
cmock_handle(${SIDEAWLK_BASE}/subsys/sal/common/sid_pal_ifc/sid_pal_critical_region_ifc.h)
target_include_directories(app PRIVATE ${SIDEAWLK_BASE}/subsys/sal/common/sid_pal_ifc)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# generate runner for the test
test_runner_generate(${app_sources})
