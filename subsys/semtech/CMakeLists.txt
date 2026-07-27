#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(SEMTECH_COMMON_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/common
)

set(LR11XX_COMMON_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/lr11xx/include
	${CMAKE_CURRENT_SOURCE_DIR}/lr11xx/include/semtech
	${CMAKE_CURRENT_SOURCE_DIR}/lr11xx/semtech
	${CMAKE_CURRENT_SOURCE_DIR}/lr11xx
)

set(SMTC_LBM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/smtc_lbm)
set(LBM_PORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lbm_port)

set(SMTC_LBM_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/lr1110/include
	${CMAKE_CURRENT_SOURCE_DIR}/lr1110_location/include
	${LBM_PORT_DIR}
	${LBM_PORT_DIR}/config
	${LBM_PORT_DIR}/RALBSP
	${SMTC_LBM_DIR}/smtc_modem_core/radio_drivers/lr11xx_driver/src
	${SMTC_LBM_DIR}/smtc_modem_core/smtc_ral/src
	${SMTC_LBM_DIR}/smtc_modem_core/smtc_ralf/src
	${SMTC_LBM_DIR}/smtc_modem_core/geolocation_services
	${SMTC_LBM_DIR}/smtc_modem_core
	${SMTC_LBM_DIR}/smtc_modem_api
	${SMTC_LBM_DIR}/smtc_modem_hal
	${SMTC_LBM_DIR}/smtc_modem_core/radio_drivers/sx126x_driver/src
	${SMTC_LBM_DIR}/smtc_modem_core/lorawan_manager
	${SMTC_LBM_DIR}/smtc_modem_core/lr1mac/src
	${SMTC_LBM_DIR}/smtc_modem_core/smtc_modem_crypto
	${SMTC_LBM_DIR}/smtc_modem_core/smtc_modem_crypto/smtc_secure_element
	${SMTC_LBM_DIR}/smtc_modem_core/lr1mac/src/smtc_real/src
	${SMTC_LBM_DIR}/smtc_modem_core/radio_planner/src
	${SMTC_LBM_DIR}/smtc_modem_core/lr1mac/src/services
	${SMTC_LBM_DIR}/smtc_modem_core/modem_supervisor
	${SMTC_LBM_DIR}/smtc_modem_core/modem_utilities
	${SMTC_LBM_DIR}/smtc_modem_core/lorawan_packages/lorawan_certification
	${SMTC_LBM_DIR}/smtc_modem_core/lorawan_api
	${SMTC_LBM_DIR}
	${SMTC_LBM_DIR}/smtc_modem_core/modem_config
	${SMTC_LBM_DIR}/smtc_modem_core/lr1mac
	${SMTC_LBM_DIR}/smtc_modem_core/modem_services
)

set(SID_PAL_PUBLIC_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/../sal/common/public/sid_pal_ifc/mfg_store
)

add_subdirectory(common)

add_library(smtc_lbm INTERFACE)
target_include_directories(smtc_lbm INTERFACE ${SMTC_LBM_INCLUDES})
target_compile_definitions(smtc_lbm INTERFACE
	RP2_103
	NUMBER_OF_STACKS=1
	LR11XX
	REGION_US_915
	ADD_LBM_GEOLOCATION
)

add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X sx126x)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X sx126x_location)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 lr11xx)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 lr1110)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 lr1110_location)

add_library(sidewalk_radio INTERFACE)

if(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X)
	target_link_libraries(sidewalk_radio INTERFACE
		"-Wl,--start-group"
		sid_pal_radio_sx126x_impl
		sid_pal_radio_sx126x_location_impl
		"-Wl,--end-group"
	)
endif()

if(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
	target_compile_definitions(sidewalk_radio INTERFACE
		RP2_103
		NUMBER_OF_STACKS=1
		LR11XX
		REGION_US_915
	)
	target_link_libraries(sidewalk_radio INTERFACE
		"-Wl,--start-group"
		sid_pal_radio_lr11xx_impl
		sid_pal_radio_lr11xx_location_impl
		"-Wl,--end-group"
	)
endif()
