#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

if(CONFIG_SIDEWALK_LOG_LEVEL_OFF)
	zephyr_compile_definitions(SID_PAL_LOG_ENABLED=0)
else()
	zephyr_compile_definitions(SID_PAL_LOG_ENABLED=1)
	# Sidewalk log levels are defined in include/sid_pal_log_ifc.h
	# Zephyr log levels are defined in ${ZEPHYR_BASE}/subsys/logging/Kconfig.template.log_config
	# To covert Zephyr log level to Sidewalk log level it hest to be decreased by one.
	zephyr_compile_definitions(SID_PAL_LOG_LEVEL=${CONFIG_SIDEWALK_LOG_LEVEL}-1)
endif() # CONFIG_SIDEWALK_LOG_LEVEL_OFF

zephyr_compile_definitions_ifndef(CONFIG_SIDEWALK_ASSERT SID_PAL_ASSERT_DISABLED)

zephyr_include_directories(
  include
  hal/include
  pal/include
  lib/include
)

add_subdirectory(pal)
add_subdirectory(hal)
add_subdirectory(lib)

include(version.cmake)
