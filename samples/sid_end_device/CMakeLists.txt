#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Sidewalk version
include(bootloader_version.cmake)

# Child images
set(hci_rpmsg_KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/child_image/hci_rpmsg/Kconfig.root)
set(mcuboot_KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/child_image/mcuboot/Kconfig.root)

# Configurations
if(CONF_FILE)
    get_filename_component(CONFIG_FILE_NAME ${CONF_FILE} NAME)
endif()

if("${CONFIG_FILE_NAME}" STREQUAL "prj_no_dfu.conf")
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/pm_static_no_dfu.yml)
else()
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static_dfu.yml)
endif()

# Zephyr CMake project
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_template)

# Source files
target_include_directories(app PRIVATE include)
target_sources(app PRIVATE
    src/main.c
    src/sidewalk.c
)

if(CONFIG_TEMPLATE_APP_SENSOR_MONITORING)
    target_sources(app PRIVATE
        src/sensor_monitoring/app_buttons.c
        src/sensor_monitoring/app_leds.c
        src/sensor_monitoring/app_sensor.c
        src/sensor_monitoring/app_tx.c
        src/sensor_monitoring/app_rx.c
        src/sensor_monitoring/app.c
    )
elseif(CONFIG_TEMPLATE_APP_DUT)
    target_sources(app PRIVATE
        src/cli/app.c
    )
else()
    target_sources(app PRIVATE
        src/hello/app.c
    )
endif()

if(CONFIG_TEMPLATE_APP_CLI)
    target_sources(app PRIVATE
        src/cli/app_dut.c
        src/cli/app_shell.c
    )
endif()
