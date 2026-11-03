#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_library(sid_serial_registration_ifc INTERFACE)

target_compile_features(sid_serial_registration_ifc INTERFACE c_std_99)

target_include_directories(sid_serial_registration_ifc
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_serial_registration_ifc
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/sid_serial_ifc.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/sid_serial_registration.h>
    $<INSTALL_INTERFACE:sid_serial_ifc.h>
    $<INSTALL_INTERFACE:sid_serial_registration.h>
)

set_target_properties(sid_serial_registration_ifc PROPERTIES PUBLIC_HEADER
  "${CMAKE_CURRENT_LIST_DIR}/sid_serial_ifc.h;${CMAKE_CURRENT_LIST_DIR}/sid_serial_registration.h"
)

target_link_libraries(sid_serial_registration_ifc
  INTERFACE
    sid_error
)
