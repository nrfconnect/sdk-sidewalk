#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# Sysbuild strips the /ns board qualifier when building MCUboot, so board-specific
# overlays named <board>_ns.overlay under sysbuild/mcuboot/boards/ are not picked
# up automatically. Apply them manually for non-secure builds.
if(SB_CONFIG_BOARD_NRF54L15DK_NRF54L15_CPUAPP_NS)
  set(mcuboot_DTC_OVERLAY_FILE
    "${CMAKE_CURRENT_LIST_DIR}/sysbuild/mcuboot/boards/nrf54l15dk_nrf54l15_cpuapp_ns.overlay"
    CACHE INTERNAL "devicetree overlay file for MCUboot on non-secure builds"
    FORCE
  )
endif()
