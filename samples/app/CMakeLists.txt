#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(hci_rpmsg_KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/child_image/hci_rpmsg/Kconfig.root)
set(mcuboot_KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/child_image/mcuboot/Kconfig.root)

if(CONF_FILE)
    get_filename_component(CONFIG_FILE_NAME ${CONF_FILE} NAME)
endif()

if("${CONFIG_FILE_NAME}" STREQUAL "prj_no_dfu.conf")
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/pm_static_no_dfu.yml)
else()
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static_dfu.yml)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(state_machine_app)

target_sources(app PRIVATE src/main.c)
