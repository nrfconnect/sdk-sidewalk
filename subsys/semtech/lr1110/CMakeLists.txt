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
)

zephyr_library_compile_definitions(
  DUAL_LINK_SUPPORT=1
)
