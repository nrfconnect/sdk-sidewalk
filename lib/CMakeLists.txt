#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

if (CONFIG_SIDEWALK)
	if(CONFIG_SIDEWALK_BLE_ONLY_LIBRARIES)
		add_subdirectory(ble_only)
	elseif(CONFIG_SIDEWALK_BLE_AND_900_LIBRARIES)
		add_subdirectory(lora_fsk)
	endif()
endif(CONFIG_SIDEWALK)
