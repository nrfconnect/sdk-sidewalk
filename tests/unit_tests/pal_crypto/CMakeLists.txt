#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sidewalk_test_crypto)
set(SIDEAWLK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

# add test file
FILE(GLOB app_sources src/*.c)
target_include_directories(app PRIVATE .)
target_include_directories(app PRIVATE ${SIDEAWLK_BASE}/subsys/sal/common/sid_pal_ifc)
target_include_directories(app PRIVATE ${SIDEAWLK_BASE}/subsys/sal/common/sid_ifc)
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../modules/crypto/mbedtls/include)
target_sources(app PRIVATE ${app_sources} ${SIDEAWLK_BASE}/subsys/sal/sid_pal/src/sid_crypto.c)
set_property(SOURCE ${SIDEAWLK_BASE}/subsys/sal/sid_pal/src/sid_crypto.c PROPERTY COMPILE_FLAGS "-include src/kconfig_mock.h")

# generate runner for the test
test_runner_generate(${app_sources})
