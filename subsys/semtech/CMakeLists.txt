#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)
zephyr_include_directories_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X sx126x/include)
zephyr_include_directories_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 lr1110/include)

add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X sx126x)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 lr1110)
