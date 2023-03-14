#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(mcuboot_KCONFIG_ROOT $ENV{ZEPHYR_BASE}/../sidewalk/tests/manual/power_ble_only/child_image/mcuboot/Kconfig.root)

include($ENV{ZEPHYR_BASE}/../sidewalk/samples/common/bootloader_version.cmake)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_template)

zephyr_include_directories(
  $ENV{ZEPHYR_BASE}/../sidewalk/samples/common/include
)

target_sources(app PRIVATE
  src/main.c
  $ENV{ZEPHYR_BASE}/../sidewalk/samples/common/src/sidewalk_thread.c
)

