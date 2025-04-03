#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Zephyr CMake project
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_sid_end_device)

# Add library directory
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)

target_include_directories(app PRIVATE include)

target_sources(app
  PRIVATE
    main.c
    rtt_io.c
    sidewalk_thread.c
)

target_link_libraries(app
  PRIVATE
    # Static libraries
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libsid_asd_cli.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libsid_diagnostics_cli.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libsid_protocol_diagnostics.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libsid_protocol_phy.a
    # TBD: other dependencies
    # sid_clock_ifc
)
