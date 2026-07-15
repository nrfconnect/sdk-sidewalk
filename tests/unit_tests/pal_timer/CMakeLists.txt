#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_timer)

get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

target_sources(app PRIVATE
	src/main.c
	src/sid_timer_stub.c
	mock/critical_region.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_timer.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_uptime.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/zephyr_time.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_assert.c
)

target_include_directories(app PRIVATE
	${SIDEWALK_BASE}/subsys/sal/sid_pal/include
	${SIDEWALK_BASE}/subsys/sal/sid_pal/sid_pal_types
	${SIDEWALK_BASE}/subsys/app_utils/config/include
)

target_link_libraries(app PRIVATE sid_sdk_component_pal_ifc)

zephyr_link_libraries(
	-Wl,--wrap=sid_pal_enter_critical_region
	-Wl,--wrap=sid_pal_exit_critical_region
)
