#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Zephyr CMake project
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_sid_end_device)

target_include_directories(app PRIVATE include)

target_sources(app
  PRIVATE
    main.c
    rtt_io.c
    sidewalk_thread.c
    sx126x/app_subGHz_config.c
)

target_link_libraries(app
  PRIVATE
    # app dependencies
    sid_asd_cli
    sid_diagnostics_cli
    sid_protocol_diagnostics
    sid_clock_ifc
    sid_on_dev_cert
    sid_on_dev_cert_cli
    sid_utils
)
