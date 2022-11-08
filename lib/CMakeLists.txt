#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

add_subdirectory_ifdef(CONFIG_SIDEWALK_LINK_MASK_FSK lora_fsk)
add_subdirectory_ifdef(CONFIG_SIDEWALK_LINK_MASK_LORA lora_fsk)
add_subdirectory_ifdef(CONFIG_SIDEWALK_LINK_MASK_BLE ble_only)
add_subdirectory_ifdef(CONFIG_APP_TEST_SIDEWALK validation)
