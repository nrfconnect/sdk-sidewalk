#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Zephyr CMake project
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_sid_location)

# Source files
target_sources(app PRIVATE src/main.c)

if(CONFIG_SIDEWALK_USE_PREBUILT_LIBRARIES)
	target_link_libraries(app PRIVATE sidewalk)
endif()
