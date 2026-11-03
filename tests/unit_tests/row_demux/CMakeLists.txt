#
#  Copyright (c) 2026 Nordic Semiconductor ASA
#
#  SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
project(row_demux)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0")
add_definitions(--include ztest.h)
get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

target_sources(testbinary PRIVATE
	src/main.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_row_demux.c
)

target_include_directories(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/sal/sid_pal/include
)
