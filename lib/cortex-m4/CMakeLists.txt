#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if (CONFIG_SIDEWALK_SUBGHZ)
	add_subdirectory(lora_fsk)
else()
	add_subdirectory(ble_only)
endif()
