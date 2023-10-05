#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(hci_rpmsg_KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/child_image/hci_rpmsg/Kconfig.root)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

find_package(PythonInterp 3.6 REQUIRED)
find_package(Python 3.6 REQUIRED)

execute_process(COMMAND ${PYTHON_EXECUTABLE} $ENV{ZEPHYR_BASE}/../sidewalk/scripts/generate_version_info_overlay.py

project(sensor_monitoring)

FILE(GLOB app_sources
    src/*.c
    ext/*.c
)

target_sources(app PRIVATE ${app_sources})

zephyr_include_directories(
    include
    ext
)
