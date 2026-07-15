#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
# Compatibility INTERFACE libs for PAL type headers not yet in the public SDK drop.

add_library(sid_pal_timer_types INTERFACE)

target_include_directories(sid_pal_timer_types
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_pal_timer_types
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/sid_pal_timer_types.h
)

add_library(sid_pal_platform_init_types INTERFACE)

target_include_directories(sid_pal_platform_init_types
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

if(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
  target_include_directories(sid_pal_platform_init_types INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../../../semtech/lr1110_location/include
  )
endif()

target_sources(sid_pal_platform_init_types
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/sid_pal_platform_init_types.h
)

add_library(sid_time_types INTERFACE)

target_sources(sid_time_types
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../../../app_utils/config/include/sid_time_types.h
)

target_include_directories(sid_time_types
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../../../app_utils/config/include
)

install(TARGETS sid_time_types OPTIONAL)

set(SID_COMMON_HEADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../app_utils/config/include)

add_library(sid_time_ops INTERFACE)
target_include_directories(sid_time_ops INTERFACE ${SID_COMMON_HEADERS_DIR})
target_link_libraries(sid_time_ops INTERFACE sid_time_types)
install(TARGETS sid_time_ops OPTIONAL)

add_library(sid_base64 INTERFACE)
target_include_directories(sid_base64 INTERFACE ${SID_COMMON_HEADERS_DIR})
install(TARGETS sid_base64 OPTIONAL)

add_library(sid_utils INTERFACE)
target_include_directories(sid_utils INTERFACE ${SID_COMMON_HEADERS_DIR})
install(TARGETS sid_utils OPTIONAL)
