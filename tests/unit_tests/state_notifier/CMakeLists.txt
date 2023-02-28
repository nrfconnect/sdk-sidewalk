# 
#  Copyright (c) 2023 Nordic Semiconductor ASA
# 
#  SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
# 

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
project(state_notifier)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(SIDEAWLK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

target_sources(testbinary PRIVATE src/main.c ${SIDEAWLK_BASE}/utils/src/state_notifier.c)

target_include_directories(testbinary PRIVATE . ${SIDEAWLK_BASE}/utils/include)
