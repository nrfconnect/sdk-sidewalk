#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Zephyr CMake project
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_sid_end_device)

# Source files
target_include_directories(app PRIVATE include)
target_sources(app PRIVATE
src/main.c
src/sidewalk.c
src/sidewalk_events.c
)

target_sources_ifdef(CONFIG_SIDEWALK_FILE_TRANSFER app PRIVATE 
    src/sbdt/scratch_buffer.c
)
target_sources_ifdef(CONFIG_SIDEWALK_FILE_TRANSFER_DFU app PRIVATE
    src/sbdt/dfu_file_transfer.c
)

if(CONFIG_SID_END_DEVICE_SENSOR_MONITORING)
    target_sources(app PRIVATE
        src/sensor_monitoring/app_buttons.c
        src/sensor_monitoring/app_leds.c
        src/sensor_monitoring/app_sensor.c
        src/sensor_monitoring/app_tx.c
        src/sensor_monitoring/app_rx.c
        src/sensor_monitoring/app.c
    )
elseif(CONFIG_SID_END_DEVICE_DUT)
    target_sources(app PRIVATE
        src/cli/app.c
    )
else()
    target_sources(app PRIVATE
        src/hello/app.c
    )
endif()

if(CONFIG_SID_END_DEVICE_CLI)
    target_sources(app PRIVATE
        src/cli/app_dut.c
        src/cli/app_shell.c
    )
    target_sources_ifdef(CONFIG_SIDEWALK_FILE_TRANSFER_SHELL app PRIVATE
        src/cli/sbdt_shell.c
        src/cli/sbdt_shell_events.c
        src/cli/sbdt_shell_file_transfer.c
    )
    target_sources_ifdef(CONFIG_SIDEWALK_ON_DEV_CERT app PRIVATE 
        src/cli/sid_on_dev_cert_cli.c
    )
    target_sources_ifdef(CONFIG_SIDEWALK_LOCATION_SHELL app PRIVATE
        src/cli/location_shell.c
        src/cli/location_shell_events.c
    )
endif()
