#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_storage_kv)
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

# add test file
FILE(GLOB app_sources src/*.c)
target_include_directories(app PRIVATE "${SIDEWALK_BASE}/subsys/sal/common/sid_pal_ifc"
        "${SIDEWALK_BASE}/subsys/sal/common/sid_ifc"
        "${SIDEWALK_BASE}/utils/include"
        )
target_sources(app PRIVATE 
        ${app_sources} 
        "${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_storage.c" 
        "${SIDEWALK_BASE}/utils/settings_utils/settings_utils.c"
        )
