#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_library(sid_pal_radio_lr11xx_location_impl STATIC
	sid_pal_gnss_wifi_impl.c
	smtc_modem_impl.c
)

if(NOT CONFIG_SIDEWALK_WIFI_LOCATION_NRF70)
	target_sources(sid_pal_radio_lr11xx_location_impl PRIVATE
		sid_pal_wifi_lr1110_impl.c
	)
endif()

target_include_directories(sid_pal_radio_lr11xx_location_impl PRIVATE
	${SEMTECH_COMMON_INCLUDES}
	${LR11XX_COMMON_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../app_utils/config/include
	${SID_PAL_PUBLIC_INCLUDES}
)

target_compile_definitions(sid_pal_radio_lr11xx_location_impl PRIVATE
	DUAL_LINK_SUPPORT=1
	SID_PLATFORM_NORDIC=1
	SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
	LR11XX
	RP2_103
	NUMBER_OF_STACKS=1
	REGION_US_915
)

target_link_libraries(sid_pal_radio_lr11xx_location_impl PUBLIC
	smtc_lbm
	semtech_radio_ifc
	sid_pal_gnss_ifc
	sid_pal_wifi_ifc
	sid_pal_log_ifc
	sid_pal_timer_ifc
	sid_pal_delay_ifc
	sid_pal_gpio_ifc
	sid_pal_radio_ifc
	sid_pal_serial_bus_ifc
	sid_pal_mfg_store_ifc
	sid_900_cfg_ifc
	sid_time_ops
	sid_clock_ifc
	sid_utils
	zephyr_interface
)

if(CONFIG_SIDEWALK_USE_PREBUILT_LIBRARIES)
	if(CONFIG_CPU_CORTEX_M4)
		set(_sid_lora_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/cortex-m4/lora_fsk)
	else()
		set(_sid_lora_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/cortex-m33/lora_fsk)
	endif()
	target_link_libraries(sid_pal_radio_lr11xx_location_impl PRIVATE
		${_sid_lora_lib_dir}/libsmtc_lbm.a
	)
endif()
