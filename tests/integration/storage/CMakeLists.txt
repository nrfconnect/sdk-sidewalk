#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_storage_test)

# add test file
FILE(GLOB app_sources src/*.c)
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)
target_sources(app PRIVATE ${app_sources} ${SIDEWALK_BASE}/utils/settings_utils/settings_utils.c)
target_include_directories(app PRIVATE .  ${SIDEWALK_BASE}/utils/include)
