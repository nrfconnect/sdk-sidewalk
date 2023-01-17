#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

include($ENV{ZEPHYR_BASE}/../sidewalk/samples/common/bootloader_version.cmake)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_template)

zephyr_include_directories(
  src/
  $ENV{ZEPHYR_BASE}/../sidewalk/samples/common/include
)

target_sources(app PRIVATE
  src/main.c
  src/sid_dut_shell.c
  src/sid_api_delegated.c
  src/sid_thread.c
)

if (CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	target_sources(app PRIVATE $ENV{ZEPHYR_BASE}/../sidewalk/samples/common/src/nordic_dfu.c)
endif()
