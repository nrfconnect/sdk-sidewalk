#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Zephyr CMake project
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_lr11xx_firmware_update)

# Source files
target_include_directories(app PRIVATE include)
target_sources(app PRIVATE
	src/main.c
	src/app.c
	src/lr11xx_firmware_update.c
)

if(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
	target_include_directories(app PRIVATE
		${ZEPHYR_SIDEWALK_MODULE_DIR}/subsys/semtech/lr11xx
		${ZEPHYR_SIDEWALK_MODULE_DIR}/subsys/semtech/lr11xx/semtech
	)
	target_link_libraries(app PRIVATE
		smtc_lbm
		semtech_radio_ifc
	)
endif()

if(CONFIG_SIDEWALK_USE_PREBUILT_LIBRARIES)
	target_link_libraries(app PRIVATE sidewalk)
endif()
