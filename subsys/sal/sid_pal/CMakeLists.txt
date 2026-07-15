#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory(src)
add_subdirectory_ifdef(CONFIG_SIDEWALK_ON_DEV_CERT sid_on_dev_cert)

add_library(sidewalk_pal INTERFACE)

target_include_directories(sidewalk_pal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(sidewalk_pal INTERFACE
	sid_sdk_component_pal_ifc
	sid_time_ops
)

if(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X OR CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
	target_link_libraries(sidewalk_pal INTERFACE sidewalk_radio)
endif()
