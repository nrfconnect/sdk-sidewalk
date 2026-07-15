#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

function(sidewalk_add_ifc_targets sidewalk_base binary_dir)
	if(TARGET sid_sdk_component_pal_ifc)
		return()
	endif()

	set(SAL_COMMON ${sidewalk_base}/subsys/sal/common)
	set(BIN ${binary_dir}/sidewalk_ifc)

	add_subdirectory(${sidewalk_base}/subsys/sal/sid_pal/sid_pal_types ${BIN}/sid_pal_types)

	set(SID_IFC_MODULES
		sid_900_cfg sid_api sid_ble_cfg sid_bulk_data_transfer_api
		sid_detect_unwanted_location_tracker_api sid_device_capabilities
		sid_device_information_api sid_diag_log_cfg sid_error sid_location
		sid_on_dev_cert sid_reset_info sid_sdk_config
		sid_sdk_version sid_setup_failure sid_time_sync_config
	)
	foreach(module ${SID_IFC_MODULES})
		add_subdirectory(${SAL_COMMON}/public/sid_ifc/${module} ${BIN}/sid_ifc_${module})
	endforeach()

	set(SID_PAL_IFC_MODULES
		assert ble_adapter common critical_region crypto delay dult gnss gpio log
		mfg_store radio serial_bus_ifc storage_kv swi temperature timer uptime wifi
	)
	foreach(module ${SID_PAL_IFC_MODULES})
		add_subdirectory(${SAL_COMMON}/public/sid_pal_ifc/${module} ${BIN}/sid_pal_ifc_${module})
	endforeach()

	add_library(sid_sdk_component_ifc INTERFACE)
	target_link_libraries(sid_sdk_component_ifc INTERFACE
		sid_900_cfg_ifc sid_api_ifc sid_ble_cfg_ifc sid_ble_link_cfg_ifc
		sid_bulk_data_transfer_api_ifc
		sid_detect_unwanted_location_tracker_api_ifc sid_device_capabilities_ifc
		sid_device_information_api_ifc sid_diag_log_cfg sid_error sid_location_ifc
		sid_on_dev_cert_ifc sid_reset_info_types_ifc
		sid_sdk_config_ifc sid_sdk_version_ifc sid_setup_failure sid_time_sync_config
		sid_time_types
	)

	add_library(sid_sdk_component_pal_ifc INTERFACE)
	target_link_libraries(sid_sdk_component_pal_ifc INTERFACE
		sid_pal_assert_ifc sid_pal_ble_adapter_ifc sid_pal_common_ifc
		sid_pal_critical_region_ifc sid_pal_crypto_ifc sid_pal_delay_ifc
		sid_pal_dult_ifc sid_pal_gpio_ifc sid_pal_log_ifc sid_pal_mfg_store_ifc
		sid_pal_radio_ifc sid_pal_serial_bus_ifc sid_pal_storage_kv_ifc
		sid_pal_swi_ifc sid_pal_temperature_ifc sid_pal_timer_ifc sid_pal_uptime_ifc
	)
endfunction()

function(sidewalk_link_test_ifc target sidewalk_base binary_dir)
	sidewalk_add_ifc_targets(${sidewalk_base} ${binary_dir})
	foreach(module ${ARGN})
		target_link_libraries(${target} PRIVATE ${module})
	endforeach()
endfunction()

function(sidewalk_link_test_sdk_ifc target sidewalk_base binary_dir)
	sidewalk_add_ifc_targets(${sidewalk_base} ${binary_dir})
	target_link_libraries(${target} PRIVATE sid_sdk_component_ifc)
	foreach(module ${ARGN})
		target_link_libraries(${target} PRIVATE ${module})
	endforeach()
endfunction()
