#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_include_directories(.)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_UPTIME zephyr_time.c)
