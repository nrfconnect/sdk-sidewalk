#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# MCUboot needs the flash partitions of the application
string(REPLACE "/" "_" board_overlay "${BOARD}_${BOARD_QUALIFIERS}.overlay")
sysbuild_cache_set(VAR mcuboot_EXTRA_DTC_OVERLAY_FILE APPEND
		   ${CMAKE_CURRENT_LIST_DIR}/boards/${board_overlay})
