#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_library(sid_mfg_credentials_ifc INTERFACE)

target_compile_features(sid_mfg_credentials_ifc INTERFACE c_std_99)

target_include_directories(sid_mfg_credentials_ifc
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_mfg_credentials_ifc
  INTERFACE
    sid_mfg_credentials.h
)

target_link_libraries(sid_mfg_credentials_ifc
  INTERFACE
    sid_error
)
