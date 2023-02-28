#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

if(CONFIG_SIDEWALK_BUILD)
	add_subdirectory(lib)
	add_subdirectory(subsys)
	add_subdirectory(utils)
	include(version.cmake)
endif()
