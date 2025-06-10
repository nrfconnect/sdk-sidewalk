#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK src/app_ble_config.c)

if(NOT CONFIG_RADIO_LR11XX)
	zephyr_library_sources_ifdef(CONFIG_SIDEWALK_SUBGHZ_SUPPORT src/app_subGHz_config.c)
endif()
zephyr_library_sources_ifndef(CONFIG_SIDEWALK_SUBGHZ_SUPPORT src/app_subGHz_config_empty.c)

