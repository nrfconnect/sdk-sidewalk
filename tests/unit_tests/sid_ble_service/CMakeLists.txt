#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_sid_ble_service)
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

target_include_directories(app PRIVATE $${SIDEWALK_BASE}/subsys/sal/sid_pal/include)
target_sources(app PRIVATE ${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_ble_service.c ${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_ble_ama_service.c)

cmock_handle(${SIDEWALK_BASE}/subsys/sal/sid_pal/include/sid_ble_adapter_callbacks.h)

# add test file
target_sources(app PRIVATE src/main.c)

# generate runner for the test
test_runner_generate(src/main.c)
