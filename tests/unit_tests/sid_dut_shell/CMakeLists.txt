#
#  Copyright (c) 2023 Nordic Semiconductor ASA
#
#  SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
project(state_notifier)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})

get_filename_component(SIDEWALK_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. ABSOLUTE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")

target_sources(testbinary PRIVATE
	src/main.c
	${SIDEWALK_BASE}/samples/sid_end_device/src/cli/app_shell.c
	${ZEPHYR_BASE}/lib/utils/hex.c
	mock/sidewalk_version.c
	mock/stub_ep_cfg_metrics.c
)

add_definitions(--include ztest.h)

target_include_directories(testbinary PRIVATE
	${SIDEWALK_BASE}/samples/sid_end_device/include
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_api
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_900_cfg
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_ble_cfg
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_bulk_data_transfer_api
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_detect_unwanted_location_tracker_api
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_device_capabilities
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_device_information_api
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_diag_log_cfg
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_error
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_location
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_on_dev_cert
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_reset_info
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_sdk_config
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_sdk_version
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_setup_failure
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_ifc/sid_time_sync_config
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/assert
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/ble_adapter
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/common
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/critical_region
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/crypto
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/delay
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/dult
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/gpio
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/gnss
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/log
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/mfg_store
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/radio
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/serial_bus_ifc
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/storage_kv
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/swi
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/temperature
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/timer
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/uptime
	${SIDEWALK_BASE}/subsys/sal/common/public/sid_pal_ifc/wifi
	${SIDEWALK_BASE}/subsys/app_utils/config/include
	${SIDEWALK_BASE}/subsys/sal/sid_pal/sid_pal_types
	${SIDEWALK_BASE}/subsys/app_utils/hal/include
	${SIDEWALK_BASE}/utils/include
	.
	src
)
