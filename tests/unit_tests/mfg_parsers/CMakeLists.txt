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
	src/mfg_parser.c
	src/mock_mem.c
	src/sid_crypto_keys.c
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
	${SIDEWALK_BASE}/subsys/app_utils/hal/include
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/mfg_store
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_error
	${SIDEWALK_BASE}/subsys/config/common/include
	${SIDEWALK_BASE}/subsys/app_utils/config/include
)
