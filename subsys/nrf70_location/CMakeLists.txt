#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# A regular (non zephyr_library) static library: it is pulled in explicitly as
# a dependency of sidewalk_pal (see subsys/sal/sid_pal/CMakeLists.txt) instead
# of being whole-archive linked into the final image the way zephyr_library()
# targets are.
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

# The nRF70 Wi-Fi PAL takes over from the sx126x_location NOSUPPORT stub
# (subsys/semtech/sx126x_location), which builds both its GNSS and Wi-Fi
# NOSUPPORT implementations from a single translation unit. Rather than
# editing that file/target (out of scope for this PAL, see Kconfig comment on
# SIDEWALK_WIFI_LOCATION_NRF70) or accepting duplicate symbols at link time,
# drop its sources here so the resulting static library is empty and
# contributes no symbols. This only takes effect when SX126X is selected
# alongside this PAL; the target does not exist otherwise.
if(TARGET sid_pal_radio_sx126x_location_impl)
	set_target_properties(sid_pal_radio_sx126x_location_impl PROPERTIES
		SOURCES ""
		LINKER_LANGUAGE C
	)
endif()
