#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(APP_UTILS_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)
set(APP_UTILS_HAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hal)

set(APP_UTILS_SOURCES
	${APP_UTILS_HAL_DIR}/src/memory.c
	${APP_UTILS_HAL_DIR}/src/reset.c
)

if(CONFIG_SIDEWALK_BLE)
	list(APPEND APP_UTILS_SOURCES ${APP_UTILS_CONFIG_DIR}/src/app_ble_config.c)
endif()

if(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
	if(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X)
		list(APPEND APP_UTILS_SOURCES ${APP_UTILS_CONFIG_DIR}/src/app_subGHz_config_sx126x.c)
	endif()
	if(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
		list(APPEND APP_UTILS_SOURCES
			${APP_UTILS_CONFIG_DIR}/src/app_subGHz_config_lr11xx.c
			${APP_UTILS_CONFIG_DIR}/src/app_location_config_lr11xx.c
		)
	endif()
else()
	list(APPEND APP_UTILS_SOURCES ${APP_UTILS_CONFIG_DIR}/src/app_subGHz_config_empty.c)
endif()

zephyr_library_named(app_utils)
target_sources(app_utils PRIVATE ${APP_UTILS_SOURCES})

target_include_directories(app_utils
	PUBLIC
		${APP_UTILS_CONFIG_DIR}/include
		${APP_UTILS_HAL_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../sal/sid_pal/include
)

target_link_libraries(app_utils PUBLIC
	sid_error
	sid_ble_link_cfg_ifc
	sid_pal_mfg_store_ifc
	sid_900_cfg_ifc
)

if(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X)
	target_include_directories(app_utils PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../sal/sid_pal/sid_pal_types
		${CMAKE_CURRENT_SOURCE_DIR}/../semtech/include
		${CMAKE_CURRENT_SOURCE_DIR}/../semtech/sx126x/include
	)
	target_link_libraries(app_utils PUBLIC
		semtech_radio_ifc
		sid_pal_serial_bus_ifc
		sid_pal_radio_ifc
		sid_pal_gpio_ifc
	)
endif()

if(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
	target_include_directories(app_utils PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../sal/sid_pal/sid_pal_types
		${CMAKE_CURRENT_SOURCE_DIR}/../semtech/include
		${CMAKE_CURRENT_SOURCE_DIR}/../semtech/common
		${CMAKE_CURRENT_SOURCE_DIR}/../semtech/lr11xx
		${CMAKE_CURRENT_SOURCE_DIR}/../semtech/lr11xx/include
		${CMAKE_CURRENT_SOURCE_DIR}/../semtech/lr11xx/semtech
		${CMAKE_CURRENT_SOURCE_DIR}/../semtech/lr1110_location/include
	)
	target_link_libraries(app_utils PUBLIC
		smtc_lbm
		semtech_radio_ifc
		sid_pal_serial_bus_ifc
		sid_pal_radio_ifc
		sid_pal_gpio_ifc
	)
endif()
