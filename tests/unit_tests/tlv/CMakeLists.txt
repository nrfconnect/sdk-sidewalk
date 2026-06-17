#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(BOARD unit_testing)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

project(device)

add_definitions(-DARCH_STACK_PTR_ALIGN=8)

FILE(GLOB app_sources src/*.c)

target_sources(testbinary PRIVATE
	${app_sources}
	${SIDEWALK_BASE}/utils/tlv/tlv.c
	${SIDEWALK_BASE}/utils/tlv/tlv_ram_storage_impl.c
)

target_include_directories(testbinary PRIVATE
	.
	src
	${SIDEWALK_BASE}/utils/include
)
