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
  include/
  $ENV{ZEPHYR_BASE}/../sidewalk/subsys/sal/sid_pal/include
  $ENV{ZEPHYR_BASE}/../sidewalk/subsys/config
  $ENV{ZEPHYR_BASE}/../sidewalk/utils/include
)

target_sources(app PRIVATE
  src/main.c
  src/sid_callbacks.c
  src/sid_thread.c
  $ENV{ZEPHYR_BASE}/../sidewalk/subsys/config/board_init.c
  $ENV{ZEPHYR_BASE}/../sidewalk/subsys/config/app_ble_config.c
  $ENV{ZEPHYR_BASE}/../sidewalk/utils/src/sid_api_delegated.c
)

if(CONFIG_SIDEWALK_SUBGHZ)
  target_sources(app PRIVATE $ENV{ZEPHYR_BASE}/../sidewalk/subsys/config/app_subGHz_config.c)
endif()
