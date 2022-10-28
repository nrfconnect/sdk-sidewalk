#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sid_validation_storage_kv)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE src)

# generate runner for the test
test_runner_generate(${app_sources})
