#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_library(sid_state_lite_ifc INTERFACE)

target_compile_features(sid_state_lite_ifc INTERFACE c_std_99)

target_include_directories(sid_state_lite_ifc
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_state_lite_ifc
  INTERFACE
    sid_state_lite.h
)
