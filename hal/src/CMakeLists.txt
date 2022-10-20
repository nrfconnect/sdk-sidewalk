#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_library_sources(
    memory.c
    reset.c
)

if (CONFIG_SIDEWALK_CLI)
    zephyr_library()
    zephyr_library_sources(shell.c)
    zephyr_include_directories(${ZEPHYR_BASE}/../sidewalk/lib/include/)
    zephyr_include_directories(${ZEPHYR_BASE}/../sidewalk/samples/template/include)
endif()
