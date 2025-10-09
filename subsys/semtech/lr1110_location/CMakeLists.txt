#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(
    include
    .
)

zephyr_library_sources(
    sid_pal_gnss_wifi_impl.c
    smtc_modem_impl.c
)

zephyr_library_compile_definitions(
  DUAL_LINK_SUPPORT=1
  SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
  LR11XX
)
