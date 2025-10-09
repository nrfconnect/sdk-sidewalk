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
    lr11xx_radio.c
    lr11xx_hal.c
    lr11xx_radio_fsk.c
    lr11xx_radio_lora.c
    lr11xx_halo.c
    semtech/lr11xx_bootloader.c
    semtech/lr11xx_crypto_engine.c
    semtech/lr11xx_driver_version.c
    semtech/lr11xx_gnss.c
    semtech/lr11xx_lr_fhss.c
    semtech/lr11xx_radio.c
    semtech/lr11xx_radio_timings.c
    semtech/lr11xx_rttof.c
    semtech/lr11xx_regmem.c
    semtech/lr11xx_system.c
    semtech/lr11xx_wifi.c
)

zephyr_library_compile_definitions(
  DUAL_LINK_SUPPORT=1
  RTOS_ZEPHYR=1
  LR11XX_DISABLE_WARNINGS=1
)
