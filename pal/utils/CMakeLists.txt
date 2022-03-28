#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_library()

zephyr_include_directories(.)

zephyr_library_sources(
	zephyr_time.c
)
