#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sidewalk_test_crypto)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources} $ENV{ZEPHYR_BASE}/../sidewalk/pal/src/sid_crypto.c)
target_include_directories(app PRIVATE .)
zephyr_include_directories(${ZEPHYR_BASE}/../mbedtls/include)
set_property(SOURCE $ENV{ZEPHYR_BASE}/../sidewalk/pal/src/sid_crypto.c PROPERTY COMPILE_FLAGS
	"-include src/kconfig_mock.h")

# generate runner for the test
test_runner_generate(${app_sources})
