#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

zephyr_library_sources(
    nrf70_wifi_scan.c
    sid_pal_wifi_nrf70_impl.c
)
