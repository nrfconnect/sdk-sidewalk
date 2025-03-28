#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(
    include
    include/semtech
)

zephyr_library_sources(
    sx126x_hal.c
    sx126x_radio.c
    sx126x_radio_fsk.c
    sx126x_radio_lora.c
    semtech/sx126x.c
    semtech/sx126x_halo.c
    semtech/sx126x_timings.c
)

if(CONFIG_SIDEWALK_BUILD_DIAGNOSTICS)
    zephyr_library_sources(sx126x_radio_test.c)
endif()

zephyr_library_compile_definitions(
  DUAL_LINK_SUPPORT=1
)
