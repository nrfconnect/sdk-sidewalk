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
    ext/*.c
    $ENV{ZEPHYR_BASE}/../sidewalk/subsys/config/board_init.c
    $ENV{ZEPHYR_BASE}/../sidewalk/subsys/config/app_ble_config.c
)

target_sources(app PRIVATE ${app_sources})

zephyr_include_directories(
    $ENV{ZEPHYR_BASE}/../sidewalk/subsys/config
    include
    ext
)

if(CONFIG_SIDEWALK_SUBGHZ)
  target_sources(app PRIVATE $ENV{ZEPHYR_BASE}/../sidewalk/subsys/config/app_subGHz_config.c)
endif()
