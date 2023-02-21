#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

include($ENV{ZEPHYR_BASE}/../sidewalk/utils/bootloader_version.cmake)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_template)

zephyr_include_directories(
  $ENV{ZEPHYR_BASE}/../sidewalk/utils/include
)

target_sources(app PRIVATE
  src/main.c
  $ENV{ZEPHYR_BASE}/../sidewalk/utils/src/sidewalk_thread.c
)

if (CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	target_sources(app PRIVATE $ENV{ZEPHYR_BASE}/../sidewalk/utils/src/nordic_dfu.c)
endif()
