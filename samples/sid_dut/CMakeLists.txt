#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(hci_rpmsg_KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/child_image/hci_rpmsg/Kconfig.root)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_template)

set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

zephyr_include_directories(src)
zephyr_include_directories(${SIDEWALK_BASE}/samples/common/include)
zephyr_include_directories(${SIDEWALK_BASE}/utils)

target_sources(app PRIVATE
  src/main.c
  src/sid_dut_shell.c
  src/sid_thread.c
  ${SIDEWALK_BASE}/utils/sid_api_delegated.c
)
