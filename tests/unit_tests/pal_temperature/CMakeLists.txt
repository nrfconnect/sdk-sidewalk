#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

project(device)

target_sources(app PRIVATE
	src/main.c
	src/dummy_sensor.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_temperature.c
)

set_property(SOURCE ${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_temperature.c PROPERTY COMPILE_FLAGS
	"-include src/kconfig_mock.h")

target_include_directories(app PRIVATE
	.
	src
	${SIDEWALK_BASE}/subsys/sal/sid_pal/include
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/temperature
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_error
)
