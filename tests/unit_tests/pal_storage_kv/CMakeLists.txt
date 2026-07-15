#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_storage_kv)
get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

target_include_directories(app PRIVATE
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/storage_kv
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_error
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_sdk_config
	${SIDEWALK_BASE}/utils/include
)

target_sources(app PRIVATE
	src/main.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_storage.c
	${SIDEWALK_BASE}/utils/settings_utils/settings_utils.c
)
