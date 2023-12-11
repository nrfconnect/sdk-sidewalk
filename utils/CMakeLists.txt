#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(
    .
    include
)
add_subdirectory(src)

zephyr_library_sources(sid_api_delegated.c)
