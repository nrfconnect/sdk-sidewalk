#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

include(../common/bootloader_version.cmake)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_template)

zephyr_include_directories(
  ../common/include
)

target_sources(app PRIVATE
  ../common/src/main.c
  ../common/src/sidewalk_thread.c
)
