#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_sid_ble_service)

get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

target_include_directories(app PRIVATE
	${SIDEWALK_BASE}/subsys/sal/sid_pal/include
)

target_link_libraries(app PRIVATE sid_pal_ble_adapter_ifc)
target_sources(app PRIVATE
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_ble_service.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_ble_ama_service.c
	src/main.c
)
