#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_include_directories_ifdef(CONFIG_SIDEWALK_HAL include)
add_subdirectory_ifdef(CONFIG_SIDEWALK_HAL src)
