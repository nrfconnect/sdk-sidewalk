#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_assert)

set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)
# Workaround on build system not able to find this definition (in kernel.h)
add_definitions(-DARCH_STACK_PTR_ALIGN=8)
# add test file
target_include_directories(testbinary PRIVATE ${SIDEWALK_BASE}/subsys/sal/common/sid_pal_ifc)
target_sources(testbinary PRIVATE 
    ${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_assert.c
    src/main.c
)
