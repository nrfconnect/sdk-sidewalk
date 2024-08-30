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
	${SIDEWALK_BASE}/utils/tlv/tlv_flash_storage_impl.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_mfg_hex_v7.c
	${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_mfg_hex_v8.c
)

target_include_directories(app PRIVATE
	.
	./mocks
	src
	${SIDEWALK_BASE}/utils/include
	${SIDEWALK_BASE}/subsys/sal/sid_pal/include
	${SIDEWALK_BASE}/subsys/hal/include
	${SIDEWALK_BASE}/subsys/sal/common/sid_pal_ifc
	${SIDEWALK_BASE}/subsys/config/common/include
)
