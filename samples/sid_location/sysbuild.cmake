#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# MCUboot needs the flash partitions of the application
string(REPLACE "/" "_" board_overlay "${BOARD}_${BOARD_QUALIFIERS}.overlay")
sysbuild_cache_set(VAR mcuboot_EXTRA_DTC_OVERLAY_FILE APPEND
		   ${CMAKE_CURRENT_LIST_DIR}/boards/${board_overlay})

if(SB_CONFIG_BOARD_NRF54L15DK_NRF54L15_CPUAPP AND
   "${SHIELD};${${DEFAULT_IMAGE}_SHIELD}" MATCHES "nrf_sidewalk_eb")
	sysbuild_cache_set(VAR ${DEFAULT_IMAGE}_EXTRA_DTC_OVERLAY_FILE APPEND
			   ${CMAKE_CURRENT_LIST_DIR}/shields/nrf_sidewalk_eb_nrf54l15dk.overlay)
endif()
