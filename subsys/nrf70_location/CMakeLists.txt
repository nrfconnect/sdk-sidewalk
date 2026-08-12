#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_library(sid_pal_wifi_nrf70_impl STATIC
	nrf70_wifi_scan.c
	sid_pal_wifi_nrf70_impl.c
)

target_link_libraries(sid_pal_wifi_nrf70_impl PUBLIC
	sid_pal_wifi_ifc
	zephyr_interface
)

add_dependencies(sid_pal_wifi_nrf70_impl zephyr_generated_headers)
