#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library_named(sid_pal_wifi_nrf70_impl)

target_sources(sid_pal_wifi_nrf70_impl PRIVATE
	nrf70_wifi_scan.c
	sid_pal_wifi_nrf70_impl.c
)

if(NOT CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
	target_sources(sid_pal_wifi_nrf70_impl PRIVATE
		sid_pal_gnss_stub.c
	)
	target_link_libraries(sid_pal_wifi_nrf70_impl PRIVATE
		sid_pal_gnss_ifc
	)
endif()

target_include_directories(sid_pal_wifi_nrf70_impl PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sid_pal_wifi_nrf70_impl PRIVATE
	sid_pal_wifi_ifc
)
