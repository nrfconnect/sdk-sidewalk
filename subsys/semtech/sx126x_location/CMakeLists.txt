#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_library(sid_pal_radio_sx126x_location_impl STATIC
	sid_pal_gnss_wifi_impl.c
)

target_include_directories(sid_pal_radio_sx126x_location_impl PRIVATE
	${SEMTECH_COMMON_INCLUDES}
)

target_link_libraries(sid_pal_radio_sx126x_location_impl PUBLIC
	sid_pal_gnss_ifc
	sid_pal_wifi_ifc
	sid_error
	zephyr_interface
)
