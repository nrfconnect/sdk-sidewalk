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

if(CONFIG_SIDEWALK_WIFI_LOCATION_NRF70_GNSS_STUB)
	target_sources(sid_pal_wifi_nrf70_impl PRIVATE
		sid_pal_gnss_stub.c
	)
	target_link_libraries(sid_pal_wifi_nrf70_impl PUBLIC
		sid_pal_gnss_ifc
	)
endif()

# This PAL replaces the sx126x_location NOSUPPORT stub, which would otherwise
# provide duplicate Wi-Fi symbols.
if(TARGET sid_pal_radio_sx126x_location_impl)
	set_target_properties(sid_pal_radio_sx126x_location_impl PROPERTIES
		SOURCES ""
		LINKER_LANGUAGE C
	)
endif()
