#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(BOARD unit_testing)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_log)
get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

add_definitions(--include ztest.h)
target_compile_definitions(testbinary PRIVATE CONFIG_SIDEWALK_LOG_LEVEL=0 CONFIG_SIDEWALK_LOG_MSG_LENGTH_MAX=80)

target_sources(testbinary PRIVATE
	src/main.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_log.c
	mock/log_minimal.c
)

target_include_directories(testbinary PRIVATE
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/log
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_error
)
