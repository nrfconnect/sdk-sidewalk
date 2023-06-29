#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(
    include
)

zephyr_library_sources(
    sx126x_empty.c
)

zephyr_compile_definitions_ifdef(CONFIG_SIDEWALK_LINK_MASK_FSK DUAL_LINK_SUPPORT=0)
zephyr_compile_definitions_ifndef(CONFIG_SIDEWALK_LINK_MASK_FSK DUAL_LINK_SUPPORT=1)
