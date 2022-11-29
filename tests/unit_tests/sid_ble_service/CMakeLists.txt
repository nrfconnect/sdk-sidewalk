#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_sid_ble_service)

target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/pal/include)
target_sources(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/pal/src/sid_ble_service.c)

cmock_handle(${ZEPHYR_BASE}/../sidewalk/pal/include/sid_ble_adapter_callbacks.h)

# add test file
target_sources(app PRIVATE src/main.c)

# generate runner for the test
test_runner_generate(src/main.c)
