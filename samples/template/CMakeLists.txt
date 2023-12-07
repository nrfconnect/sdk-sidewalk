#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(hci_rpmsg_KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/child_image/hci_rpmsg/Kconfig.root)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_template)

zephyr_include_directories(src)
zephyr_include_directories($ENV{ZEPHYR_BASE}/../sidewalk/utils)

# target_sources(app PRIVATE
#   src/demo_app/main.c
# )

zephyr_library_sources_ifdef(CONFIG_SIDEWALK src/demo_app/main.c )
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_DUT_CLI src/sid_dut_shell.c)
