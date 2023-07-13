#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)
add_subdirectory_ifdef(CONFIG_SIDEWALK_PAL_RADIO_SOURCE sx126x)
