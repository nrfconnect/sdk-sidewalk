#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory(ace)
add_subdirectory(config)
add_subdirectory(hal)
add_subdirectory(sal)
add_subdirectory(semtech)

add_subdirectory_ifdef(CONFIG_SIDEWALK_DEMO_PARSER demo)
