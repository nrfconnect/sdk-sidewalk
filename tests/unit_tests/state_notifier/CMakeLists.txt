#
#  Copyright (c) 2023 Nordic Semiconductor ASA
#
#  SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
project(state_notifier)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

target_sources(testbinary PRIVATE src/main.c ${SIDEWALK_BASE}/utils/state_notifier/state_notifier.c)

target_include_directories(testbinary PRIVATE include ${SIDEWALK_BASE}/utils/include)
