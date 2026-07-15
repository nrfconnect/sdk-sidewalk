
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

add_library(sid_pal_radio_lr1110_impl)

target_include_directories(sid_pal_radio_lr1110_impl
  PUBLIC
    include
    include/semtech
)

target_link_libraries(sid_pal_radio_lr1110_impl
  PUBLIC
    semtech_radio_ifc
    sid_pal_radio_ifc
    sid_pal_serial_bus_ifc
    zephyr_interface
  PRIVATE
    sid_clock_ifc
    sid_pal_delay_ifc
    sid_pal_gpio_ifc
    sid_pal_log_ifc
    sid_utils
)

target_sources(sid_pal_radio_lr1110_impl
  PRIVATE
    lr1110_radio.c
    lr1110_hal.c
    lr1110_radio_fsk.c
    lr1110_radio_lora.c
    semtech/lr1110_bootloader.c
    semtech/lr1110_crypto_engine.c
    semtech/lr1110_driver_version.c
    semtech/lr1110_gnss.c
    semtech/lr1110_radio.c
    semtech/lr1110_regmem.c
    semtech/lr1110_system.c
    semtech/lr1110_wifi.c
    semtech/lr1110_halo.c
    semtech/lr1110_radio_timings.c
  PUBLIC
    include/lr1110_config.h
)

if(HALO_BUILD_DIAGNOSTICS)
  target_sources(sid_pal_radio_lr1110_impl
    PRIVATE
      lr1110_radio_test.c
  )
endif()

target_compile_definitions(sid_pal_radio_lr1110_impl
  PRIVATE
    $<$<STREQUAL:${LINK_TYPE_SUPPORT},DUAL>:DUAL_LINK_SUPPORT=1>
    $<$<STREQUAL:${LINK_TYPE_SUPPORT},GATEWAY>:DUAL_LINK_SUPPORT=1>
    $<$<STREQUAL:${LINK_TYPE_SUPPORT},FSK>:DUAL_LINK_SUPPORT=0>
    $<$<STREQUAL:${LINK_TYPE_SUPPORT},FSK_PAN>:DUAL_LINK_SUPPORT=0>
)

target_compile_options(sid_pal_radio_lr1110_impl
  PRIVATE
    -Wno-error=switch
    -DLR1110_DISABLE_WARNINGS=1
)

install(TARGETS sid_pal_radio_lr1110_impl OPTIONAL)
