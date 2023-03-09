#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_include_directories(include)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK src/app_ble_config.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_SUBGHZ src/app_subGHz_config.c)
