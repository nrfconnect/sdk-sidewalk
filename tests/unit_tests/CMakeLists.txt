#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

add_definitions(-DHALO_ENABLE_DIAGNOSTICS=1)
add_definitions(-DDEV_ID_REG=0x33AABB99)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test)

cmock_handle(${ZEPHYR_BASE}/include/fs/nvs.h)
cmock_handle(${ZEPHYR_BASE}/include/drivers/flash.h)
cmock_handle(${ZEPHYR_BASE}/../sidewalk/pal/utils/zephyr_time.h)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE .)

# generate runner for the test
test_runner_generate(${app_sources})
