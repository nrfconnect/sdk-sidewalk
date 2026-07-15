#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# sid_ifc interface libraries
add_subdirectory(public/sid_ifc/sid_900_cfg)
add_subdirectory(public/sid_ifc/sid_api)
add_subdirectory(public/sid_ifc/sid_ble_cfg)
add_subdirectory(public/sid_ifc/sid_bulk_data_transfer_api)
add_subdirectory(public/sid_ifc/sid_detect_unwanted_location_tracker_api)
add_subdirectory(public/sid_ifc/sid_device_capabilities)
add_subdirectory(public/sid_ifc/sid_diag_log_cfg)
add_subdirectory(public/sid_ifc/sid_error)
add_subdirectory(public/sid_ifc/sid_on_dev_cert)
add_subdirectory(public/sid_ifc/sid_reset_info)
add_subdirectory(public/sid_ifc/sid_sdk_config)
add_subdirectory(public/sid_ifc/sid_sdk_version)
add_subdirectory(public/sid_ifc/sid_setup_failure)
add_subdirectory(public/sid_ifc/sid_location)
add_subdirectory(public/sid_ifc/sid_device_information_api)
add_subdirectory(public/sid_ifc/sid_time_sync_config)

add_library(sid_sdk_component_ifc INTERFACE)

target_link_libraries(sid_sdk_component_ifc
  INTERFACE
    sid_900_cfg_ifc
    sid_api_ifc
    sid_ble_cfg_ifc
    sid_ble_link_cfg_ifc
    sid_bulk_data_transfer_api_ifc
    sid_detect_unwanted_location_tracker_api_ifc
    sid_device_capabilities_ifc
    sid_device_information_api_ifc
    sid_diag_log_cfg
    sid_error
    sid_location_ifc
    sid_on_dev_cert_ifc
    sid_reset_info_types_ifc
    sid_sdk_config_ifc
    sid_sdk_version_ifc
    sid_setup_failure
    sid_time_sync_config
    sid_time_types
)

# sid_pal_ifc interface libraries
add_subdirectory(public/sid_pal_ifc/assert)
add_subdirectory(public/sid_pal_ifc/ble_adapter)
add_subdirectory(public/sid_pal_ifc/common)
add_subdirectory(public/sid_pal_ifc/critical_region)
add_subdirectory(public/sid_pal_ifc/crypto)
add_subdirectory(public/sid_pal_ifc/delay)
add_subdirectory(public/sid_pal_ifc/dult)
add_subdirectory(public/sid_pal_ifc/gpio)
add_subdirectory(public/sid_pal_ifc/log)
add_subdirectory(public/sid_pal_ifc/mfg_store)
add_subdirectory(public/sid_pal_ifc/radio)
add_subdirectory(public/sid_pal_ifc/serial_bus_ifc)
add_subdirectory(public/sid_pal_ifc/storage_kv)
add_subdirectory(public/sid_pal_ifc/swi)
add_subdirectory(public/sid_pal_ifc/temperature)
add_subdirectory(public/sid_pal_ifc/timer)
add_subdirectory(public/sid_pal_ifc/uptime)
add_subdirectory(public/sid_pal_ifc/gnss)
add_subdirectory(public/sid_pal_ifc/wifi)

add_library(sid_sdk_component_pal_ifc INTERFACE)

target_link_libraries(sid_sdk_component_pal_ifc
  INTERFACE
    sid_pal_assert_ifc
    sid_pal_ble_adapter_ifc
    sid_pal_common_ifc
    sid_pal_critical_region_ifc
    sid_pal_crypto_ifc
    sid_pal_delay_ifc
    sid_pal_dult_ifc
    sid_pal_gpio_ifc
    sid_pal_log_ifc
    sid_pal_mfg_store_ifc
    sid_pal_radio_ifc
    sid_pal_serial_bus_ifc
    sid_pal_storage_kv_ifc
    sid_pal_swi_ifc
    sid_pal_temperature_ifc
    sid_pal_timer_ifc
    sid_pal_uptime_ifc
)

# internal libraries
add_subdirectory(internal/sid_endian)
add_subdirectory(internal/sid_clock_ifc)

add_subdirectory(ace)
