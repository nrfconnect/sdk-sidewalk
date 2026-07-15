#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(
	include
	${CMAKE_CURRENT_SOURCE_DIR}/../sal/common/public/sid_ifc/sid_error
	${CMAKE_CURRENT_SOURCE_DIR}/../sal/common/public/sid_pal_ifc/log
)
zephyr_library_sources(sid_demo_parser.c)
