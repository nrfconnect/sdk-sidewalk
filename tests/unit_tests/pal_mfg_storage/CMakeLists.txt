#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

add_definitions(-DHALO_ENABLE_DIAGNOSTICS=1)
add_definitions(-DDEV_ID_REG=0x33AABB99)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_mfg_storage)

cmock_handle(${ZEPHYR_BASE}/include/zephyr/drivers/flash.h)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# generate runner for the test
test_runner_generate(${app_sources})
