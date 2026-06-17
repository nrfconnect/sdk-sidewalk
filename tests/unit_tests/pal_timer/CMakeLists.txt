#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_timer)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
	${app_sources}
	mock/critical_region.c
)

zephyr_link_libraries(
	-Wl,--wrap=sid_pal_enter_critical_region
	-Wl,--wrap=sid_pal_exit_critical_region
)
