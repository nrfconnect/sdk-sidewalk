#
# Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file. This file is a
# Modifiable File, as defined in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#

if(NOT LINK_TYPE_SUPPORT)
  set(LINK_TYPE_SUPPORT DUAL)
endif()

add_library(sid_pal_radio_sx126x_impl)

target_include_directories(sid_pal_radio_sx126x_impl
  PUBLIC
    include
    include/semtech
)

target_link_libraries(sid_pal_radio_sx126x_impl
  PUBLIC
    sid_pal_radio_ifc
    semtech_radio_ifc
    sid_pal_serial_bus_ifc
    zephyr_interface
  PRIVATE
    sid_clock_ifc
    sid_pal_critical_region_ifc
    sid_pal_delay_ifc
    sid_pal_gpio_ifc
    sid_utils
)

target_sources(sid_pal_radio_sx126x_impl
  PRIVATE
    sx126x_hal.c
    sx126x_radio.c
    sx126x_radio_fsk.c
    sx126x_radio_lora.c
    semtech/sx126x.c
    semtech/sx126x_halo.c
    semtech/sx126x_timings.c
    include/sx126x_radio.h
  PUBLIC
    include/sx126x_config.h
)

if(HALO_BUILD_DIAGNOSTICS)
  target_sources(sid_pal_radio_sx126x_impl
    PRIVATE
      sx126x_radio_test.c
  )
endif()

target_compile_definitions(sid_pal_radio_sx126x_impl
  PRIVATE
    $<$<STREQUAL:${LINK_TYPE_SUPPORT},DUAL>:DUAL_LINK_SUPPORT=1>
    $<$<STREQUAL:${LINK_TYPE_SUPPORT},GATEWAY>:DUAL_LINK_SUPPORT=1>
    $<$<STREQUAL:${LINK_TYPE_SUPPORT},FSK>:DUAL_LINK_SUPPORT=0>
    $<$<STREQUAL:${LINK_TYPE_SUPPORT},FSK_PAN>:DUAL_LINK_SUPPORT=0>
)

target_compile_options(sid_pal_radio_sx126x_impl
  PRIVATE
    -Wno-error=switch
)

install(TARGETS sid_pal_radio_sx126x_impl OPTIONAL)
