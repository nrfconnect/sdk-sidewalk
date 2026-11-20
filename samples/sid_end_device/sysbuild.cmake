#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# MCUboot uses the application board overlay. MCUboot is built for the secure
# target, so the /ns target needs a dedicated overlay.
if(SB_CONFIG_BOARD_NRF54L15DK_NRF54L15_CPUAPP_NS)
  set(mcuboot_DTC_OVERLAY_FILE
    "${CMAKE_CURRENT_LIST_DIR}/sysbuild/mcuboot/boards/nrf54l15dk_nrf54l15_cpuapp_ns.overlay"
    CACHE INTERNAL "devicetree overlay file for MCUboot on non-secure builds"
    FORCE
  )
else()
  string(REPLACE "/" "_" board_overlay "${BOARD}_${BOARD_QUALIFIERS}.overlay")
  sysbuild_cache_set(VAR mcuboot_EXTRA_DTC_OVERLAY_FILE APPEND
                     ${CMAKE_CURRENT_LIST_DIR}/boards/${board_overlay})
endif()

# The nrf7002eb2 shield repurposes the led1 pin as its SPI-CS line and deletes
# the node on nRF54L15, so the state-notifier alias pointing at it has to go.
if(SB_CONFIG_BOARD_NRF54L15DK_NRF54L15_CPUAPP OR SB_CONFIG_BOARD_NRF54L15DK_NRF54L15_CPUAPP_NS)
  foreach(image sid_end_device mcuboot)
    if("${SHIELD};${${image}_SHIELD}" MATCHES "nrf7002eb2")
      sysbuild_cache_set(VAR ${image}_EXTRA_DTC_OVERLAY_FILE APPEND
                         ${CMAKE_CURRENT_LIST_DIR}/shields/nrf7002eb2_nrf54l15dk.overlay)
    endif()
  endforeach()
endif()
