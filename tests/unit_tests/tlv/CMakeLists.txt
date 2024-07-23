#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

project(device)

FILE(GLOB app_sources src/*.c)

target_sources(app PRIVATE
	${app_sources}
	${SIDEWALK_BASE}/utils/tlv/tlv.c
	${SIDEWALK_BASE}/utils/tlv/tlv_ram_storage_impl.c
)

target_include_directories(app PRIVATE
	.
	src
	${SIDEWALK_BASE}/utils/include
)
