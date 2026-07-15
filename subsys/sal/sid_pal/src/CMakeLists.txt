#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(SID_PAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
set(SID_PAL_APP_UTILS_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/../../../app_utils/config/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../../app_utils/hal/include
)

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_LOG)
	if(NOT(CONFIG_LOG) OR CONFIG_SIDEWALK_LOG_LEVEL_OFF)
		set(SID_PAL_LOG_DEFS SID_PAL_LOG_ENABLED=0)
	else()
		set(SID_PAL_LOG_DEFS
			SID_PAL_LOG_ENABLED=1
			SID_PAL_LOG_LEVEL=${CONFIG_SIDEWALK_LOG_LEVEL}-1
		)
	endif()

	zephyr_library_named(sid_pal_log_impl)
	target_sources(sid_pal_log_impl PRIVATE sid_log.c)
	target_include_directories(sid_pal_log_impl PRIVATE ${SID_PAL_INCLUDE})
	target_compile_definitions(sid_pal_log_impl PRIVATE ${SID_PAL_LOG_DEFS})
	target_link_libraries(sid_pal_log_impl PRIVATE sid_pal_log_ifc)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_ASSERT)
	zephyr_library_named(sid_pal_assert_impl)
	target_sources(sid_pal_assert_impl PRIVATE sid_assert.c)
	target_include_directories(sid_pal_assert_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_assert_impl PRIVATE sid_pal_assert_ifc)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_BLE)
	set(SID_PAL_BLE_ADAPTER_SOURCES
		sid_ble_adapter.c
		sid_ble_service.c
		sid_ble_ama_service.c
		sid_ble_adapter_callbacks.c
		sid_ble_advert.c
		sid_ble_connection.c
		hci_utils.c
		bt_app_callbacks.c
	)
	if(CONFIG_SIDEWALK_VENDOR_SERVICE)
		list(APPEND SID_PAL_BLE_ADAPTER_SOURCES sid_ble_vnd_service.c)
	endif()
	if(CONFIG_SIDEWALK_LOGGING_SERVICE)
		list(APPEND SID_PAL_BLE_ADAPTER_SOURCES sid_ble_log_service.c)
	endif()

	zephyr_library_named(sid_pal_ble_adapter_impl)
	target_sources(sid_pal_ble_adapter_impl PRIVATE ${SID_PAL_BLE_ADAPTER_SOURCES})
	target_include_directories(sid_pal_ble_adapter_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_ble_adapter_impl PRIVATE
		sid_pal_ble_adapter_ifc
		sid_ble_cfg_ifc
		sid_error
	)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_CRYPTO)
	set(SID_PAL_CRYPTO_SOURCES sid_crypto.c)
	if(CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE)
		list(APPEND SID_PAL_CRYPTO_SOURCES sid_crypto_keys.c)
	endif()

	zephyr_library_named(sid_pal_crypto_impl)
	zephyr_library_sources(${SID_PAL_CRYPTO_SOURCES})
	zephyr_library_include_directories(${SID_PAL_INCLUDE})
	zephyr_library_link_libraries(sid_pal_crypto_ifc sid_error)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_MFG_STORAGE)
	set(SID_PAL_MFG_STORE_SOURCES sid_mfg_storage.c sid_mfg_hex_v8.c)
	if(CONFIG_SIDEWALK_MFG_STORAGE_SUPPORT_HEX_v7)
		list(APPEND SID_PAL_MFG_STORE_SOURCES sid_mfg_hex_v7.c)
	endif()

	zephyr_library_named(sid_pal_mfg_store_impl)
	target_sources(sid_pal_mfg_store_impl PRIVATE ${SID_PAL_MFG_STORE_SOURCES})
	target_include_directories(sid_pal_mfg_store_impl PRIVATE
		${SID_PAL_INCLUDE}
		${SID_PAL_APP_UTILS_INCLUDES}
	)
	target_link_libraries(sid_pal_mfg_store_impl PRIVATE
		sid_pal_mfg_store_ifc
		sid_error
	)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_DEPRECATED_SIDEWALK_MFG_STORAGE)
	zephyr_library_named(sid_pal_mfg_store_deprecated_impl)
	target_sources(sid_pal_mfg_store_deprecated_impl PRIVATE sid_mfg_storage_deprecated.c)
	target_include_directories(sid_pal_mfg_store_deprecated_impl PRIVATE
		${SID_PAL_INCLUDE}
		${SID_PAL_APP_UTILS_INCLUDES}
	)
	target_link_libraries(sid_pal_mfg_store_deprecated_impl PRIVATE
		sid_pal_mfg_store_ifc
		sid_error
	)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_STORAGE)
	zephyr_library_named(sid_pal_storage_kv_impl)
	zephyr_library_sources(sid_storage.c)
	zephyr_library_include_directories(${SID_PAL_INCLUDE})
	zephyr_library_link_libraries(sid_pal_storage_kv_ifc sid_error)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_TIMER)
	zephyr_library_named(sid_pal_timer_impl)
	target_sources(sid_pal_timer_impl PRIVATE sid_timer.c)
	target_include_directories(sid_pal_timer_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_timer_impl PRIVATE
		sid_pal_timer_ifc
		sid_pal_uptime_ifc
		sid_pal_assert_ifc
		sid_pal_critical_region_ifc
		sid_time_ops
	)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_UPTIME)
	zephyr_library_named(sid_pal_uptime_impl)
	target_sources(sid_pal_uptime_impl PRIVATE sid_uptime.c zephyr_time.c)
	target_include_directories(sid_pal_uptime_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_uptime_impl PRIVATE sid_pal_uptime_ifc)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_CRITICAL_REGION)
	zephyr_library_named(sid_pal_critical_region_impl)
	target_sources(sid_pal_critical_region_impl PRIVATE sid_critical_region.c)
	target_include_directories(sid_pal_critical_region_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_critical_region_impl PRIVATE sid_pal_critical_region_ifc)
endif()

if(CONFIG_SIDEWALK_GPIO)
	zephyr_library_named(sid_pal_gpio_impl)
	target_sources(sid_pal_gpio_impl PRIVATE sid_gpio.c sid_gpio_utils.c)
	target_include_directories(sid_pal_gpio_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_gpio_impl PRIVATE sid_pal_gpio_ifc)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_TEMPERATURE)
	if(CONFIG_HAS_HW_NRF_TEMP)
		zephyr_library_named(sid_pal_temperature_impl)
		target_sources(sid_pal_temperature_impl PRIVATE sid_temperature.c)
	else()
		zephyr_library_named(sid_pal_temperature_impl)
		target_sources(sid_pal_temperature_impl PRIVATE sid_temperature_mock.c)
		message("Note: Sidewalk uses MOCK temperature implementation")
	endif()
	target_include_directories(sid_pal_temperature_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_temperature_impl PRIVATE sid_pal_temperature_ifc)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_SW_INTERRUPTS)
	zephyr_library_named(sid_pal_swi_impl)
	target_sources(sid_pal_swi_impl PRIVATE sid_sw_interrupts.c)
	target_include_directories(sid_pal_swi_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_swi_impl PRIVATE sid_pal_swi_ifc)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_DELAY)
	zephyr_library_named(sid_pal_delay_impl)
	target_sources(sid_pal_delay_impl PRIVATE sid_delay.c)
	target_include_directories(sid_pal_delay_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_delay_impl PRIVATE sid_pal_delay_ifc)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_SPI_BUS)
	zephyr_library_named(sid_pal_serial_bus_impl)
	target_sources(sid_pal_serial_bus_impl PRIVATE sid_pal_serial_bus_spi.c)
	target_include_directories(sid_pal_serial_bus_impl PRIVATE
		${SID_PAL_INCLUDE}
		${CMAKE_CURRENT_SOURCE_DIR}/../../../app_utils/config/include
	)
	target_link_libraries(sid_pal_serial_bus_impl PRIVATE
		sid_pal_serial_bus_ifc
		sid_pal_gpio_ifc
		sid_pal_mfg_store_ifc
		sid_900_cfg_ifc
	)
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK)
	zephyr_library_named(sid_pal_common_impl)
	target_sources(sid_pal_common_impl PRIVATE sid_common.c)
	target_include_directories(sid_pal_common_impl PRIVATE ${SID_PAL_INCLUDE})
	if(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X)
		target_include_directories(sid_pal_common_impl PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/include
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/sx126x/include
		)
		target_link_libraries(sid_pal_common_impl PRIVATE
			semtech_radio_ifc
			sid_pal_radio_ifc
			sid_pal_serial_bus_ifc
			sid_time_types
		)
	endif()
	if(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
		target_include_directories(sid_pal_common_impl PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/include
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/common
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/lr11xx/include
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/lr11xx/include/semtech
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/lr11xx/semtech
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/lr11xx
			${CMAKE_CURRENT_SOURCE_DIR}/../../../semtech/lr1110_location/include
		)
	endif()
	target_link_libraries(sid_pal_common_impl PRIVATE
		sid_pal_common_ifc
		sid_error
		sid_pal_temperature_ifc
	)
	if(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
		target_link_libraries(sid_pal_common_impl PRIVATE
			semtech_radio_ifc
			sid_pal_radio_ifc
			sid_pal_serial_bus_ifc
			sid_time_types
		)
	endif()
endif()

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_DEPRECATED_SIDEWALK_PAL_INIT)
	zephyr_library_named(sid_pal_init_impl)
	target_sources(sid_pal_init_impl PRIVATE pal_init.c)
	target_include_directories(sid_pal_init_impl PRIVATE ${SID_PAL_INCLUDE})
	target_link_libraries(sid_pal_init_impl PRIVATE
		sid_pal_crypto_ifc
		sid_pal_storage_kv_ifc
		sid_pal_mfg_store_ifc
		sid_pal_temperature_ifc
	)
endif()

# zephyr_library_named() leaves ZEPHYR_CURRENT_LIBRARY set; later bare
# zephyr_library_sources() calls (e.g. in other modules) would attach to the
# wrong target if this is not cleared.
set(ZEPHYR_CURRENT_LIBRARY)
