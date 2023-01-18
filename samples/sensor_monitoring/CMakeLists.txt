#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor_monitoring)

FILE(GLOB app_sources
    src/*.c
    config/*.c
    ext/*.c
)

if (CONFIG_SIDEWALK_LINK_MASK_BLE)
    list(FILTER app_sources EXCLUDE REGEX "_900_*")
endif()

target_sources(app PRIVATE ${app_sources})

zephyr_include_directories(
    config
    include
    ext
)
