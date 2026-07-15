#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(SID_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_library(sidewalk_sdk INTERFACE)

target_link_libraries(sidewalk_sdk INTERFACE
	${SID_LIB_DIR}/libsid_protocol_all.a
	${SID_LIB_DIR}/libsmtc_lbm.a
	${SID_LIB_DIR}/libsid_fsk_tpc.a
	sid_sdk_component_ifc
	ace
)
