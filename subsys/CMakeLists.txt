#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if (CONFIG_SIDEWALK_USE_PREBUILTS)
    add_subdirectory(ace)
    add_subdirectory(config)
    add_subdirectory(hal)
    add_subdirectory(semtech)
endif()

add_subdirectory(sal)

add_subdirectory_ifdef(CONFIG_SIDEWALK_DEMO_PARSER demo)
