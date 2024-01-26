#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

add_definitions(-DHALO_ENABLE_DIAGNOSTICS=1)
add_definitions(-DDEV_ID_REG=0x33AABB99)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_mfg_storage)

cmock_handle(${ZEPHYR_BASE}/include/zephyr/drivers/flash.h zephyr/drivers)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE 
        ${app_sources} 
        ${ZEPHYR_BASE}/../sidewalk/subsys/sal/sid_pal/src/sid_mfg_storage.c
)

target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/subsys/sal/common/sid_pal_ifc
        ${ZEPHYR_BASE}/../sidewalk/subsys/sal/common/sid_ifc
        ${ZEPHYR_BASE}/../sidewalk/subsys/config/common/include/)

# generate runner for the test
test_runner_generate(${app_sources})
