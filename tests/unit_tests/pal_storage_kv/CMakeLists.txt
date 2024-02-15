#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_storage_kv)

# add test file
FILE(GLOB app_sources src/*.c)
target_include_directories(app PRIVATE "${ZEPHYR_BASE}/../sidewalk/subsys/sal/common/sid_pal_ifc"
        "${ZEPHYR_BASE}/../sidewalk/subsys/sal/common/sid_ifc"
        "${ZEPHYR_BASE}/../sidewalk/utils/include"
        )
target_sources(app PRIVATE 
        ${app_sources} 
        "${ZEPHYR_BASE}/../sidewalk/subsys/sal/sid_pal/src/sid_storage.c" 
        "${ZEPHYR_BASE}/../sidewalk/utils/src/settings_utils.c"
        )
