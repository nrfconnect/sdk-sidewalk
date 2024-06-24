#
#  Copyright (c) 2023 Nordic Semiconductor ASA
#
#  SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
project(scratch_buffer)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0")
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

target_sources(testbinary PRIVATE src/main.c ${SIDEWALK_BASE}/samples/sid_end_device/src/sbdt/scratch_buffer.c mock/log_minimal.c)

target_include_directories(testbinary PRIVATE ${SIDEWALK_BASE}/samples/sid_end_device/include  ${SIDEWALK_BASE}/subsys/hal/include)
