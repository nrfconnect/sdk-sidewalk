#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if (CONFIG_SIDEWALK)
	if (CONFIG_SIDEWALK_LINK_MASK_FSK OR CONFIG_SIDEWALK_LINK_MASK_LORA)
		add_subdirectory(lora_fsk)
	else()
		add_subdirectory(ble_only)
	endif()
endif(CONFIG_SIDEWALK)
