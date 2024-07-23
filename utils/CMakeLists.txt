#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

add_subdirectory_ifdef(CONFIG_SIDEWALK_BUTTONS buttons_utils)
add_subdirectory_ifdef(CONFIG_SIDEWALK_DFU sidewalk_dfu)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SETTINGS_UTILS settings_utils)
add_subdirectory_ifdef(CONFIG_STATE_NOTIFIER state_notifier)
add_subdirectory_ifdef(CONFIG_SIDEWALK_TLV tlv)
