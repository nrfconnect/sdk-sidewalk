#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_timer_test)

get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE .)
target_include_directories(app PRIVATE ${SIDEWALK_BASE}/subsys/app_utils/config/include)
target_link_options(app PRIVATE "LINKER:--start-group")
target_link_libraries(app PRIVATE sidewalk_pal)
target_link_options(app PRIVATE "LINKER:--end-group")
