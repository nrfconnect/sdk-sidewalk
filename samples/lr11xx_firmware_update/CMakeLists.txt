#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_lr11xx_firmware_update)

target_include_directories(app PRIVATE include)
target_sources(app PRIVATE
	src/main.c
	src/app.c
	src/lr11xx_firmware_update.c
)
