#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK src/app_ble_config.c)

if(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
  zephyr_library_sources_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X src/app_subGHz_config_sx126x.c)
  zephyr_library_sources_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 
    src/app_subGHz_config_lr11xx.c
    src/app_location_config_lr11xx.c
  )
else()
  zephyr_library_sources(src/app_subGHz_config_empty.c)
endif()
