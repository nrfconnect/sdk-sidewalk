#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_critical_region)
set(SIDEAWLK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

# add test file
target_include_directories(app PRIVATE src)
target_include_directories(app PRIVATE ${SIDEAWLK_BASE}/subsys/sal/common/sid_pal_ifc)
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE ${SIDEAWLK_BASE}/subsys/sal/sid_pal/src/sid_sw_interrupts.c)

# generate runner for the test
test_runner_generate(src/main.c)
