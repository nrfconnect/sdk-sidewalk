#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# Sidewalk-specific nrf7002eb2 overlays live under boards/shields/nrf7002eb2/.
# Do not map state-notifier-time-sync to &led1 in board overlays; add it here only 
# when the nrf7002eb2 shield is not used, and drop the alias from the shield overlay
# when it is.
if(DEFINED BOARD AND BOARD MATCHES "^nrf54l15dk_nrf54l15_cpuapp(_ns)?$")
	set(nrf7002eb2_shield_dir ${CMAKE_CURRENT_LIST_DIR}/boards/shields/nrf7002eb2)
	set(nrf7002eb2_shield_selected FALSE)

	if(DEFINED SHIELD)
		string(REPLACE " " ";" shield_list "${SHIELD}")
		foreach(shield_name IN LISTS shield_list)
			if(shield_name MATCHES "^nrf7002eb2")
				set(nrf7002eb2_shield_selected TRUE)
				set(shield_overlay ${nrf7002eb2_shield_dir}/boards/${BOARD}.overlay)
				if(EXISTS "${shield_overlay}")
					list(APPEND EXTRA_DTC_OVERLAY_FILE "${shield_overlay}")
				endif()
				break()
			endif()
		endforeach()
	endif()

	if(NOT nrf7002eb2_shield_selected)
		list(APPEND EXTRA_DTC_OVERLAY_FILE
		     "${nrf7002eb2_shield_dir}/nrf54l15_state_notifier_time_sync.overlay")
	endif()
endif()

if(CONFIG_SIDEWALK_BUILD)
    if (CONFIG_SIDEWALK_USE_PREBUILT_LIBRARIES)
        add_subdirectory(lib)
    endif()
    add_subdirectory(subsys)
    add_subdirectory(utils)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/sidewalk.cmake)
endif()

if(CONFIG_SOC_SERIES_NRF54LX AND CONFIG_SETTINGS_NVS)
    message(WARNING "
    The nRF54L series is not optimized for the NVS settings backend due to RRAM specifications.
    Consider using the CONFIG_SETTINGS_ZMS Kconfig option, which is recommended for this type of non-volatile memory.")
endif()
